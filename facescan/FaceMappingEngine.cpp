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
#include "FaceMappingEngine.h"
#include "CPUTModel.h"
#include "CPUTMaterial.h"
#include "CPUTSprite.h"
#include "CPUTScene.h"
#include "CPUT_DX11.h"

#include <string>
#include "CPUTVertexShaderDX11.h"
#include "CPUTGeometryShaderDX11.h"
#include "CPUTSoftwareMesh.h"
#include "FaceMap\FaceMapUtil.h"
#include "facemap\CDisplacementMapStage.h"
#include "facemap\CHeadGeometryStage.h"
#include "facemap\CHeadBlendStage.h"
#include "OBJExporter.h"

#include <DXGItype.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <DXProgrammableCapture.h>

const char *sPostBlendColorModes[] =
{
    "None",
    "Colorize",
    "Adjust",
};
static int cAssert3 = 3 / ((ARRAYSIZE(sPostBlendColorModes) == PostBlendColorMode_Count) ? 1 : 0);

FaceMappingEngine::FaceMappingEngine():mpQDXWidget(NULL),
    mpFullscreenSprite(NULL),
    mfElapsedTime(0.0),
    mpCameraController(NULL),
    mpShadowCameraSet(NULL),
    mpShadowRenderTarget(NULL)
{

}

void FaceMappingEngine::SetDefaultTweaks()
{
    mTweaks.Scale = 1.0f;
    mTweaks.FaceYaw = 0.0f;
    mTweaks.FacePitch = 0.0f;
    mTweaks.FaceRoll = 0.0f;
    mTweaks.OutputTextureResolution = 2048;
    mTweaks.DisplaceOffset = float3(0.0f, 0.0f, -3.5f);
    mTweaks.BlendColor1 = CPUTColorFromBytes(228, 194, 171, 255);
    mTweaks.BlendColor2 = CPUTColorFromBytes(205, 50, 50, 255);
    mTweaks.PostBlendAdjust[0] = mTweaks.PostBlendAdjust[1] = int3(0, 0, 0);
    mTweaks.PostBlendColorize[0] = mTweaks.PostBlendColorize[1] = int3(180, 50, 0);
    mTweaks.PostBlendMode = PostBlendColorMode_None;
    mTweaks.OtherHeadBlend = 0.0f;
    mTweaks.OtherHeadTexture = NULL;
    mTweaks.OtherHeadMesh = NULL;
}

void FaceMappingEngine::SetDefaultDebug()
{
    mShowMapLandmarks = true;
    mRenderLandmarkMesh = false;
    mRenderMorphedLandmarkMesh = false;
    mRenderHeadLandmarks = false;
    mShowWireframe = false;
    mUseOrthoCamera = false;
    mSkipFaceFit = false;
    mHideCubeMap = false;
    mSkipFaceDisplace = false;
    mSkipFaceColorBlend = false;
    mSkipSeamFill = false;

    mDirectionalLightHeight = 0.0f;
    mDirectionalLightAngle = 0.0f;
    mDirectionalLightIntensity = 0.7f;
    mAmbientLightIntensity = 0.3f;

}

void FaceMappingEngine::loadLandmarkSet(CPUTAssetSet *landmarkSet)
{
    int landmarkCount = landmarkSet->GetAssetCount();
    for (int i = 1; i < landmarkCount; i++) // skip root node
    {
        CPUTRenderNode *node = NULL;
        landmarkSet->GetAssetByIndex(i, &node);
        mHeadInfo.BaseHeadLandmarks.push_back(node->GetPosition());
        int lmIdx = i - 1;
        if (lmIdx >= kLandmarkIndex_FaceOutlineStart && lmIdx <= kLandmarkIndex_FaceOutlineEnd)
        {
            // Shift the face outline landmarks so they are don't get used by the algorithm
            mHeadInfo.BaseHeadLandmarks[lmIdx].x += 100.0f;
        }
        SAFE_RELEASE(node);
    }
}

void FaceMappingEngine::loadCubeMap(std::string mediaDir)
{
    {
        std::string skyDir = mediaDir + "/cubeMap_01/";
        CPUTAssetLibrary* pAssetLibrary = CPUTAssetLibrary::GetAssetLibrary();
        pAssetLibrary->SetMediaDirectoryName(skyDir);
        mpCubemap = pAssetLibrary->GetAssetSet("cubeMap_01");
        pAssetLibrary->SetMediaDirectoryName(mediaDir + "\\");
    }
}

