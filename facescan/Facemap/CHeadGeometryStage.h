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
#ifndef __HEAD_GEOMETRY_STAGE__
#define __HEAD_GEOMETRY_STAGE__

#include <vector>
#include "CPUTSoftwareMesh.h"
#include "FaceMapUtil.h"

class CPUTSoftwareTexture;
class CPUTModel;
class CPUTMeshDX11;
struct CDisplacementMapStageOutput;
struct SBaseHeadInfo;
class CPUTSoftwareMesh;

struct HeadVertex;
struct HeadProjectionInfo;

struct MorphTargetEntry;

struct SHeadGeometryStageInput
{
	CPUTSoftwareTexture *DisplacementMap;
	CDisplacementMapStageOutput *DisplacementMapInfo;
	SBaseHeadInfo *BaseHeadInfo;
	float Scale;
	float ZDisplaceOffset;
	std::vector<MorphTargetEntry> MorphTargetEntries;
	float OtherHeadBlend;
	CPUTSoftwareMesh *OtherHeadMesh;
	int Flags;
};


class CHeadGeometryStage
{
public:

	CHeadGeometryStage();
	~CHeadGeometryStage();
	void Execute(SHeadGeometryStageInput *input);

	CPUTSoftwareMesh MorphedLandmarkMesh;
    std::vector<std::pair<int, float>> LandmarkIdxToMorphedMeshVertIdx;
    CPUTSoftwareMesh DeformedMesh;

    float MorphVsScanChinHeightDelta=FLT_MAX;
    float MorphVsScanFaceWidthDelta=FLT_MAX;
    float MorphVsScanLipsSpacingDelta=FLT_MAX;

protected:
    void updateLandmarksToMorphedMeshVerticesMapItem(int landmarkIndex, int vIdx, float distance);
    void updateHeadProjectionInfo(CDisplacementMapStageOutput *dispMapInfo, SBaseHeadInfo *headInfo, float scale, float zDisplaceOffset, HeadProjectionInfo *outProjInfo);
    void updateMorphVsScanDeltas(const std::vector<float2> &mapLandmarks, const float4x4& mapToHeadSpaceTransform, const CPUTSoftwareMesh *dstMesh);
    void updateMorphedLandmarkMesh(CPUTSoftwareMesh* landmarkMesh, const std::vector<float2>& mapLandmarks, const float4x4& mapToHeadSpaceTransform);
    void updateLandmarkMeshVertexToLandmarkIndexMap(const CPUTSoftwareMesh* landmarkMesh, const std::vector<float3>& baseHeadLandmarks, const std::vector<float2> &mapLandmarks);
    void updateLandmarksToMorphedMeshVerticesMap();
    void fitDeformedMeshToFace(CPUTSoftwareTexture *controlMapColor);
    void blendDeformedMeshZAndTexture(const HeadProjectionInfo &hpi, CPUTSoftwareTexture *controlMapDisplacement, CPUTSoftwareTexture *displacementMap, bool skipDisplacementMap);
    void updateDeformedMeshNormals(CPUTSoftwareTexture *controlMapDisplacement);

private:
	std::vector<MappedVertex> mMappedFaceVertices;
    std::vector<int> mLandmarkMeshVertexToLandmarkIndex;
	CPUTCamera *mMapProjectionCamera;

};

#endif
