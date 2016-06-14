// --------------------------------------------------------------------------------------------------------------------
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
// --------------------------------------------------------------------------------------------------------------------
#ifndef __QTWINDOWWIN_H__
#define __QTWINDOWWIN_H__
// ====================================================================================================================
#include <QApplication>
#include <QWidget>
#include "CPUT.h"

#include "CPUTWindow.h"

class CPUTQtWindowWin;
class QDXWidget;

class QDXWidget : public QWidget, public CPUTWindow
{
public:
	explicit QDXWidget(QWidget *parent = 0);
    virtual ~QDXWidget();

    virtual bool init();
    virtual void update();


	std::vector<std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>> QDXWidget::mWndProc;

	std::vector<std::function<void(int width, int height)>> mResizeEventCallbacks;
    virtual void RegisterCallbackResizeEvent(std::function<void(int width, int height)> callback) { mResizeEventCallbacks.push_back(callback); }

    void RegisterCallbackKeyboardEvent(std::function<CPUTEventHandledCode(CPUTKey key, CPUTKeyState state)> callback) { mKeyboardEventCallbacks.push_back(callback); }
	std::vector<std::function<CPUTEventHandledCode(CPUTKey key, CPUTKeyState state)>> mKeyboardEventCallbacks;

    void RegisterCallbackMouseEvent(std::function<CPUTEventHandledCode(int x, int y, int wheel, CPUTMouseState state, CPUTEventID message)> callback) { mMouseEventCallbacks.push_back(callback); }
	std::vector<std::function<CPUTEventHandledCode(int x, int y, int wheel, CPUTMouseState state, CPUTEventID message)>> mMouseEventCallbacks;

	virtual bool nativeEvent( QByteArray const& eventType, void* message, long* result ) override;

	void resizeEvent( QResizeEvent* event );


    void RegisterWndProc(std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> callback) { mWndProc.push_back(callback); }


	CPUTMouseState ConvertMouseState( Qt::MouseButtons buttons, Qt::KeyboardModifiers wParam );

	CPUTKey ConvertVirtualKeyToCPUTKey( int wParam );

	void keyPressEvent(QKeyEvent* event);
	void keyReleaseEvent(QKeyEvent* event);

	void mouseMoveEvent(QMouseEvent* event);

	void mousePressEvent(QMouseEvent* event);

	void mouseReleaseEvent(QMouseEvent* event);

	void wheelEvent(QWheelEvent* event);

	void CaptureMouse() { grabMouse(); }
	void ReleaseMouse() { releaseMouse(); }

public slots:
    void updateNow();
    void updateLater();

protected:
    bool m_updatePending;

    bool event(QEvent *event);

    QPaintEngine *paintEngine() const { return 0; }
    virtual void paintEvent(QPaintEvent *e) { Q_UNUSED(e); }

public:
	CPUTResult Create(const std::string WindowTitle, CPUTWindowCreationParams windowParams) override;


	int StartMessageLoop();

	void* GetNativeWindowHandle() const override;
	void GetClientDimensions(int* pWidth, int* pHeight) override;


	void GetClientDimensions( int* pX, int* pY, int* pWidth, int* pHeight ) override;
};
// ====================================================================================================================
#endif
