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
#ifndef __CPUT_SAMPLESTARTDX11_H__
#define __CPUT_SAMPLESTARTDX11_H__

#include <stdio.h>
#include <time.h>
#include "CPUTSprite.h"

#include "CPUT_DX11.h"
#include <D3D11.h>
#include "CPUTBufferDX11.h"

#include "FaceMappingUtil.h"

#include "CPUTSprite.h"
#include "CPUTScene.h"
#include "CPUTParser.h"
#include "CPUTTextureDX11.h"

class FaceMapping;

// define some controls
#define FPS_VALUES 64
const CPUTControlID ID_MAIN_PANEL = 10;
const CPUTControlID ID_SECONDARY_PANEL = 20;
const CPUTControlID ID_FULLSCREEN_BUTTON = 100;
const CPUTControlID ID_NEXTMODEL_BUTTON = 101;
const CPUTControlID ID_TEST_CONTROL = 1000;
const CPUTControlID ID_IGNORE_CONTROL_ID = -1;

//-----------------------------------------------------------------------------

class MySample : public CPUT_DX11
{

private:
    QDXWidget             *mpQDXWidget;
    CPUTSprite			*mpFullscreenSprite;
    FaceMapping         *mpFaceMapping;

    float                  mfElapsedTime;
    CPUTCameraController  *mpCameraController;
    CPUTAssetSet          *mpShadowCameraSet;
    CPUTRenderTargetDepth *mpShadowRenderTarget;



public:
    MySample() : mpQDXWidget(NULL),
        mpFullscreenSprite(NULL),
        mpFaceMapping(NULL),
        mfElapsedTime(0.0),
        mpCameraController(NULL),
        mpShadowCameraSet(NULL),
        mpShadowRenderTarget(NULL)
    {
		
    }

    virtual ~MySample(){}

    virtual CPUTEventHandledCode HandleKeyboardEvent(CPUTKey key, CPUTKeyState state);
    virtual CPUTEventHandledCode HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state, CPUTEventID message);
    virtual void                 HandleCallbackEvent( CPUTEventID Event, CPUTControlID ControlID, CPUTControl *pControl );
    
    virtual void Create();
    virtual void Shutdown();
    virtual void Render(double deltaSeconds);
    virtual void Update(double deltaSeconds);
    virtual void ResizeWindow(UINT width, UINT height);

    void LoadAssets();
    QWidget& GetQWidget();
};
#endif // __CPUT_SAMPLESTARTDX11_H__
