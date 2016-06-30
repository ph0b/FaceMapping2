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

};
static SSampleUtilGlob gUtilGlob;


void FaceMappingUtil_Init()
{
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

CPUTTexture *LoadTexture(std::string &dir, const char *filename)
{
    std::string textureName;
    CPUTFileSystem::CombinePath(dir, filename, &textureName);
    return CPUTTexture::Create(std::string("dynamicLoad"), textureName, false);
}

bool LoadCPUTModelToSWMesh(CPUTAssetSet *set, const char *modelName, CPUTSoftwareMesh *outMesh)
{
    CPUTModel *model = NULL;
    CPUTResult result = set->GetAssetByName(modelName, (CPUTRenderNode**)&model);
    assert(result == CPUT_SUCCESS);

    // get first mesh
    CPUTMeshDX11 *dx11Mesh = (CPUTMeshDX11 *)model->GetMesh(0);
    outMesh->CopyFromDX11Mesh(dx11Mesh);
    outMesh->ApplyTransform(model->GetWorldMatrix());
    SAFE_RELEASE(model);
    return true;
}