void FaceMappingEngine::loadHeadModelAndAssets(std::string mediaDir)
{
    CPUTAssetLibrary *pAssetLibrary = CPUTAssetLibrary::GetAssetLibrary();

    pAssetLibrary->SetMediaDirectoryName(mediaDir+"\\");
    mDisplayHead = pAssetLibrary->FindModel("templateHeadModel", true);

    CPUTFileSystem::CombinePath(mediaDir, "headassets.scene", &mediaDir);
    mHeadAssetScene = CPUTScene::Create();
    mHeadAssetScene->LoadScene(mediaDir);

    CPUTAssetSet *landmarkSet = mHeadAssetScene->GetAssetSet(0);
    loadLandmarkSet(landmarkSet);

    CPUTAssetSet *landmarkMeshSet = mHeadAssetScene->GetAssetSet(1);
    // Load landmark mesh
    landmarkMeshSet->GetAssetByIndex(1, (CPUTRenderNode**)&mCPUTLandmarkModel);
    mHeadInfo.LandmarkMesh.CopyFromDX11Mesh((CPUTMeshDX11*)mCPUTLandmarkModel->GetMesh(0));
    mHeadInfo.LandmarkMesh.ApplyTransform(mCPUTLandmarkModel->GetWorldMatrix());

    CPUTAssetSet *headSet = mHeadAssetScene->GetAssetSet(2);
    LoadCPUTModelToSWMesh(headSet, "Base_Head.mdl", &mBaseMesh);

    loadHair(NULL, NULL, "Hairless");

    CPUTAssetSet *hairShortSet = mHeadAssetScene->GetAssetSet(3);
    loadHair(hairShortSet, "ShortHair.mdl", "Short");

    CPUTAssetSet *hairMediumSet = mHeadAssetScene->GetAssetSet(4);
    loadHair(hairMediumSet, "HairMedium.mdl", "Medium");

    CPUTAssetSet *hairLongSet = mHeadAssetScene->GetAssetSet(5);
    loadHair(hairLongSet, "Long_Hair.mdl", "Long");

    CPUTAssetSet *hairHelmet1Set = mHeadAssetScene->GetAssetSet(6);
    loadHair(hairHelmet1Set, "hair1_retopo.mdl", "Helmet Short");

    CPUTAssetSet *hairHelmet2Set = mHeadAssetScene->GetAssetSet(7);
    loadHair(hairHelmet2Set, "hair2.mdl", "Helmet 2");

    CPUTAssetSet *hairHelmet3Set = mHeadAssetScene->GetAssetSet(8);
    loadHair(hairHelmet3Set, "hair3.mdl", "Helmet 3");

    CPUTAssetSet *hairHelmet4Set = mHeadAssetScene->GetAssetSet(9);
    loadHair(hairHelmet4Set, "hair4.mdl", "Helmet 4");

    CPUTAssetSet *beardSet = mHeadAssetScene->GetAssetSet(10);
    loadBeardPart(beardSet, "Chops.mdl", "Chops");
    loadBeardPart(beardSet, "moustache.mdl", "Mustache");
    loadBeardPart(beardSet, "SideBurns.mdl", "Sideburns");
    loadBeardPart(beardSet, "SoulPatch.mdl", "Soulpatch");
    loadBeardPart(beardSet, "goatee.mdl", "Goatee");

    mDisplayHead = pAssetLibrary->FindModel("templateHeadModel", true);
}

void FaceMappingEngine::loadHeadTextures(std::string headDir)
{
    mHeadInfo.BaseHeadMesh = &mBaseMesh;
    mHeadInfo.Textures[eBaseHeadTexture_ControlMap_Displacement] = LoadTexture(headDir, "DisplacementControlMap.png");
    mHeadInfo.Textures[eBaseHeadTexture_ControlMap_Color] = LoadTexture(headDir, "ColorControlMap.png");
    mHeadInfo.Textures[eBaseHeadTexture_FeatureMap] = LoadTexture(headDir, "FeatureMap.png");
    mHeadInfo.Textures[eBaseHeadTexture_ColorTransfer] = LoadTexture(headDir, "ColorTransferMap.png");
    mHeadInfo.Textures[eBaseHeadTexture_Skin] = LoadTexture(headDir, "SkinMask.png");
}

