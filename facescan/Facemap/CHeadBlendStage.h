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
#ifndef __HEAD_BLEND_STAGE__
#define __HEAD_BLEND_STAGE__

class CPUTModel;
class CPUTRenderTargetColor;
class CPUTMeshDX11;
class CPUTComputeShaderDX11;
class CPUTTextureDX11;
struct SBaseHeadInfo;
class CPUTSoftwareMesh;
class CPUTSprite;

struct SCodeTextureWrap
{
    ID3D11Texture2D *Texture;
    ID3D11Resource *Resource;
    ID3D11ShaderResourceView *SRV;
    D3D11_TEXTURE2D_DESC Desc;
};

#include "CPipeline.h"

struct SHeadBlendStageInput
{
	CPUTRenderParameters *RenderParams;
	SBaseHeadInfo *BaseHeadInfo;
	CPUTSoftwareMesh *DeformedMesh; // mesh that was deformed in the geometry stage
	ID3D11ShaderResourceView *GeneratedFaceColorMap;

	CPUTColor4 BlendColor1;
	CPUTColor4 BlendColor2;
	int3 PostBlendColorize[2];
	int3 PostBlendAdjust[2];
	PostBlendColorMode PostBlendMode;
	int Flags;
	float OtherHeadBlend;
	CPUTTexture *OtherHeadTexture;
};

struct SHeadBlendStageOutput
{
	CPUTTexture *OutputDiffuse;
};

class CHeadBlendStage
{
public:

	CHeadBlendStage();
	~CHeadBlendStage();

	void Execute(SHeadBlendStageInput *input);
	
	SHeadBlendStageOutput Output;

private:

	void InitResources();
	bool mIsInited;

    static const int kCodeTexturesCount = 8;
    CPUTTextureDX11 *mCodeTextures[kCodeTexturesCount];
	CPUTMaterial *mBlendMaterial;
	CPUTMeshDX11 *mRenderMesh;
	CPUTRenderTargetColor *mDiffuseRenderTarget;
	CPUTRenderTargetColor *mDiffuseRenderTarget2;
	CPUTComputeShaderDX11 *mPostComputeShader;
	CPUTMaterial *mSeamFillMaterial;
	CPUTSprite *mFullscreenSprite;

    void SetCodeTexture(int index, SCodeTextureWrap *texture);
    void SetCodeTexture(int index, ID3D11ShaderResourceView *srv);
    void SetCodeTexture(int index, CPUTTexture *texture);
};

#endif
