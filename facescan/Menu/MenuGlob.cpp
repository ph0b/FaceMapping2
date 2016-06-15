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
#include "MenuGlob.h"
#include "CPUTGuiControllerDX11.h"
#include "Menu_FaceScanPreview.h"

Menu_FaceMapping *gMenu_FaceMapping;
Menu_FaceScanPreview *gMenu_FaceScanPreview;

static int gScreenWidth = 0;
static int gScreenHeight = 0;

void MenuGlob_Init()
{
    gMenu_FaceMapping = new Menu_FaceMapping();
    gMenu_FaceScanPreview = new Menu_FaceScanPreview();

    gMenu_FaceMapping->Init();
    gMenu_FaceScanPreview->Init();
}

void MenuGlob_Shutdown()
{
    gMenu_FaceMapping->Shutdown();
    gMenu_FaceScanPreview->Shutdown();

    SAFE_DELETE(gMenu_FaceMapping);
    SAFE_DELETE(gMenu_FaceScanPreview);
}

CPUTGuiController *MenuGlob_GUI()
{
	return (CPUTGuiController*)CPUTGuiControllerDX11::GetController();
}

void MenuGlob_GetScreenDim(int *width, int *height)
{
	*width = gScreenWidth;
	*height = gScreenHeight;
}

void MenuGlob_SetScreenDim(int width, int height)
{
	gScreenWidth = width;
	gScreenHeight = height;
}