void FaceMappingEngine::addMorphParameters(CPUTAssetSet* headSet)
{
    addMorphParam("Front Profile", "Head Width", 0.5f, "shape_width", 0.0f, 1.0f, -2.0f, 2.0f);
    addMorphParam("Front Profile", "Eye Area Width", 0.5f, "shape_orbit_width", 0.0f, 1.0f, -2.0f, 2.0f);
    addMorphParam("Front Profile", "Cheekbone Width", 0.5f, "shape_cheekbone_size", 0.0f, 1.0f, -2.0f, 2.0f);
    addMorphParam("Front Profile", "OCC Width", 0.5f, "shape_OCC_width", 0.0f, 1.0f, -2.0f, 2.0f);
    addMorphParam("Front Profile", "Jaw Width", 0.5f, "shape_jaw_width", 0.0f, 1.0f, -2.0f, 2.0f);
    addMorphParam("Front Profile", "Jaw Level", 0.5f, "shape_jaw_level", 0.0f, 1.0f, -2.0f, 2.0f);
    addMorphParam("Front Profile", "Chin Width", 0.5f, "shape_chin_width", 0.0f, 1.0f, -2.0f, 2.0f);
    addMorphParam("Front Profile", "Neck Width", 0.5f, "shape_neck_girth", 0.0f, 1.0f, -2.0f, 2.0f);

    addMorphParam("Base Shape", "Shape 1", 0.0f, "shape_1", 0.0f, 1.0f, 0.0f, 1.0f);
    addMorphParam("Base Shape", "Shape 2", 0.0f, "shape_2", 0.0f, 1.0f, 0.0f, 1.0f);
    addMorphParam("Base Shape", "Shape 3", 0.0f, "shape_3", 0.0f, 1.0f, 0.0f, 1.0f);
    addMorphParam("Base Shape", "Width", 0.0f, "shape_width", 0.0f, 1.0f, 0.0f, 1.0f);

    SMorphTweakParamDef def;
    //TODO: use addMorphParam
    def.Reset("Base Shape", "Roundness", 0.5f);
    def.MorphParts.push_back(SMorphTweakParamPart("shape_round", 0.5f, 1.0f, 0.0f, 1.0f));
    def.MorphParts.push_back(SMorphTweakParamPart("shape_square", 0.0f, 0.5f, 1.0f, 0.0f));
    mMorphParamDefs.push_back(def);

    def.Reset("Base Shape", "BMI", 0.5f);
    def.MorphParts.push_back(SMorphTweakParamPart("shape_BMI_Heavy", 0.5f, 1.0f, 0.0f, 1.0f));
    def.MorphParts.push_back(SMorphTweakParamPart("shape_BMI_Lean", 0.0f, 0.5f, 1.0f, 0.0f));
    mMorphParamDefs.push_back(def);

    def.Reset("Jaw", "Cheekbone", 0.0f);
    def.MorphParts.push_back(SMorphTweakParamPart("shape_Cheekbone_Size", 0.0f, 1.0f, 0.0f, 1.0f));
    mMorphParamDefs.push_back(def);

    def.Reset("Jaw", "Chin Protrude", 0.5f);
    def.MorphParts.push_back(SMorphTweakParamPart("shape_chin_back", 0.0f, 0.5f, 1.0f, 0.0f));
    def.MorphParts.push_back(SMorphTweakParamPart("shape_chin_front", 0.5f, 1.0f, 0.0f, 1.0f));
    mMorphParamDefs.push_back(def);

    def.Reset("Jaw", "Chin Level", 0.0f);
    def.MorphParts.push_back(SMorphTweakParamPart("shape_chin_level", 0.0f, 1.0f, 0.0f, 1.0f));
    mMorphParamDefs.push_back(def);

    def.Reset("Jaw", "Chin Width", 0.5f);
    def.MorphParts.push_back(SMorphTweakParamPart("shape_chin_narrow", 0.0f, 0.5f, 1.0f, 0.0f));
    def.MorphParts.push_back(SMorphTweakParamPart("shape_chin_width", 0.5f, 1.0f, 0.0f, 1.0f));
    mMorphParamDefs.push_back(def);

    def.Reset("Other", "Neck Slope", 0.0f);
    def.MorphParts.push_back(SMorphTweakParamPart("shape_chin_neck_slope", 0.0f, 1.0f, 0.0f, 1.0f));
    mMorphParamDefs.push_back(def);

    // Add all the morph targets available.

    for (int i = 0; i < (int)headSet->GetAssetCount(); i++)
    {
        CPUTRenderNode *node = NULL;
        headSet->GetAssetByIndex(i, &node);
        if (strstr(node->GetName().c_str(), ".mdl") && !strstr(node->GetName().c_str(), "Cshape"))
        {
            CPUTModel *model = (CPUTModel*)node;
            CPUTMeshDX11 *mesh = (CPUTMeshDX11*)model->GetMesh(0);
            if (mesh->GetVertexCount() == mBaseMesh.GetVertCount())
            {
                std::string modelName = CPUTFileSystem::basename(model->GetName(), true);
                std::string name = modelName;
                if (strncmp(name.c_str(), "shape_", 6) == 0)
                {
                    name = name.substr(6);
                }

                addMorphParam("All Shapes", name.c_str(), 0.5f, modelName.c_str(), 0.0f, 1.0f, -5.0f, 5.0f);
                addMorphParam("All Shapes Post", name.c_str(), 0.5f, modelName.c_str(), 0.0f, 1.0f, -5.0f, 5.0f);
            }
        }
        SAFE_RELEASE(node);
    }

    def.Reset("Shape", "BMI", 0.5f);
    def.MorphParts.push_back(SMorphTweakParamPart("shape_BMI_Heavy", 0.5f, 1.0f, 0.0f, 1.0f));
    def.MorphParts.push_back(SMorphTweakParamPart("shape_BMI_Lean", 0.0f, 0.5f, 1.0f, 0.0f));
    mPostMorphParamDefs.push_back(def);

    addMorphParam("Shape", "Ogre", 0.0f, "shape_Ogre", 0.0f, 1.0f, 0.0f, 1.0f);

}

