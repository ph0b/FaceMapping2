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
#ifndef __MENU_FACEMAPPING__
#define __MENU_FACEMAPPING__

#include "CFaceModel.h"
#include <vector>
#include "CPUT.h"
#include "CPUTRenderTarget.h"
#include "CPUTSprite.h"
#include "CPUTScene.h"
#include "CPUTCamera.h"
#include "CPUTText.h"
#include "CPUTModel.h"
#include "CPUTSoftwareMesh.h"
#include "FaceMap/CPipeline.h"

enum DebugTextureView
{
    DebugTextureView_None,
    DebugTextureView_DisplacementColor,
    DebugTextureView_DisplacementDepth,
    DebugTextureView_FinalHeadDiffuse,
    DebugTextureView_Count
};

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

class FaceMapping
{
public:
        void Init();
        void Shutdown();

    void HandleCPUTEvent(int eventID, int controlID, CPUTControl *control);
        CPUTEventHandledCode HandleKeyboardEvent(CPUTKey key, CPUTKeyState state);

    void Update(float dt);
         CPUTEventHandledCode HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state, CPUTEventID message);

    void LoadFace(const std::string &filename);

    void Render(CPUTRenderParameters &renderParams);

    bool IsFaceLoaded();

        void SetDefaultTweaks();
        void SetHairFromIndex(int hairIndex);


private:

    void SetDefaultDebug();

    void ResetTweaks();
    void ResetActiveMorphTargets(bool post);

        void loadHair(CPUTAssetSet *set, const char *modelName, const char *displayName );
        void loadBeardPart(CPUTAssetSet *beardSet, const char *modelName, const char *displayName);

    void UpdateLayout(CPUTRenderParameters &renderParams);
    void CreateMorphTargetEntries(std::vector<MorphTargetEntry> &list, std::vector<SMorphTweakParamDef> &defs, std::vector<float> &weights, bool post);
        void addMorphParam(const char *category, const char *name, float defaultValue, const char *modelName, float range1, float range2, float apply1, float apply2);

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
    bool mShowMapLandmarks;
    bool mUseOrthoCamera;
    bool mFullscreenDebugTextureViewer;
    DebugTextureView mDebugTextureView;
    DebugHeadDisplayTexture mDebugHeadDisplayTextureView;
    bool mRenderLandmarkMesh;
    bool mRenderMorphedLandmarkMesh;
    bool mRenderHeadLandmarks;

    CPUTSoftwareMesh mOtherHead;
    CPUTTexture *mOtherHeadTexture;

    CPUTSprite *mDebugTextureSprite;

    // Lighting
    float mDirectionalLightHeight;
    float mDirectionalLightAngle;
    float mDirectionalLightIntensity;
    float mAmbientLightIntensity;

        void loadLandmarkSet(CPUTAssetSet *landmarkSet);
        void loadCubeMap(std::string mediaDir);
        void loadHeadModelAndAssets(std::string mediaDir);
        void loadHeadTextures(std::string headDir);
        void addMorphParameters(CPUTAssetSet* headSet);
        void loadMorphTargets(CPUTAssetSet* headSet);
};

#endif