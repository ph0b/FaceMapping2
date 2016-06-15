//--------------------------------------------------------------------------------------
// Copyright 2011 Intel Corporation
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

#include <QApplication>


#include "SampleStart.h"

const std::string WINDOW_TITLE = "FaceScan 2.0";
const std::string GUI_DIR = "Media/gui_assets/";
const std::string SYSTEM_DIR = "Media/System/";

#ifdef _DEBUG

#pragma warning(disable:4074)//initializers put in compiler reserved initialization area
#pragma init_seg(compiler)//global objects in this file get constructed very early on

struct CrtBreakAllocSetter {
    CrtBreakAllocSetter() {
        //_crtBreakAlloc = 67844;
    }
};

CrtBreakAllocSetter g_crtBreakAllocSetter;

#endif//_DEBUG

// Application entry point.  Execution begins here.
//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{

#ifdef DEBUG
    // tell VS to report leaks at any exit of the program
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //http://msdn.microsoft.com/en-us/library/x98tx3cf%28v=vs.100%29.aspx
    //Add a watch for �{,,msvcr110d.dll}_crtBreakAlloc� to the watch window
    //Set the value of the watch to the memory allocation number reported by your sample at exit.
    //Note that the �msvcr110d.dll� is for MSVC2012.  Other versions of MSVC use different versions of this dll; you�ll need to specify the appropriate version.
#endif
    CPUTResult result = CPUT_SUCCESS;
    int returnCode = 0;

    QApplication a(argc,argv);

    // create an instance of my sample
    MySample *pSample = new MySample();

    // window and device parameters
    CPUTWindowCreationParams params;
    result = pSample->CPUTCreateWindowAndContext(WINDOW_TITLE, params);
    ASSERT(CPUTSUCCESS(result), "CPUT Error creating window and context.");
    pSample->Create();

    QWidget& widget = pSample->GetQWidget();
    widget.setParent(NULL);
    widget.show();

    returnCode = a.exec();

    pSample->ReleaseResources();
    pSample->DeviceShutdown();

    // cleanup resources
    SAFE_DELETE(pSample);

#if defined CPUT_FOR_DX11 && defined SUPER_DEBUG_DX
    typedef HRESULT(__stdcall *fPtrDXGIGetDebugInterface)(const IID&, void**);
    HMODULE hMod = GetModuleHandle(L"Dxgidebug.dll");
    fPtrDXGIGetDebugInterface DXGIGetDebugInterface = (fPtrDXGIGetDebugInterface)GetProcAddress(hMod, "DXGIGetDebugInterface");

    IDXGIDebug *pDebugInterface;
    DXGIGetDebugInterface(__uuidof(IDXGIDebug), (void**)&pDebugInterface);

    pDebugInterface->ReportLiveObjects(DXGI_DEBUG_D3D11, DXGI_DEBUG_RLO_ALL);
#endif
    return returnCode;
}