void FaceMappingEngine::loadMorphTargets(CPUTAssetSet* headSet)
{
    CPUTSoftwareMesh tempMesh;
    for (int i = 0; i < 2; i++)
    {
        std::vector<SMorphTweakParamDef> &list = (i == 0) ? mMorphParamDefs : mPostMorphParamDefs;
        std::vector<float> &weights = (i == 0) ? mActiveMorphParamWeights : mActivePostMorphParamWeights;
        for (auto it = list.begin(); it != list.end(); it++)
        {
            for (auto itPart = it->MorphParts.begin(); itPart != it->MorphParts.end(); itPart++)
            {
                if (mMorphTargetMap.find(itPart->MorphTargetName) == mMorphTargetMap.end())
                {
                    std::string morphModelName = itPart->MorphTargetName + ".mdl";
                    LoadCPUTModelToSWMesh(headSet, morphModelName.c_str(), &tempMesh);

                    CMorphTarget *morphTarget = new CMorphTarget();
                    BuildMorphTarget(&mBaseMesh, &tempMesh, morphTarget);
                    mMorphTargetMap[itPart->MorphTargetName] = morphTarget;
                }
            }
            weights.push_back(it->Default);
        }
    }
}

void FaceMappingEngine::Create()
{
    CPUT_DX11::CreateResources();
    mpQDXWidget = static_cast<QDXWidget*>(mpWindow);

    mpShadowRenderTarget = CPUTRenderTargetDepth::Create();
    mpShadowRenderTarget->CreateRenderTarget(std::string("$shadow_depth"), SHADOW_WIDTH_HEIGHT, SHADOW_WIDTH_HEIGHT, DXGI_FORMAT_D32_FLOAT);

    mpCameraController = CPUTCameraControllerFPS::Create();
    mpShadowCamera = CPUTCamera::Create(CPUT_ORTHOGRAPHIC);

    int width, height;
    mpWindow->GetClientDimensions(&width, &height);

    FaceMappingUtil_Init();


    mCPUTLandmarkModel = NULL;
    mForceRebuildAll = true;
    mOtherHeadTexture = NULL;
    mLastTweaks.Scale = 0.0f;

    SetDefaultDebug();
    SetDefaultTweaks();

    CPUTCameraModelViewer *cameraModelViewer = new CPUTCameraModelViewer();
    mCameraController = cameraModelViewer;

    CPUTCamera *pCamera = (CPUTCamera*)mCameraController->GetCamera();
    pCamera->SetFov(20.f * float(3.14159265359 / 180.0));
    pCamera->SetNearPlaneDistance(0.1f);
    pCamera->SetFarPlaneDistance(400.0f);

    cameraModelViewer->SetTarget(float3(0, 0, 0));
    cameraModelViewer->SetDistance(120.0f, 0.1f, 300.0f);
    cameraModelViewer->SetViewAngles(0, 0);
}

