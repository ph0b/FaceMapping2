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
#ifndef __QTWINDOWWIN_H__
#define __QTWINDOWWIN_H__

#include <QApplication>
#include <QMainWindow>
#include "CPUT.h"

#include "CPUTOSServices.h"
#include "CPUTResource.h" // win resource.h customized for CPUT
#include "CPUTWindow.h"
#include <winuser.h> // for character codes
#include <string>

// typedef LRESULT (*WinProcHookFunc)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class CPUTQtWindowWin;

class MainWindow : public QMainWindow
{
public:
	explicit MainWindow( CPUTQtWindowWin* parent = 0 );

	~MainWindow() {}

	QWidget* c;
private:
	CPUTQtWindowWin* mCPUTWindow;

};

// // OS-specific window class
// //-----------------------------------------------------------------------------
class CPUTQtWindowWin : public CPUTWindow {
public:
    // construction
    CPUTQtWindowWin();
    virtual ~CPUTQtWindowWin();

    // Creates a graphics-context friendly window
    virtual CPUTResult Create(const std::string WindowTitle, CPUTWindowCreationParams windowParams);

    int StartMessageLoop();

    // return the HWND/Window handle for the created window
    HWND GetHWnd() {
		auto widget = mWindow->c;
		auto id = widget->winId();
	    return reinterpret_cast<HWND>(id);
    };

    // screen/window dimensions
    void GetClientDimensions( int *pWidth, int *pHeight);
    void GetClientDimensions( int *pX, int *pY, int *pWidth, int *pHeight);
    void GetDesktopDimensions(int *pX, int *pY, int *pWidth, int *pHeight);
    bool IsWindowMaximized();
    bool IsWindowMinimized();
    bool DoesWindowHaveFocus();
    void SetFullscreenState(bool fullscreen);
    bool GetFullscreenState();
    void GetWindowDimensions(int *pX, int *pY, int *pWidth, int *pHeight);

	// Mouse capture - 'binds'/releases all mouse input to this window
    virtual void CaptureMouse();
    virtual void ReleaseMouse();

    static void RegisterWndProc(std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> callback) { mWndProc.push_back(callback); }

	static std::vector<std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>> mWndProc;

    bool                mFullscreen;               // Is in fullscreen mode?
    RECT                mWindowedRect;
    int                 mAppClosedReturnCode;      // windows OS return code
    std::string         mAppTitle;                 // title put at top of window

    static bool         mbMaxMinFullScreen;
    DWORD               mWindowedStyle;
	bool WindowCreated = false;

    // Window creation helper functions
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    // CPUT conversion helper functions
    static CPUTMouseState ConvertMouseState(WPARAM wParam);
    static CPUTKey ConvertVirtualKeyToCPUTKey(WPARAM wParam);
    static CPUTKey ConvertCharacterToCPUTKey(WPARAM wParam);
private:
	QApplication* mApplication;
	MainWindow* mWindow;
};


#endif //#ifndef __WINDOWWIN_H__
