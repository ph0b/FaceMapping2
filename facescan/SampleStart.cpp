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

CPUTCamera* GetCamera(CPUTScene* pScene);

MySample * MySample::Instance = NULL;

const UINT SHADOW_WIDTH_HEIGHT = 2048;
//-----------------------------------------------------------------------------
void MySample::Create()
{
    CreateResources();

    mpQDXWidget = static_cast<QDXWidget*>(mpWindow);

    int windowWidth, windowHeight;
    mpWindow->GetClientDimensions(&windowWidth, &windowHeight);

    SampleUtil_Init();

    mpFaceMapping = new FaceMapping();
    mpFaceMapping->Init();
    //gScreenWidth = windowWidth;
    //gScreenHeight = windowHeight;

    // go directly to the face mapping menu
    std::string userDir = GetUserDataDirectory();
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
    SampleUtil_Shutdown();
}

//-----------------------------------------------------------------------------
void MySample::Update(double deltaSeconds)
{
    if (mpWindow->DoesWindowHaveFocus())
    {
        mpCameraController->SetCamera(mpCamera);
        if (mpCameraController)
            mpCameraController->Update((float)deltaSeconds);
    }

    mpFaceMapping->Update((float)deltaSeconds);

    CPUTGetGuiController()->ControlModified();
    CPUTGetGuiController()->Update();
}

// Handle keyboard events
//-----------------------------------------------------------------------------
CPUTEventHandledCode MySample::HandleKeyboardEvent(CPUTKey key, CPUTKeyState state)
{
    CPUTEventHandledCode    handled = CPUT_EVENT_UNHANDLED;

    switch(key)
    {
    case KEY_ESCAPE:
        handled = CPUT_EVENT_HANDLED;
        PostQuitMessage(0);
        break;

    default:
        break;
    }

    if (handled == CPUT_EVENT_UNHANDLED)
    {
        handled = mpFaceMapping->HandleKeyboardEvent(key, state);
    }

    // pass it to the camera controller
    if(handled == CPUT_EVENT_UNHANDLED)
    {
        if (mpCameraController)
            handled = mpCameraController->HandleKeyboardEvent(key, state);
    }
    return handled;
}

// Handle mouse events
//-----------------------------------------------------------------------------
CPUTEventHandledCode MySample::HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state, CPUTEventID message)
{
    CPUTEventHandledCode code = mpFaceMapping->HandleMouseEvent( x, y, wheel, state, message );
    if (code != CPUT_EVENT_HANDLED)
    {
        if (mpCameraController)
        {
            return mpCameraController->HandleMouseEvent(x, y, wheel, state, message);
        }
    }
    return code;
}

// Handle any control callback events
//-----------------------------------------------------------------------------
void MySample::HandleCallbackEvent( CPUTEventID Event, CPUTControlID ControlID, CPUTControl *pControl )
{
    UNREFERENCED_PARAMETER(Event);
    UNREFERENCED_PARAMETER(pControl);
    std::string SelectedItem;
    static bool resize = false;

    switch(ControlID)
    {
    case ID_FULLSCREEN_BUTTON:
        CPUTToggleFullScreenMode();
        break;

    default:
        break;
    }

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

//    const int DEFAULT_MATERIAL = 0;
//    const int SHADOW_MATERIAL = 1;
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

void MySample::ReleaseResources()
{
    // Note: these two are defined in the base.  We release them because we addref them.
    SAFE_RELEASE(mpCamera);
    SAFE_RELEASE(mpShadowCamera);
    SAFE_DELETE(mpCameraController);
    SAFE_DELETE(mpDebugSprite);
    SAFE_DELETE(mpShadowRenderTarget);
    CPUTAssetLibrary::GetAssetLibrary()->ReleaseAllLibraryLists();
    CPUT_DX11::ReleaseResources();
}

CPUTCamera* GetCamera(CPUTScene* pScene)
{
    for (unsigned int i = 0; i < pScene->GetNumAssetSets(); i++)
    {
        CPUTCamera* pCamera = pScene->GetAssetSet(i)->GetFirstCamera();
        if (pCamera)
            return pCamera;
    }
    return NULL;
}

void MySample::CreateResources()
{
    CPUT_DX11::CreateResources();
    int width, height;
    mpWindow->GetClientDimensions(&width, &height);

    CPUTAssetLibrary *pAssetLibrary = CPUTAssetLibrary::GetAssetLibrary();

    std::string executableDirectory;
    std::string mediaDirectory;

    CPUTFileSystem::GetExecutableDirectory(&executableDirectory);
    CPUTFileSystem::ResolveAbsolutePathAndFilename(executableDirectory + "Media/", &mediaDirectory);
    
    pAssetLibrary->SetMediaDirectoryName(mediaDirectory + "gui_assets/");
    pAssetLibrary->SetSystemDirectoryName(mediaDirectory + "System/");
    pAssetLibrary->SetFontDirectoryName(mediaDirectory + "gui_assets/Font/");

    CPUTGuiController *pGUI = CPUTGetGuiController();
    pGUI->Initialize("guimaterial_dds_16", "arial_16.fnt");

    pGUI->SetCallback(this);

    pAssetLibrary->SetMediaDirectoryName(mediaDirectory);

    mpShadowRenderTarget = CPUTRenderTargetDepth::Create();
    mpShadowRenderTarget->CreateRenderTarget(std::string("$shadow_depth"), SHADOW_WIDTH_HEIGHT, SHADOW_WIDTH_HEIGHT, DXGI_FORMAT_D32_FLOAT);

    CPUTMaterial* pDebugMaterial = pAssetLibrary->GetMaterial("%sprite");
    mpDebugSprite = CPUTSprite::Create(-1.0f, -1.0f, 0.5f, 0.5f, pDebugMaterial);
    SAFE_RELEASE(pDebugMaterial);

    mpCameraController = CPUTCameraControllerFPS::Create();
    mpShadowCamera = CPUTCamera::Create(CPUT_ORTHOGRAPHIC);

    pGUI->Resize(width, height);

}