void FaceMappingEngine::LoadContent()
{
    std::string executableDirectory;
    CPUTFileSystem::GetExecutableDirectory(&executableDirectory);

    std::string mediaDir;
    CPUTFileSystem::GetMediaDirectory(&mediaDir);

    CPUTAssetLibrary *pAssetLibrary = CPUTAssetLibrary::GetAssetLibrary();
    pAssetLibrary->SetSystemDirectoryName(mediaDir + "/System/");
    pAssetLibrary->SetMediaDirectoryName(mediaDir);

    loadCubeMap(mediaDir);
    loadHeadModelAndAssets(mediaDir);

    std::string headDir;
    CPUTFileSystem::CombinePath(mediaDir, "/HeadTextures", &headDir);
    loadHeadTextures(headDir);

    CPUTAssetSet* headSet = mHeadAssetScene->GetAssetSet(2);
    addMorphParameters(headSet);
    loadMorphTargets(headSet);
}

void FaceMappingEngine::addMorphParam(const char *category, const char *name, float defaultValue, const char *modelName, float range1, float range2, float apply1, float apply2)
{
    SMorphTweakParamDef def;
    def.Reset(category, name, defaultValue);
    def.MorphParts.push_back(SMorphTweakParamPart(modelName, range1, range2, apply1, apply2));
    mMorphParamDefs.push_back(def);
}

void FaceMappingEngine::loadHair(CPUTAssetSet *set, const char *modelName, const char *displayName )
{
    if(set!=NULL){
        SHairDef hairDef(NULL, displayName, new CPUTSoftwareMesh());
        set->GetAssetByName(modelName, (CPUTRenderNode**)&hairDef.Model);
        CPUTMeshDX11 *dx11Mesh = (CPUTMeshDX11 *)hairDef.Model->GetMesh(0);
        hairDef.SWMesh->CopyFromDX11Mesh(dx11Mesh);
        hairDef.SWMesh->ApplyTransform(hairDef.Model->GetWorldMatrix());
        mHairDefs.push_back(hairDef);
    }
    else {
        mHairDefs.push_back(SHairDef(NULL, displayName, NULL));
    }
}

void FaceMappingEngine::loadBeardPart(CPUTAssetSet *beardSet, const char *modelName, const char *displayName){
    SHairDef beardDef(NULL, displayName, new CPUTSoftwareMesh());
    beardSet->GetAssetByName(modelName, (CPUTRenderNode**)&beardDef.Model);
    CPUTMeshDX11 *dx11Mesh = (CPUTMeshDX11 *)beardDef.Model->GetMesh(0);
    beardDef.SWMesh->CopyFromDX11Mesh(dx11Mesh);
    beardDef.SWMesh->ApplyTransform(beardDef.Model->GetWorldMatrix());
    mBeardDefs.push_back(beardDef);
    mBeardEnabled.push_back(false);
}

void FaceMappingEngine::SetHairIndex(int hairIndex)
{
    mCurrentHairIndex = hairIndex;
    assert(hairIndex >= 0 && hairIndex < (int)mHairDefs.size());
    SHairDef *def = &mHairDefs[hairIndex];
    if (def->Model != NULL)
    {
        mCurrentHair.CopyFrom(def->SWMesh);
    }
    mForceRebuildAll = true;
}



void FaceMappingEngine::Shutdown()
{
    CPUT_DX11::Shutdown();

    SAFE_DELETE(mCameraController);
    SAFE_DELETE(mHeadAssetScene);
    SAFE_RELEASE(mCPUTLandmarkModel);
    SAFE_RELEASE(mpCubemap);
    for (int i = 0; i < (int)mHairDefs.size(); i++)
    {
        SAFE_RELEASE(mHairDefs[i].Model);
        SAFE_DELETE(mHairDefs[i].SWMesh);
    }
    for (int i = 0; i < (int)mBeardDefs.size(); i++)
    {
        SAFE_RELEASE(mBeardDefs[i].Model);
        SAFE_DELETE(mBeardDefs[i].SWMesh);
    }
    for (auto it = mMorphTargetMap.begin(); it != mMorphTargetMap.end(); it++)
    {
        SAFE_DELETE(it->second);
    }

    for (int i = 0; i < eBaseHeadTexture_Count; i++)
    {
        SAFE_RELEASE(mHeadInfo.Textures[i]);
    }
    mMorphTargetMap.clear();

    FaceMappingUtil_Shutdown();

    // Note: these two are defined in the base.  We release them because we addref them.
    SAFE_RELEASE(mpCamera);
    SAFE_RELEASE(mpShadowCamera);
    SAFE_DELETE(mpCameraController);
    SAFE_DELETE(mpShadowRenderTarget);
    CPUTAssetLibrary::GetAssetLibrary()->ReleaseAllLibraryLists();
    CPUT_DX11::ReleaseResources();
}

