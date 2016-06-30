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
#ifndef __SAMPLE_UTIL__
#define __SAMPLE_UTIL__

#include <D3D11.h>
#include <string>

#include "cput.h"

struct tObjModel;
class CPUTSoftwareMesh;

struct SCodeTextureWrap
{
	ID3D11Texture2D *Texture;
	ID3D11Resource *Resource;
	ID3D11ShaderResourceView *SRV;
	D3D11_TEXTURE2D_DESC Desc;
};

void FaceMappingUtil_Init();
void FaceMappingUtil_Shutdown();

void SetCodeTexture(int index, ID3D11ShaderResourceView *srv);
void SetCodeTexture(int index, SCodeTextureWrap *texture);
void SetCodeTexture(int index, CPUTTexture *texture);

class CPUTMaterial;
CPUTMaterial *GetCodeSpriteMaterial();
CPUTMaterial *GetMeshPreviewMaterial();

class CPUTModel;
CPUTModel *LoadObjAsCPUTModel(const char *objFilename);

void CopyOBJDataToSoftwareMesh(tObjModel *objModel, CPUTSoftwareMesh *softwareMesh);

CPUTTexture *LoadTexture(std::string &dir, const char *filename);

class CPUTAssetSet;
bool LoadCPUTModelToSWMesh(CPUTAssetSet *set, const char *modelName, CPUTSoftwareMesh *outMesh);

#endif
