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

#include <QDebug>
#include "SampleStart.h"
#include "CPUTMaterial.h"
#include "CPUTLight.h"
#ifdef CPUT_FOR_DX11
#include "CPUTBufferDX11.h"
#include "CPUTTextureDX11.h"
#include "CPUTGuiControllerDX11.h"
#include <DXGIDebug.h>
#endif

#include "CPUTRenderTarget.h"
#include "CPUTFont.h"

#include "FaceMapping.h"
#include "FaceMappingUtil.h"

CPUTCamera* GetCamera(CPUTScene* pScene);

const UINT SHADOW_WIDTH_HEIGHT = 2048;
//-----------------------------------------------------------------------------
void MySample::Create()
{
    CPUT_DX11::CreateResources();
    int width, height;
    mpWindow->GetClientDimensions(&width, &height);

    std::string executableDirectory;
    CPUTFileSystem::GetExecutableDirectory(&executableDirectory);

    std::string mediaDir;
    CPUTFileSystem::ResolveAbsolutePathAndFilename(executableDirectory + "Media/", &mediaDir);

    CPUTAssetLibrary *pAssetLibrary = CPUTAssetLibrary::GetAssetLibrary();
    pAssetLibrary->SetSystemDirectoryName(mediaDir + "System/");
    pAssetLibrary->SetMediaDirectoryName(mediaDir);

    mpShadowRenderTarget = CPUTRenderTargetDepth::Create();
    mpShadowRenderTarget->CreateRenderTarget(std::string("$shadow_depth"), SHADOW_WIDTH_HEIGHT, SHADOW_WIDTH_HEIGHT, DXGI_FORMAT_D32_FLOAT);

    mpCameraController = CPUTCameraControllerFPS::Create();
    mpShadowCamera = CPUTCamera::Create(CPUT_ORTHOGRAPHIC);

    mpQDXWidget = static_cast<QDXWidget*>(mpWindow);

    int windowWidth, windowHeight;
    mpWindow->GetClientDimensions(&windowWidth, &windowHeight);

    FaceMappingUtil_Init();

    mpFaceMapping = new FaceMapping();
    mpFaceMapping->Init();
    //gScreenWidth = windowWidth;
    //gScreenHeight = windowHeight;

    // go directly to the face mapping menu
    std::string userDir = GetUserDataDirectory();


//    std::string dir;
//    CPUTFileSystem::GetExecutableDirectory(&dir);
//    std::string userDir;
//    CPUTFileSystem::CombinePath(dir, "userdata", &userDir);

    std::string debugFace;
    CPUTFileSystem::CombinePath(userDir, "joe_sr300_1.obj", &debugFace);
    mpFaceMapping->LoadFace(debugFace);
}


QWidget& MySample::GetQWidget(){
    return *mpQDXWidget;
}

void MySample::Shutdown()
{
    CPUT_DX11::Shutdown();
    SAFE_DELETE(mpFaceMapping);
    FaceMappingUtil_Shutdown();

    // Note: these two are defined in the base.  We release them because we addref them.
    SAFE_RELEASE(mpCamera);
    SAFE_RELEASE(mpShadowCamera);
    SAFE_DELETE(mpCameraController);
    SAFE_DELETE(mpShadowRenderTarget);
    CPUTAssetLibrary::GetAssetLibrary()->ReleaseAllLibraryLists();
    CPUT_DX11::ReleaseResources();
}

void MySample::Update(double deltaSeconds)
{
    mpFaceMapping->Update((float)deltaSeconds);

}

CPUTEventHandledCode MySample::HandleKeyboardEvent(CPUTKey key, CPUTKeyState state)
{
    return mpFaceMapping->HandleKeyboardEvent(key, state);
}

CPUTEventHandledCode MySample::HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state, CPUTEventID message)
{
    return mpFaceMapping->HandleMouseEvent( x, y, wheel, state, message );
}

// Handle any control callback events
//-----------------------------------------------------------------------------
void MySample::HandleCallbackEvent( CPUTEventID Event, CPUTControlID ControlID, CPUTControl *pControl )
{

}

//-----------------------------------------------------------------------------
void MySample::ResizeWindow(UINT width, UINT height)
{
    CPUT_DX11::ResizeWindow( width, height );

    // Resize any application-specific render targets here
    if( mpCamera )
        mpCamera->SetAspectRatio(((float)width)/((float)height));

}

//-----------------------------------------------------------------------------
void MySample::Render(double deltaSeconds)
{
    CPUTRenderParameters renderParams;

    renderParams.mpShadowCamera = NULL;
    renderParams.mpCamera = mpShadowCamera;
    renderParams.mpPerFrameConstants = (CPUTBuffer*)mpPerFrameConstantBuffer;
    renderParams.mpPerModelConstants = (CPUTBuffer*)mpPerModelConstantBuffer;
    //Animation
    renderParams.mpSkinningData = (CPUTBuffer*)mpSkinningDataConstantBuffer;

    int windowWidth, windowHeight;
    mpWindow->GetClientDimensions( &windowWidth, &windowHeight);

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

    mpFaceMapping->Render(renderParams);
}
