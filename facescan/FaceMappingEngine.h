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
#ifndef __FACEMAPPINGENGINE__
#define __FACEMAPPINGENGINE__

#include <D3D11.h>
#include <stdio.h>
#include <time.h>
#include <vector>

#include "CPUT.h"
#include "CPUT_DX11.h"
#include "CPUTBufferDX11.h"

#include "CPUTRenderTarget.h"
#include "CPUTSprite.h"
#include "CPUTParser.h"
#include "CPUTTextureDX11.h"
#include "CPUTScene.h"
#include "CPUTCamera.h"
#include "CPUTModel.h"
#include "CPUTSoftwareMesh.h"

#include "CFaceModel.h"
#include "FaceMap/CPipeline.h"

enum DebugHeadDisplayTexture
{
    DebugHeadDisplayTexture_None,
    DebugHeadDisplayTexture_DisplacmentControlMap,
    DebugHeadDisplayTexture_ColorControlMap,
    DebugHeadDisplayTexture_FeatureMap,
    DebugHeadDisplayTexture_ColorTransferMap,
    DebugHeadDisplayTexture_SkinMap,
    DebugHeadDisplayTexture_Count
};

struct SMorphTweakParamPart
{
    std::string MorphTargetName;
    float ParamRange0;
    float ParamRange1;
    float Apply0;
    float Apply1;


    SMorphTweakParamPart(std::string morphTargetName, float paramRange0, float paramRange1, float apply0, float apply1)
    {
        MorphTargetName = morphTargetName;
        ParamRange0 = paramRange0;
        ParamRange1 = paramRange1;
        Apply0 = apply0;
        Apply1 = apply1;
    }
};

struct SHairDef
{
    SHairDef(CPUTModel *model, const char *name, CPUTSoftwareMesh*swmesh)
    {
        Model = model;
        Name = name;
        SWMesh = swmesh;
    }
    SHairDef()
    {
        Model = NULL;
        SWMesh = NULL;
    }

    CPUTModel *Model;
    std::string Name;
    CPUTSoftwareMesh *SWMesh;
};

struct SMorphTweakParamDef
{
    std::string Name;
    std::string Category;
    std::vector<SMorphTweakParamPart> MorphParts;
    float Default;

    void Reset(std::string category, std::string name, float defaultValue)
    {
        Name = name;
        Category = category;
        Default = defaultValue;
        MorphParts.clear();
    }
};

// data that goes along with the face maps providing information about how to do the projection
struct FaceMapInfo
{
    // distance between the left and right eye center in the face model space and map space
    float EyeDistance_FaceModelSpace;
    float EyeDistance_MapSpace;

    // coordinates of the anchor point projected onto the map textures x: -1 to 1, y: -1 to 1
    float2 Anchor_MapSpace;

    // the depth values in the depth image will be in range 0-1 DepthMap_ZRange is the Z distance that
    // range represents in world space. AKA: farClip - nearClip
    float DepthMap_ZRange;

    // The z value at which the face mesh starts. The near clipping plane is shifted away from the face to avoid artifacts
    // with near plane clipping
    float DepthMap_ZMeshStart;
};


struct HairInfo
{
    const char *name;
    CPUTAssetSet *set;
};

class FaceMappingEngine: public CPUT_DX11
{
public:
    FaceMappingEngine();

    virtual void Create();
    virtual void Update(double dt);
    virtual void Render(double deltaSeconds);
    virtual void ResizeWindow(UINT width, UINT height);
    virtual void Shutdown();

    virtual CPUTEventHandledCode HandleKeyboardEvent(CPUTKey key, CPUTKeyState state);
    virtual CPUTEventHandledCode HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state, CPUTEventID message);

    QWidget& GetQWidget();

    void LoadContent();

    void LoadFace(const std::string &filename);

    bool IsFaceLoaded();

    void SetDefaultTweaks();
    void SetHairIndex(int hairIndex);
    void SetBeardIndex(int beardIndex, bool enable);

    void setMorphParamWeight(int idx, float value);//TODO: switch to map and keys instead of vectors and indexes.

    void setPostMorphParamWeight(int idx, float value);

    void SetFaceOrientation(float yaw, float pitch, float roll);
    void SetBlendColor1(float r, float g, float b);
    void SetBlendColor2(float r, float g, float b);
    void SetFaceZOffset(float offset);

    void ExportOBJTo(std::string outFilename);

