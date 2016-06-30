//--------------------------------------------------------------------------------------
// Copyright 2013 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
//--------------------------------------------------------------------------------------
#include "FaceMappingUtil.h"
#include "assert.h"
#include "CPUT.h"
#include "CPUT_DX11.h"
#include "CPUTTextureDX11.h"
#include "CPUTModel.h"
#include "CPUTScene.h"
#include "CPUTTexture.h"
#include "assert.h"
#include "ObjLoader.h"
#include "FileAPI.h"
#include "CPUTSprite.h"
#include "CPUTBuffer.h"
#include "CPUTSoftwareMesh.h"

#define CODE_TEXTURE_COUNT 8

struct SSampleUtilGlob
{
	CPUTTextureDX11 *codeTextures[CODE_TEXTURE_COUNT];
	CPUTMaterial *codeMaterial;

	std::string userDataDirectory;
	std::string myAssetsDirectory;

    CPUTModel *boxModel;

	CPUTSprite *QuadSprite;
};

static SSampleUtilGlob gUtilGlob;

const std::string &GetUserDataDirectory()
{
    return gUtilGlob.userDataDirectory;
}

const std::string &GetMyAssetsDirectory()
{
	return gUtilGlob.myAssetsDirectory;
}

void FaceMappingUtil_Init()
{
	std::string dir;
	CPUTFileSystem::GetExecutableDirectory(&dir);
	CPUTFileSystem::CombinePath(dir, "Media\\MyAssets", &gUtilGlob.myAssetsDirectory);
    CPUTFileSystem::CombinePath(dir, "userdata", &gUtilGlob.userDataDirectory);

    CPUTAssetLibrary *pAssetLibrary = CPUTAssetLibrary::GetAssetLibrary();
    for (int i = 0; i < CODE_TEXTURE_COUNT; i++)
    {
        char textureName[64];
        snprintf(textureName, sizeof(textureName), "$CODETEXTURE%d", i);
        std::string textureNameString = std::string(textureName);
        gUtilGlob.codeTextures[i] = (CPUTTextureDX11*)CPUTTextureDX11::Create(textureNameString, NULL, NULL);
        pAssetLibrary->AddTexture(textureName, "", "", gUtilGlob.codeTextures[i]);
    }
}

void FaceMappingUtil_Shutdown()
{
	SAFE_RELEASE(gUtilGlob.codeMaterial);
    SAFE_DELETE(gUtilGlob.QuadSprite);

	for (int i = 0; i < CODE_TEXTURE_COUNT; i++)
	{
		SetCodeTexture(i, (CPUTTexture*)NULL);
		SAFE_RELEASE(gUtilGlob.codeTextures[i]);
	}

}

#define CODE_TEXTURE_COUNT 8

void SetCodeTexture(int index, CPUTTexture *texture)
{
	CPUTTextureDX11* dxTexture = (CPUTTextureDX11*)texture;
	SetCodeTexture(index, dxTexture != NULL ? dxTexture->GetShaderResourceView() : NULL);
}

void SetCodeTexture(int index, ID3D11ShaderResourceView *srv)
{
	assert(index < CODE_TEXTURE_COUNT);
	gUtilGlob.codeTextures[index]->SetTextureAndShaderResourceView(NULL, srv);
}

void SetCodeTexture(int index, SCodeTextureWrap *texture)
{
	assert(index < CODE_TEXTURE_COUNT);
	gUtilGlob.codeTextures[index]->SetTextureAndShaderResourceView(NULL, (texture != NULL) ? texture->SRV : NULL);
}

void DrawCube(CPUTRenderParameters &renderParams, float3 position, float size, CPUTColor4 color)
{
    DrawBox(renderParams, position, float3(size, size, size), color);
}

void DrawBox(CPUTRenderParameters &renderParams, float3 position, float3 size, CPUTColor4 color)
{
    size *= 0.01f;
    float4x4 parentMatrix = float4x4Scale(size) * float4x4Translation(position);
    gUtilGlob.boxModel->SetParentMatrix(parentMatrix);
    gUtilGlob.boxModel->mUserData1 = color.ToFloat4();
    gUtilGlob.boxModel->Render(renderParams, 0);
}


void DrawQuadSC(CPUTRenderParameters &renderParams, float2 position, float size, CPUTColor4 color)
{
	CPUTBuffer *pBuffer = (CPUTBuffer*)(renderParams.mpPerModelConstants);
	CPUTModelConstantBuffer cb;
	
	cb.UserData1 = color.ToFloat4();
	pBuffer->SetData(0, sizeof(CPUTModelConstantBuffer), &cb);

	CPUTSprite *sprite = gUtilGlob.QuadSprite;
	sprite->SetCoordType(SpriteCoordType_Screen);
	sprite->SetC(position.x, position.y, size, size);
	sprite->DrawSprite(renderParams);
}

void CopyOBJDataToSoftwareMesh(tObjModel *objModel, CPUTSoftwareMesh *softwareMesh)
{
	int vertexCount = (int)objModel->m_vertices.size();
	int indexCount = (int)objModel->m_indices.size();

	softwareMesh->FreeAll();
	softwareMesh->UpdateVertexCount(vertexCount);
	softwareMesh->UpdateIndexCount(indexCount);
	softwareMesh->AddComponent(eSMComponent_Position);
	softwareMesh->AddComponent(eSMComponent_Normal);
	softwareMesh->AddComponent(eSMComponent_Tex1);

	if (sizeof(ObjIndexInt) == sizeof(uint32) && softwareMesh->IB != NULL)
		memcpy(softwareMesh->IB, &objModel->m_indices[0], sizeof(uint32) * indexCount);

	
	if (vertexCount > 0){
		tVertex *srcV = &objModel->m_vertices[0];
		for (int i = 0; i < vertexCount; i++)
		{
			softwareMesh->Pos[i] = float3(srcV->x, srcV->y, srcV->z);
			softwareMesh->Normal[i] = float3(srcV->nx, srcV->ny, srcV->nz);
			softwareMesh->Tex[i] = float2(srcV->u, srcV->v);
			srcV++;
		}
	}
}


ViewportScoped::ViewportScoped(ID3D11DeviceContext *context, D3D11_VIEWPORT *viewports, int count, CPUTRenderParameters *params)
{
    mStoredViewportCount = 8;
    context->RSGetViewports(&mStoredViewportCount, mStoredViewports);
    mContext = context;
    mParams = params;
    if (params)
    {
        mStoredParamWidth = params->mWidth;
        mStoredParamHeight = params->mHeight;
        params->mWidth = (int)viewports->Width;
        params->mHeight = (int)viewports->Height;
    }
    context->RSSetViewports(count, viewports);
}

ViewportScoped::~ViewportScoped()
{
    if (mParams)
    {
        mParams->mWidth = mStoredParamWidth;
        mParams->mHeight = mStoredParamHeight;
    }
    mContext->RSSetViewports(mStoredViewportCount, mStoredViewports);
}