QWidget& FaceMappingEngine::GetQWidget(){
    return *mpQDXWidget;
}

void FaceMappingEngine::LoadFace(const std::string &filename)
{
    mObjFilename = filename;
    mFaceModel.LoadObjFilename(filename);

    CPUTAssetLibrary *pAssetLibrary = CPUTAssetLibrary::GetAssetLibrary();
    pAssetLibrary->SetRootRelativeMediaDirectory("MyAssets");
    std::string matName = pAssetLibrary->GetMaterialDirectoryName();
    CPUTFileSystem::CombinePath(matName, "displace_map_render.mtl", &matName);

    mForceRebuildAll = true;
}

CPUTEventHandledCode FaceMappingEngine::HandleKeyboardEvent(CPUTKey key, CPUTKeyState state)
{
    if (mCameraController != NULL)
    {
        mCameraController->HandleKeyboardEvent(key, state);
    }
    return CPUT_EVENT_HANDLED;
}

CPUTEventHandledCode FaceMappingEngine::HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state, CPUTEventID message)
{
    if (mCameraController != NULL)
    {
        return mCameraController->HandleMouseEvent(x, y, wheel, state, message);
    }
    return CPUT_EVENT_UNHANDLED;
}

void FaceMappingEngine::Update(double dt)
{
    if (mCameraController != NULL)
    {
        mCameraController->Update(dt);
    }
}

void FaceMappingEngine::ResizeWindow(UINT width, UINT height)
{
    CPUT_DX11::ResizeWindow( width, height );

    // Resize any application-specific render targets here
    if( mpCamera )
        mpCamera->SetAspectRatio(((float)width)/((float)height));

}

void FaceMappingEngine::ResetActiveMorphTargets(bool post)
{
    std::vector<SMorphTweakParamDef> &list = post ? mPostMorphParamDefs : mMorphParamDefs;
    std::vector<float> &weights = post ? mActivePostMorphParamWeights : mActiveMorphParamWeights;
    for (int i = 0; i < (int)list.size(); i++)
    {
        weights[i] = list[i].Default;
    }
}


inline float RemapRange(float value, float r1Min, float r1Max, float r2Min, float r2Max)
{
    float ratio = (value - r1Min) / (r1Max - r1Min);
    ratio = floatClamp(ratio, 0.0f, 1.0f);
    return r2Min + ratio * (r2Max - r2Min);
}

void FaceMappingEngine::CreateMorphTargetEntries(std::vector<MorphTargetEntry> &list, std::vector<SMorphTweakParamDef> &defs, std::vector<float> &weights, bool post)
{
    for (int i = 0; i < (int)defs.size(); i++)
    {
        SMorphTweakParamDef *def = &defs[i];
        float ratio = weights[i];
        for (auto part = def->MorphParts.begin(); part != def->MorphParts.end(); part++)
        {
            float weight = RemapRange(ratio, part->ParamRange0, part->ParamRange1, part->Apply0, part->Apply1);
            if (abs(weight) > 0.00001f)
            {
                MorphTargetEntry entry;
                entry.Weight = weight;
                entry.Target = mMorphTargetMap[part->MorphTargetName];
                entry.Post = post;
                list.push_back(entry);
            }
        }
    }
}

bool FaceMappingEngine::IsFaceLoaded()
{
    return mFaceModel.GetMesh()->GetVertCount() > 0;
}