protected:
    void loadLandmarkSet(CPUTAssetSet *landmarkSet);
    void loadCubeMap(std::string mediaDir);
    void loadHeadModelAndAssets(std::string mediaDir);
    void loadHeadTextures(std::string headDir);
    void addMorphParameters(CPUTAssetSet* headSet);
    void loadMorphTargets(CPUTAssetSet* headSet);

    CPUTTexture *LoadTexture(std::string &dir, const char *filename);
    void SetCodeTexture(int index, CPUTTexture *texture);
    void SetCodeTexture(int index, ID3D11ShaderResourceView *srv);

    void LoadCPUTModelToSWMesh(CPUTAssetSet *set, const char *modelName, CPUTSoftwareMesh *outMesh);

    void SetDefaultDebug();

    void ResetTweaks();
    void ResetActiveMorphTargets(bool post);

    void loadHair(CPUTAssetSet *set, const char *modelName, const char *displayName );
    void loadBeardPart(CPUTAssetSet *beardSet, const char *modelName, const char *displayName);

    void UpdateLayout(CPUTRenderParameters &renderParams);
    void CreateMorphTargetEntries(std::vector<MorphTargetEntry> &list, std::vector<SMorphTweakParamDef> &defs, std::vector<float> &weights, bool post);
    void addMorphParam(const char *category, const char *name, float defaultValue, const char *modelName, float range1, float range2, float apply1, float apply2);
    void addMorphParam(const char *category, const char *name, float defaultValue, const char *modelName, float range1, float range2, float apply1, float apply2, const char *modelName_2, float range1_2, float range2_2, float apply1_2, float apply2_2);


private:
    static const UINT SHADOW_WIDTH_HEIGHT = 2048;
    static const int kCodeTexturesCount = 8;

    QDXWidget             *mpQDXWidget;
    CPUTSprite			*mpFullscreenSprite;

    CPUTTextureDX11 *mCodeTextures[kCodeTexturesCount];

    float                  mfElapsedTime;
    CPUTCameraController  *mpCameraController;
    CPUTAssetSet          *mpShadowCameraSet;
    CPUTRenderTargetDepth *mpShadowRenderTarget;

    float2 mViewportDim;
    float mImGUIMenuWidth;

    CFaceModel mFaceModel;

    int3 mPostBlendColorize[2];
    int3 mPostBlendAdjust[2];
    MappingTweaks mTweaks;
    MappingTweaks mLastTweaks;

    CPUTScene *mHeadAssetScene;

    CPUTCameraController *mCameraController;

    std::string mObjFilename;

    CPUTModel *mDisplayHead;

    CPUTModel *mCPUTLandmarkModel;

    CPUTAssetSet *mpCubemap;

    std::vector<SMorphTweakParamDef> mMorphParamDefs;
    std::vector<float> mActiveMorphParamWeights;

    std::vector<SMorphTweakParamDef> mPostMorphParamDefs;
    std::vector<float> mActivePostMorphParamWeights;

    std::map<std::string, CMorphTarget *> mMorphTargetMap;
    CPUTSoftwareMesh mBaseMesh;

    SBaseHeadInfo mHeadInfo;

    std::vector<SHairDef> mHairDefs;
    CPUTSoftwareMesh mCurrentHair;

    // Beard
    std::vector<SHairDef> mBeardDefs;
    std::vector<bool> mBeardEnabled;
    CPUTSoftwareMesh mFinalBeard;

    CPipeline mPipeline;
    CPipelineOutput mPipelineOutput;

    bool mForceRebuildAll;

    bool mSkipFaceFit;
    bool mSkipFaceDisplace;
    bool mSkipFaceColorBlend;
    bool mSkipSeamFill;

    int mCurrentHairIndex;
    bool mHideCubeMap;
    bool mShowWireframe;
    bool mRenderLandmarkMesh;
    bool mRenderMorphedLandmarkMesh;

    CPUTSoftwareMesh mOtherHead;
    CPUTTexture *mOtherHeadTexture;

    DebugHeadDisplayTexture mDebugHeadDisplayTextureView;

    // Lighting
    float mDirectionalLightHeight;
    float mDirectionalLightAngle;
    float mAmbientLightIntensity;
};

#endif
