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
#include "CPUT.h"
#include "CHeadGeometryStage.h"
#include "CDisplacementMapStage.h"
#include "CPUTMeshDX11.h"
#include "CPUTTextureDX11.h"
#include "CPUTModel.h"
#include "CPUTSoftwareTexture.h"
#include "CPipeline.h"
#include "CPUTSoftwareMesh.h"

#include <utility>

struct HeadVertex
{
    float3 Position;
    float3 Normal;
    float2 UV1; // head texture coords
    float2 UV2; // face map texture coords
};

static D3D11_INPUT_ELEMENT_DESC gHeadVertexDesc[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { NULL, 0, (DXGI_FORMAT)0, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

struct HeadProjectionInfo
{
    // Converts head vertex position in model space to texture coordinants on generated maps
    float4x4 ViewProjTexMatrix;

    // Converts from the map coordinates to model space
    float4x4 MapToHeadSpaceTransform;

    // max/min z displacement in head model space. The maximum displacement should be applied when the depth
    float ExtrudeMinZ;
    float ExtrudeMaxZ;

    // used to convert the depth value from the depth map into
    float DepthMapRangeMin;
    float DepthMapRangeMax;
};


CHeadGeometryStage::CHeadGeometryStage()
{
    mMapProjectionCamera = CPUTCamera::Create(CPUT_ORTHOGRAPHIC);
}

CHeadGeometryStage::~CHeadGeometryStage()
{
    SAFE_RELEASE(mMapProjectionCamera);
}


void CHeadGeometryStage::updateHeadProjectionInfo(CDisplacementMapStageOutput *dispMapInfo, SBaseHeadInfo *headInfo, float scale, float zDisplaceOffset, HeadProjectionInfo *outProjInfo)
{
    float eyeDistanceMS; // model space
    float2 mapScaleFactor;
    float zScaleMSToRS;

    float3 leftEyeCenter = headInfo->BaseHeadLandmarks[kLandmarkIndex_LeftEyeAnchor];
    float3 rightEyeCenter = headInfo->BaseHeadLandmarks[kLandmarkIndex_RightEyeAnchor];
    float3 noseTip = headInfo->BaseHeadLandmarks[kLandmarkIndex_NoseTip];

    // calculate head model scale metric
    eyeDistanceMS = abs(leftEyeCenter.x - rightEyeCenter.x);
    float3 anchor = (leftEyeCenter + rightEyeCenter) / 2.0f;
    float2 anchorMS = float2(anchor.x, anchor.y);

    // calculate scaling
    mapScaleFactor.x = (eyeDistanceMS / dispMapInfo->EyeDistance_MapSpace) *scale;
    mapScaleFactor.y = (eyeDistanceMS / dispMapInfo->EyeDistance_MapSpace) *scale;
    zScaleMSToRS = (eyeDistanceMS / dispMapInfo->EyeDistance_FaceModelSpace) *scale;
    float zScaleRSToMS = 1.0f / zScaleMSToRS;

    // calculate the projection onto the head model
    float2 headProjectionPos;
    headProjectionPos.y = anchorMS.y + (dispMapInfo->Anchor_MapSpace.y * mapScaleFactor.y);
    headProjectionPos.x = anchorMS.x - (dispMapInfo->Anchor_MapSpace.x * mapScaleFactor.x);

    mMapProjectionCamera->SetAspectRatio(1.0f);
    mMapProjectionCamera->SetWidth(2.0f * mapScaleFactor.x);
    mMapProjectionCamera->SetHeight(2.0f * mapScaleFactor.y);
    mMapProjectionCamera->SetPosition(headProjectionPos.x, headProjectionPos.y, -10.0f);
    mMapProjectionCamera->LookAt(headProjectionPos.x, headProjectionPos.y, 0.0f);
    mMapProjectionCamera->Update();
    float4x4 viewMatrix = *mMapProjectionCamera->GetViewMatrix();
    float4x4 projMatrix = *mMapProjectionCamera->GetProjectionMatrix();
    float4x4 deviceToTex = float4x4Scale(0.5f, -0.5f, 0.0f) * float4x4Translation(0.5f,0.5f,0.0f); // device normal coordinates to texture coords
    outProjInfo->ViewProjTexMatrix = viewMatrix * projMatrix * deviceToTex;

    // 5 units in model space. This is a hardcoded offset from the tip of the nose. This could be specified with a model landmark
    float extrudeMaxZ = 7;
    float headMSUnitsPerRSUnitsZNormalized = zScaleRSToMS / dispMapInfo->DepthMap_ZRange;
    float extrudeBaseZ = noseTip.z + extrudeMaxZ;

    // max is nose tip
    outProjInfo->ExtrudeMinZ = extrudeBaseZ + zDisplaceOffset;
    outProjInfo->ExtrudeMaxZ = outProjInfo->ExtrudeMinZ - extrudeMaxZ* scale;
    outProjInfo->DepthMapRangeMin = dispMapInfo->DepthMap_ZMeshStart - extrudeMaxZ * headMSUnitsPerRSUnitsZNormalized;
    outProjInfo->DepthMapRangeMax = dispMapInfo->DepthMap_ZMeshStart;


    float4x4 mapToHead = float4x4Scale( mapScaleFactor.x, -mapScaleFactor.y, 1.0f);
    mapToHead = mapToHead * float4x4Translation(headProjectionPos.x, headProjectionPos.y, 0.0f);
    outProjInfo->MapToHeadSpaceTransform = mapToHead;
}

static void ApplyMorphTargets(std::vector<MorphTargetEntry> &entries, CPUTSoftwareMesh *mesh, bool post)
{
    // Apply all the morph targets
    int vertCount = mesh->GetVertCount();
    for (auto it = entries.begin(); it != entries.end(); ++it) {
        if (it->Post == post)
        {
            CMorphTarget *morphTarget = it->Target;
            for (auto vit = morphTarget->MorphVerts.begin(); vit != morphTarget->MorphVerts.end(); ++vit)
            {
                assert(vit->VertIndex < vertCount);
                mesh->Pos[vit->VertIndex] += vit->Delta * it->Weight;
                mesh->Normal[vit->VertIndex] += vit->NormalDelta * it->Weight;
            }
        }
    }

    for (int i = 0; i < vertCount; i++)
        mesh->Normal[i].normalize();
}

void CalculateMeshNormals(float3 *positions, uint32 *faces, int vertexCount, int faceCount, float3 *outNormals)
{
    ZeroMemory(outNormals, sizeof(float3) *vertexCount);
    for (int i = 0; i < faceCount; i++)
    {
        uint32 idx1 = faces[i * 3];
        uint32 idx2 = faces[i * 3 + 1];
        uint32 idx3 = faces[i * 3 + 2];

        float3 posA = positions[idx1];
        float3 posB = positions[idx2];
        float3 posC = positions[idx3];

        float3 vecAtoB = posB - posA;
        float3 vecBtoC = posC - posB;

        float3 normal = cross3(vecAtoB, vecBtoC).normalize();

        outNormals[idx1] += normal;
        outNormals[idx2] += normal;
        outNormals[idx3] += normal;
    }

    for (int i = 0; i < vertexCount; i++)
    {
        outNormals[i] = outNormals[i].normalize();
    }
}

inline float RemapRange(float value, float r1Min, float r1Max, float r2Min, float r2Max)
{
    float ratio = (value - r1Min) / (r1Max - r1Min);
    ratio = floatClamp(ratio, 0.0f, 1.0f);
    return r2Min + ratio * (r2Max - r2Min);
}

void CHeadGeometryStage::updateLandmarksToMorphedMeshVerticesMapItem(int landmarkIndex, int morphedMeshVertexIdx, float distance)
{
    if(landmarkIndex>=0 && landmarkIndex < LandmarkIdxToMorphedMeshVertIdx.size()
            && distance < LandmarkIdxToMorphedMeshVertIdx[landmarkIndex].second){
        LandmarkIdxToMorphedMeshVertIdx[landmarkIndex].first = morphedMeshVertexIdx;
        LandmarkIdxToMorphedMeshVertIdx[landmarkIndex].second = distance;
    }
}

//TODO: go on with other landmarks and check for buggy values.
void CHeadGeometryStage::updateMorphVsScanDeltas(const std::vector<float2> &mapLandmarks, const float4x4& mapToHeadSpaceTransform, const CPUTSoftwareMesh *dstMesh)
{
    auto diffVecBaseScan = [&] (int lIdx1, int lIdx2){
        float3 vec1 = float4(mapLandmarks[lIdx1].x, mapLandmarks[lIdx1].y, 0.0f, 1.0f) * mapToHeadSpaceTransform;
        float3 vec2 = float4(mapLandmarks[lIdx2].x, mapLandmarks[lIdx2].y, 0.0f, 1.0f) * mapToHeadSpaceTransform;
        return vec1 - vec2;
    };

    auto diffVecMorphedHead = [&] (int lIdx1, int lIdx2){
        float3 vec1 = dstMesh->Pos[LandmarkIdxToMorphedMeshVertIdx[lIdx1].first];
        float3 vec2 = dstMesh->Pos[LandmarkIdxToMorphedMeshVertIdx[lIdx2].first];
        return vec1 - vec2;
    };

    float chinHeightOnBaseScan = abs(diffVecBaseScan(kLandmarkIndex_LowerLipTop, kLandmarkIndex_ChinBottom).y);
    float chinHeightOnMorphedHead = abs(diffVecMorphedHead(kLandmarkIndex_LowerLipTop, kLandmarkIndex_ChinBottom).y);
    MorphVsScanChinHeightDelta = (chinHeightOnMorphedHead - chinHeightOnBaseScan)/chinHeightOnBaseScan;

    float faceWidthOnBaseScan = abs(diffVecBaseScan(kLandmarkIndex_FaceRight, kLandmarkIndex_FaceLeft).x);
    float faceWidthOnMorphedHead = abs(diffVecMorphedHead(kLandmarkIndex_FaceRight, kLandmarkIndex_FaceLeft).x);
    MorphVsScanFaceWidthDelta = (faceWidthOnMorphedHead - faceWidthOnBaseScan)/faceWidthOnBaseScan;

    float lipsSpacingOnBaseScan = abs(diffVecBaseScan(kLandmarkIndex_UpperLipBottom, kLandmarkIndex_LowerLipTop).y);
    float lipsSpacingOnMorphedHead = abs(diffVecMorphedHead(kLandmarkIndex_UpperLipBottom, kLandmarkIndex_LowerLipTop).y);
    MorphVsScanLipsSpacingDelta = (lipsSpacingOnMorphedHead - lipsSpacingOnBaseScan)/lipsSpacingOnBaseScan;

    float chinWidthOnBaseScan = abs(diffVecBaseScan(kLandmarkIndex_ChinLeft, kLandmarkIndex_ChinRight).x);
    float chinWidthOnMorphedHead = abs(diffVecMorphedHead(kLandmarkIndex_ChinLeft, kLandmarkIndex_ChinRight).x);
    MorphVsScanChinWidthDelta = (chinWidthOnMorphedHead - chinWidthOnBaseScan)/chinWidthOnBaseScan;
}

void CHeadGeometryStage::updateMorphedLandmarkMesh(CPUTSoftwareMesh* landmarkMesh, const std::vector<float2>& mapLandmarks, const float4x4& mapToHeadSpaceTransform)
{
    MorphedLandmarkMesh.CopyFrom(landmarkMesh);
    // Shift the landmark mesh to match the face landmarks
    for (int i = 0; i < landmarkMesh->GetVertCount(); i++)
    {
        int idx = mLandmarkMeshVertexToLandmarkIndex[i];
        if (idx != -1)
        {
            float2 lmMapPos = mapLandmarks[idx];
            float4 pos = float4(lmMapPos.x, lmMapPos.y, MorphedLandmarkMesh.Pos[i].z, 1.0f);
            MorphedLandmarkMesh.Pos[i] = pos * mapToHeadSpaceTransform;
        }
    }
}

void CHeadGeometryStage::updateLandmarkMeshVertexToLandmarkIndexMap(const CPUTSoftwareMesh* landmarkMesh, const std::vector<float3>& baseHeadLandmarks, const std::vector<float2>& mapLandmarks)
{
    mLandmarkMeshVertexToLandmarkIndex.clear();
    int lmVertCount = landmarkMesh->GetVertCount();

    for (int i = 0; i < lmVertCount; i++)
    {
        mLandmarkMeshVertexToLandmarkIndex.push_back(-1);
        float closestDistance = FLT_MAX;
        for (int j = 0; j < baseHeadLandmarks.size(); j++)
        {
            float3 tmp = landmarkMesh->Pos[i];
            float3 vertexToLandmark = landmarkMesh->Pos[i] - baseHeadLandmarks[j];
            vertexToLandmark.z = 0.0f; // Ignore depth (i.e., project landmark onto landmark mesh plane)
            float distance = abs(vertexToLandmark.length());
            if (distance < 1.0f && distance < closestDistance
                    && mapLandmarks[j].x!=FLT_MAX) // avoid mapping a landmark if the target on the scan is invalid.
            {
                closestDistance = distance;
                mLandmarkMeshVertexToLandmarkIndex[i] = j;
            }
        }
    }
}

void CHeadGeometryStage::updateLandmarksToMorphedMeshVerticesMap()
{
    for (int vIdx = 0; vIdx < mMappedFaceVertices.size(); vIdx++) {
        float dd = mMappedFaceVertices[vIdx].ClosestDistance;
        if (dd < 10.0) {//avoid mapping vertices which are behind the head or at FLT_MAX distance.
            float3 barys = mMappedFaceVertices[vIdx].BarycentricCoordinates;
            int lIdx = mMappedFaceVertices[vIdx].TriangleIndex;   // headVertex[hIdx] projects onto landmarkMeshTriangle[lIdx]
            assert(lIdx < MorphedLandmarkMesh.IndexBufferCount);
            UINT i0 = MorphedLandmarkMesh.IB[lIdx * 3 + 0];
            UINT i1 = MorphedLandmarkMesh.IB[lIdx * 3 + 1];
            UINT i2 = MorphedLandmarkMesh.IB[lIdx * 3 + 2];

            updateLandmarksToMorphedMeshVerticesMapItem(mLandmarkMeshVertexToLandmarkIndex[i0], vIdx, 1.-barys.x);
            updateLandmarksToMorphedMeshVerticesMapItem(mLandmarkMeshVertexToLandmarkIndex[i1], vIdx, 1.-barys.y);
            updateLandmarksToMorphedMeshVerticesMapItem(mLandmarkMeshVertexToLandmarkIndex[i2], vIdx, 1.-barys.z);
        }
    }
}

void CHeadGeometryStage::fitDeformedMeshToFace(CPUTSoftwareTexture *controlMapColor)
{
    for (int vIdx = 0; vIdx < mMappedFaceVertices.size(); vIdx++)
    {
        float dd = mMappedFaceVertices[vIdx].ClosestDistance;
        if (dd != FLT_MAX)
        {
            int lIdx = mMappedFaceVertices[vIdx].TriangleIndex;   // headVertex[hIdx] projects onto landmarkMeshTriangle[lIdx]
            assert(lIdx < MorphedLandmarkMesh.IndexBufferCount);
            UINT i0 = MorphedLandmarkMesh.IB[lIdx * 3 + 0];
            UINT i1 = MorphedLandmarkMesh.IB[lIdx * 3 + 1];
            UINT i2 = MorphedLandmarkMesh.IB[lIdx * 3 + 2];

            float3 v0 = MorphedLandmarkMesh.Pos[i0];
            float3 v1 = MorphedLandmarkMesh.Pos[i1];
            float3 v2 = MorphedLandmarkMesh.Pos[i2];

            float3 barys = mMappedFaceVertices[vIdx].BarycentricCoordinates;
            float3 newPos = v0*barys.x + v1*barys.y + v2*barys.z;
            newPos.z += dd;

            CPUTColor4 samp(0.0f, 0.0f, 0.0f, 0.0f);
            controlMapColor->SampleRGBAFromUV(DeformedMesh.Tex[vIdx].x, DeformedMesh.Tex[vIdx].y, &samp);
            DeformedMesh.Pos[vIdx] = DeformedMesh.Pos[vIdx] * (1.0f - samp.r) + newPos * samp.r;
        }
    }
}

void CHeadGeometryStage::blendDeformedMeshZAndTexture(const HeadProjectionInfo &hpi, CPUTSoftwareTexture *controlMapDisplacement, CPUTSoftwareTexture* displacementMap, bool skipDisplacementMap)
{
    CPUTColor4 dispSample(0.0f, 0.0f, 0.0f, 0.0f);
    CPUTColor4 controlSample(0.0f, 0.0f, 0.0f, 0.0f);

    for (int i = 0; i < DeformedMesh.GetVertCount(); i++)
    {
        float3 pos = DeformedMesh.Pos[i];

        // project displacement map and color map
        float3 projectedUV = float4(pos, 1.0f) * hpi.ViewProjTexMatrix;

        displacementMap->SampleRGBAFromUV(projectedUV.x, projectedUV.y, &dispSample);

        float displacedZ = RemapRange(dispSample.r, hpi.DepthMapRangeMin, hpi.DepthMapRangeMax, hpi.ExtrudeMinZ, hpi.ExtrudeMaxZ);

        controlMapDisplacement->SampleRGBAFromUV(DeformedMesh.Tex[i].x, DeformedMesh.Tex[i].y, &controlSample);

        // Blend between displaced Z and default model z based on the control texture's red value
        if (!skipDisplacementMap)
            pos.z = floatLerp(pos.z, displacedZ, controlSample.r);

        DeformedMesh.Tex2[i] = float2(projectedUV.x, projectedUV.y);
        DeformedMesh.Pos[i] = pos;
    }
}

void CHeadGeometryStage::updateDeformedMeshNormals(CPUTSoftwareTexture *controlMapDisplacement)
{
    CPUTColor4 controlSample(0.0f, 0.0f, 0.0f, 0.0f);
    float3 *tempNormals = (float3*)malloc(sizeof(float3) * DeformedMesh.GetVertCount());
    CalculateMeshNormals(DeformedMesh.Pos, DeformedMesh.IB, DeformedMesh.GetVertCount(), DeformedMesh.IndexBufferCount / 3, tempNormals);
    for (int i = 0; i < DeformedMesh.GetVertCount(); i++)
    {
        controlMapDisplacement->SampleRGBAFromUV(DeformedMesh.Tex[i].x, DeformedMesh.Tex[i].y, &controlSample);
        DeformedMesh.Normal[i] = tempNormals[i] * controlSample.r + DeformedMesh.Normal[i] * (1.0f - controlSample.r);
        DeformedMesh.Normal[i] = DeformedMesh.Normal[i].normalize();
    }
    free(tempNormals);
}

void CHeadGeometryStage::readjustConnectedVertices()
{
    for(const auto& vertices: mConnectedVertices){
        int vIdx1 = vertices.first;

        float3 newPos = DeformedMesh.Pos[vIdx1];
        for(int vIdx2: vertices.second){
            newPos += DeformedMesh.Pos[vIdx2];
            newPos /= 2.;
        }

        DeformedMesh.Pos[vIdx1] = newPos;
        for(int vIdx2: vertices.second)
            DeformedMesh.Pos[vIdx2] = newPos;

    }
}

void CHeadGeometryStage::updateMapOfConnectedVertices(const CPUTSoftwareMesh *mesh)
{
    int vertCount = mesh->GetVertCount();

    for(int i=0; i< vertCount; ++i)
        for(int j = i+1; j < vertCount; ++j)
            if(mesh->Pos[i] == mesh->Pos[j])
                mConnectedVertices[i].push_back(j);
}

void CHeadGeometryStage::resetMapOfLandmarkIdxToMorphedMeshVertIdx(const CPUTSoftwareMesh *landmarkMesh)
{
    int landmarksCount = landmarkMesh->GetVertCount();
    if(LandmarkIdxToMorphedMeshVertIdx.size()!=landmarksCount){
        LandmarkIdxToMorphedMeshVertIdx.clear();
        LandmarkIdxToMorphedMeshVertIdx.reserve(landmarksCount);
        for (int i = 0; i < landmarksCount; i++)
        {
            LandmarkIdxToMorphedMeshVertIdx.push_back(std::make_pair(-1,FLT_MAX));
        }
    }
}

void CHeadGeometryStage::Execute(SHeadGeometryStageInput *input)
{
    assert(input->BaseHeadInfo->BaseHeadMesh.Pos);
    assert(input->BaseHeadInfo->BaseHeadMesh.Tex);

    CPUTSoftwareTexture *controlMapDisplacement = ((CPUTTextureDX11*)input->BaseHeadInfo->Textures[eBaseHeadTexture_ControlMap_Displacement])->GetSoftwareTexture(false, true);
    CPUTSoftwareTexture *controlMapColor = ((CPUTTextureDX11*)input->BaseHeadInfo->Textures[eBaseHeadTexture_ControlMap_Color])->GetSoftwareTexture(false, true);

    // First, correlate landmark mesh vertices to landmarks
    // Potentially difficult to force vertex indices from authoring package (e.g., 3dsMax).
    // Easier to force order of assets in asset set (note, 3dsMax appears to use selection order).
    // At worst, can manually edit asset.set file.
    // So, search landmark asset set for landmark at position of each vertex
    // and discard mapping to invalid landmarks.
    updateLandmarkMeshVertexToLandmarkIndexMap(&input->BaseHeadInfo->LandmarkMesh, input->BaseHeadInfo->BaseHeadLandmarks, input->DisplacementMapInfo->MapLandmarks);

    HeadProjectionInfo hpi;
    updateHeadProjectionInfo(input->DisplacementMapInfo, input->BaseHeadInfo, input->Scale, input->ZDisplaceOffset, &hpi);
    updateMorphedLandmarkMesh(&input->BaseHeadInfo->LandmarkMesh, input->DisplacementMapInfo->MapLandmarks, hpi.MapToHeadSpaceTransform);

    if (mMappedFaceVertices.size() != input->BaseHeadInfo->BaseHeadMesh.GetVertCount()) { //do these things only once
        // Project face vertices onto original landmark mesh
        ProjectMeshVerticesOntoMeshTriangles(&input->BaseHeadInfo->BaseHeadMesh, &input->BaseHeadInfo->LandmarkMesh, mMappedFaceVertices);

        resetMapOfLandmarkIdxToMorphedMeshVertIdx(&input->BaseHeadInfo->LandmarkMesh);

        updateMapOfConnectedVertices(&input->BaseHeadInfo->BaseHeadMesh);
    }

    DeformedMesh.CopyFrom(&input->BaseHeadInfo->BaseHeadMesh);
    DeformedMesh.AddComponent(eSMComponent_Tex2);

    ApplyMorphTargets(input->MorphTargetEntries, &DeformedMesh, false);

    if ((input->Flags & PIPELINE_FLAG_SkipFitFace) == 0)
        fitDeformedMeshToFace(controlMapColor);

    blendDeformedMeshZAndTexture(hpi, controlMapDisplacement, input->DisplacementMap, (input->Flags & PIPELINE_FLAG_SkipDisplacementMap)!=0);

    readjustConnectedVertices();

    updateLandmarksToMorphedMeshVerticesMap();
    updateMorphVsScanDeltas(input->DisplacementMapInfo->MapLandmarks, hpi.MapToHeadSpaceTransform, &DeformedMesh);

    // Apply all the post morph targets
    ApplyMorphTargets(input->MorphTargetEntries, &DeformedMesh, true);

    //    if (input->OtherHeadBlend > 0.0f && input->OtherHeadMesh != NULL)
    //    {
    //        for (int i = 0; i < base->GetVertCount(); i++)
    //        {
    //            DeformedMesh.Pos[i] = DeformedMesh.Pos[i] + (input->OtherHeadMesh->Pos[i] - DeformedMesh.Pos[i]) * input->OtherHeadBlend;
    //        }
    //    }

    // Fix up normals now that we have imprinted the face
    updateDeformedMeshNormals(controlMapDisplacement);

}