void FaceMappingEngine::Render(double deltaSeconds)
{

    CPUTRenderParameters renderParams;
    int windowWidth, windowHeight;
    mpWindow->GetClientDimensions( &windowWidth, &windowHeight);

    {
        renderParams.mpShadowCamera = NULL;
        renderParams.mpCamera = mpShadowCamera;
        renderParams.mpPerFrameConstants = (CPUTBuffer*)mpPerFrameConstantBuffer;
        renderParams.mpPerModelConstants = (CPUTBuffer*)mpPerModelConstantBuffer;
        //Animation
        renderParams.mpSkinningData = (CPUTBuffer*)mpSkinningDataConstantBuffer;

        renderParams.mWidth = windowWidth;
        renderParams.mHeight = windowHeight;
        renderParams.mRenderOnlyVisibleModels = false;

        //*******************************
        // Draw the shadow scene
        //*******************************
        UpdatePerFrameConstantBuffer(renderParams, deltaSeconds);

        renderParams.mWidth = windowWidth;
        renderParams.mHeight = windowHeight;
        renderParams.mpCamera = mpCamera;
        renderParams.mpShadowCamera = mpShadowCamera;
        UpdatePerFrameConstantBuffer(renderParams, deltaSeconds);

        // Clear back buffer
        const float clearColor[] = { 0.0993f, 0.0993f, 0.0993f, 1.0f };
        mpContext->ClearRenderTargetView( mpBackBufferRTV,  clearColor );
        mpContext->ClearDepthStencilView( mpDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0);
    }


    if(renderParams.mpShadowCamera != NULL)
    {
        float4 camPos = float4(0.0f, mDirectionalLightHeight, -10.0f, 1.0f);
        camPos = camPos * float4x4RotationY(mDirectionalLightAngle);
        renderParams.mpShadowCamera->SetPosition(camPos);
        renderParams.mpShadowCamera->LookAt(float3(0.0, 0.0f, 0.0f));
    }

    if (IsFaceLoaded())
    {
        SPipelineInput input;
        input.FaceModel = &mFaceModel;
        input.RenderParams = &renderParams;
        input.BaseHeadInfo = &mHeadInfo;
        input.Tweaks = &mTweaks;

        mTweaks.Flags = 0;
        mTweaks.Flags = mTweaks.Flags | (mSkipFaceFit ? PIPELINE_FLAG_SkipFitFace : 0);
        mTweaks.Flags = mTweaks.Flags | (mSkipFaceDisplace ? PIPELINE_FLAG_SkipDisplacementMap : 0);
        mTweaks.Flags = mTweaks.Flags | (mSkipFaceColorBlend ? PIPELINE_FLAG_SkipFaceColorBlend : 0);
        mTweaks.Flags = mTweaks.Flags | (mSkipSeamFill ? PIPELINE_FLAG_SkipColorSeamFill : 0);

        for (int i = 0; i < kMaxHeadBlends; i++)
        {
            mTweaks.OtherHeadMesh = mOtherHeadTexture != NULL ? &mOtherHead : NULL;
            mTweaks.OtherHeadTexture = mOtherHeadTexture;
        }

        mTweaks.MorphTargetEntries.clear();
        CreateMorphTargetEntries(mTweaks.MorphTargetEntries, mMorphParamDefs, mActiveMorphParamWeights, false);
        CreateMorphTargetEntries(mTweaks.MorphTargetEntries, mPostMorphParamDefs, mActivePostMorphParamWeights, true);

        input.HairInfo.clear();
        SHairDef *hairDef = (mCurrentHairIndex >= 0 && mCurrentHairIndex < (int)mHairDefs.size()) ? &mHairDefs[mCurrentHairIndex] : NULL;
        bool hasHair = hairDef != NULL && hairDef->Model != NULL;
        if (hasHair)
        {
            input.HairInfo.push_back(SHairPipelineInfo(&mCurrentHair));
        }
        bool hasBeard = false;
        for (int i = 0; i < (int)mBeardEnabled.size(); i++)
        {
            if (mBeardEnabled[i])
            {
                input.HairInfo.push_back(SHairPipelineInfo(mBeardDefs[i].SWMesh));
                hasBeard = true;
            }
        }

        if (mForceRebuildAll || mTweaks != mLastTweaks)
        {
            mPipeline.Execute(&input, &mPipelineOutput);
            mLastTweaks = mTweaks;
            mForceRebuildAll = false;

            // Copy the deformed mesh to our template model
            mPipelineOutput.DeformedMesh->CopyToDX11Mesh((CPUTMeshDX11*)mDisplayHead->GetMesh(0));

            // Copy the deformed hair
            if (hasHair)
            {
                hairDef->Model->SetParentMatrix(float4x4Identity());
                mPipelineOutput.DeformedHairMeshes[0]->CopyToDX11Mesh((CPUTMeshDX11*)hairDef->Model->GetMesh(0));
            }
            if (hasBeard)
            {
                CPUTSoftwareMesh beardMesh;
                int beardStartIndex = hasHair ? 1 : 0;
                beardMesh.CopyFromMultiple(&mPipelineOutput.DeformedHairMeshes[beardStartIndex], (int)mPipelineOutput.DeformedHairMeshes.size() - beardStartIndex);
                if (beardMesh.GetVertCount() != 0)
                {
                    mBeardDefs[0].Model->SetParentMatrix(float4x4Identity());
                    beardMesh.CopyToDX11Mesh((CPUTMeshDX11*)mBeardDefs[0].Model->GetMesh(0));
                }
            }
        }

        // Copy the blend stage diffuse texture into the material
        CPUTTexture *headTextureOverride = mPipelineOutput.DiffuseTexture;
        switch (mDebugHeadDisplayTextureView)
        {
        case DebugHeadDisplayTexture_DisplacmentControlMap: { headTextureOverride = mHeadInfo.Textures[eBaseHeadTexture_ControlMap_Displacement]; } break;
        case DebugHeadDisplayTexture_ColorControlMap: { headTextureOverride = mHeadInfo.Textures[eBaseHeadTexture_ControlMap_Color]; } break;
        case DebugHeadDisplayTexture_FeatureMap: { headTextureOverride = mHeadInfo.Textures[eBaseHeadTexture_FeatureMap]; } break;
        case DebugHeadDisplayTexture_ColorTransferMap: { headTextureOverride = mHeadInfo.Textures[eBaseHeadTexture_ColorTransfer]; } break;
        case DebugHeadDisplayTexture_SkinMap: { headTextureOverride = mHeadInfo.Textures[eBaseHeadTexture_Skin]; } break;
        default: break;
        }
        CPUTMaterial *mat = mDisplayHead->GetMaterial(0, 0);
        mat->OverridePSTexture(0, headTextureOverride);
        SAFE_RELEASE(mat);

        renderParams.mpCamera = (CPUTCamera*)mCameraController->GetCamera();
        renderParams.mpCamera->SetAspectRatio((float) windowWidth / (float)windowHeight);

        // Update per frame constants
        {
            CPUTFrameConstantBuffer frameConstants;
            CPUTCamera* pCamera = (CPUTCamera*)mCameraController->GetCamera();
            frameConstants.AmbientColor = float4(1.0) * mAmbientLightIntensity;
            frameConstants.LightColor = float4(1.0)* 0.7f;
            frameConstants.LightDirection = float4(renderParams.mpShadowCamera->GetLookWS(), 1.0);
            frameConstants.InverseView = inverse(*pCamera->GetViewMatrix());
            frameConstants.Projection = *pCamera->GetProjectionMatrix();
            renderParams.mpPerFrameConstants->SetData(0, sizeof(CPUTFrameConstantBuffer), (UINT*)&frameConstants);
        }

        if (!mHideCubeMap)
        {
            renderParams.mRenderOnlyVisibleModels = false;
            mpCubemap->RenderRecursive(renderParams, 0);
            renderParams.mRenderOnlyVisibleModels = true;
        }

        // render head
        mDisplayHead->Render(renderParams, 0);
        if (mShowWireframe)
            mDisplayHead->Render(renderParams, 2);

        if (hasHair)
        {
            hairDef->Model->Render(renderParams, 0);
            if (hairDef->Model->GetMaterialCount(0) > 2)
                hairDef->Model->Render(renderParams, 2);
        }
        if (hasBeard)
        {
            mBeardDefs[0].Model->Render(renderParams, 0);
        }

        if (mRenderLandmarkMesh)
        {
            mCPUTLandmarkModel->SetParentMatrix(float4x4Identity());
            mHeadInfo.LandmarkMesh.CopyToDX11Mesh((CPUTMeshDX11*)mCPUTLandmarkModel->GetMesh(0));
            mCPUTLandmarkModel->Render(renderParams, 0);
        }
        if (mRenderMorphedLandmarkMesh)
        {
            mCPUTLandmarkModel->SetParentMatrix(float4x4Identity());
            mPipeline.HeadGeometryStage->MorphedLandmarkMesh.CopyToDX11Mesh((CPUTMeshDX11*)mCPUTLandmarkModel->GetMesh(0));
            mCPUTLandmarkModel->Render(renderParams, 0);
        }
    }
}
