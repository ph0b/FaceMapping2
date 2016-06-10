#include "CPUTQtWindowWin.h"

#include <QGridLayout>

#include <QAbstractEventDispatcher>
#include <QAbstractNativeEventFilter>

#include <QWidget>

#include <QResizeEvent>
#include <QDebug>
#include <QImageReader>

std::vector<std::function<LRESULT(HWND,UINT,WPARAM,LPARAM)>> CPUTQtWindowWin::mWndProc;

struct EventFilter : public QAbstractNativeEventFilter {
	EventFilter(HWND w) : mWin(w) {}
	virtual bool nativeEventFilter(QByteArray const& eventType, void* message, long* result ) override
	{
		// Keyboard events do not seem to be handled by the Widget rather they are sent to the parent window?

		MSG* msg = (MSG*)message;
		if(mWin == msg->hwnd) {

			if(msg->message == WM_KEYDOWN || msg->message == WM_KEYUP ||msg->message == WM_CHAR   ) {
				auto res = CPUTQtWindowWin::WndProc(mWin, msg->message, msg->wParam, msg->lParam);
			}
		}
		return false;
	}
private:
	HWND mWin;
};



class QDXWidget : public QWidget
{
public:
    explicit QDXWidget(CPUTQtWindowWin* cput, QWidget *parent = 0);
    virtual ~QDXWidget();

    virtual bool init();
    virtual void update();

	virtual bool nativeEvent(QByteArray const& eventType, void* message, long* result ) override {

		MSG* msg = (MSG*)message;
			if (msg->message == WM_KEYDOWN) {
				auto a = 123;
			}
			auto res = CPUTQtWindowWin::WndProc(msg->hwnd, msg->message, msg->wParam, msg->lParam);
			return false;
	}

public slots:
    void updateNow();
    void updateLater();

protected:
    bool m_updatePending;

    bool event(QEvent *event);
    void resizeEvent(QResizeEvent *event);

    QPaintEngine *paintEngine() const { return 0; }
    virtual void paintEvent(QPaintEvent *e) { Q_UNUSED(e); }
private:
	CPUTQtWindowWin* mCPUT;
};



QDXWidget::QDXWidget(CPUTQtWindowWin* cput, QWidget *parent) :
    QWidget(parent)
	, mCPUT(cput)
	//,  m_d3d(nullptr)
{
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);

    setAttribute( Qt::WA_NativeWindow, true );
}

QDXWidget::~QDXWidget()
{

}

bool QDXWidget::init()
{
	return true;
}

void QDXWidget::update()
{
	// trigger render and other calls
	for (const auto &callBack : mCPUT->mLoopEventCallbacks) {
		callBack();
	}
}

void QDXWidget::updateNow()
{
	update();
    updateLater();
}

void QDXWidget::updateLater()
{
    if (!m_updatePending)
    {
        m_updatePending = true;
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

bool QDXWidget::event(QEvent *event)
{
    if (event->type() == QEvent::UpdateRequest)
    {
        m_updatePending = false;
        updateNow();
        return true;
    }
    else
    {
        return QWidget::event(event);
    }
}

void QDXWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
}



MainWindow::MainWindow( CPUTQtWindowWin* parent ) {
	mCPUTWindow = parent;
	QHBoxLayout* layout = new QHBoxLayout;

	QWidget* central = new QDXWidget(mCPUTWindow, this);
	setCentralWidget( central );

	centralWidget()->setLayout( layout );

	c = central;
}

#ifdef _DEBUG
wchar_t lpQtMsgBuf[100]; // declare global in case error is about lack of resources
_inline void HandleWin32Error()
{
    DWORD err = GetLastError();
    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        lpQtMsgBuf,
        100,
        NULL );

    int numBytes = WideCharToMultiByte(CP_UTF8, 0, lpQtMsgBuf, -1, NULL, 0, NULL, NULL);
    char* wstr = new char[numBytes];
    WideCharToMultiByte(CP_UTF8, 0, lpQtMsgBuf, -1, wstr, numBytes, NULL, NULL);

    ASSERT(false, wstr);
    delete[] wstr;
}
#else
_inline void HandleWin32Error() {}
#endif

// static initializers
bool CPUTQtWindowWin::mbMaxMinFullScreen=false;

// Constructor
//-----------------------------------------------------------------------------
CPUTQtWindowWin::CPUTQtWindowWin()
: mFullscreen(false)
{
    mAppTitle.clear();
}

// Destructor
//-----------------------------------------------------------------------------
CPUTQtWindowWin::~CPUTQtWindowWin()
{
    mAppTitle.clear();
}

// Create window
//-----------------------------------------------------------------------------
CPUTResult CPUTQtWindowWin::Create(const std::string WindowTitle, CPUTWindowCreationParams windowParams)
{
	auto numArgs = 0;
	mApplication = new QApplication(numArgs, nullptr);


    //
    // Validate that the window starting position is within the virtual desktop
    //
    ASSERT((windowParams.windowPositionX == -1) || (windowParams.windowPositionX >= GetSystemMetrics(SM_XVIRTUALSCREEN)), "You are attempting to create a window outside the desktop coordinates.  Check your CPUTWindowCreationParams");
    ASSERT((windowParams.windowPositionX <= GetSystemMetrics(SM_XVIRTUALSCREEN) + GetSystemMetrics(SM_CXVIRTUALSCREEN)),  "You are attempting to create a window outside the desktop coordinates.  Check your CPUTWindowCreationParams");
    ASSERT((windowParams.windowPositionY == -1) || (windowParams.windowPositionY >= GetSystemMetrics(SM_YVIRTUALSCREEN)), "You are attempting to create a window outside the desktop coordinates.  Check your CPUTWindowCreationParams");
    ASSERT((windowParams.windowPositionY <= GetSystemMetrics(SM_YVIRTUALSCREEN) + GetSystemMetrics(SM_CYVIRTUALSCREEN)),  "You are attempting to create a window outside the desktop coordinates.  Check your CPUTWindowCreationParams");

    //
    // Width or height of zero means to use a default size
    //
    if( (0 == windowParams.windowWidth) || (0 == windowParams.windowHeight) )
    {
        windowParams.windowWidth = 1280;
        windowParams.windowHeight = 720;
    }

    mAppTitle = WindowTitle;
    if (0 == mAppTitle.compare(""))
    {
        mAppTitle = "CPUT Sample";
    }

	mWindow = new MainWindow(this);
	mWindow->setFixedSize(windowParams.windowWidth, windowParams.windowHeight);
	mWindow->show();

	// Force creation
	auto hwnd = mWindow->winId();

	QAbstractEventDispatcher::instance()->installNativeEventFilter(
		new EventFilter((HWND)mWindow->winId()));

	WindowCreated = true;

    return CPUT_SUCCESS;
}

// // Get the OS window dimensions - just working "client" area
// //-----------------------------------------------------------------------------
void CPUTQtWindowWin::GetClientDimensions(int *pX, int *pY, int *pWidth, int *pHeight)
{
	mWindow->width();
	*pX = mWindow->pos().x();
	*pY = mWindow->pos().y();
	*pWidth = mWindow->width();
	*pHeight = mWindow->height();
}


// // Get the OS window client area dimensions
// //-----------------------------------------------------------------------------
void CPUTQtWindowWin::GetClientDimensions(int *pWidth, int *pHeight)
{
	*pWidth = mWindow->width();
	*pHeight = mWindow->height();
}

// Get the desktop dimensions - for the monitor containing most of the active window
//-----------------------------------------------------------------------------
void CPUTQtWindowWin::GetDesktopDimensions(int *pX, int *pY, int *pWidth, int *pHeight)
{
    RECT windowRect;
    GetWindowRect(reinterpret_cast<HWND>(mWindow->winId()), &windowRect);
    HMONITOR hMonitor = MonitorFromRect(&windowRect, MONITOR_DEFAULTTONEAREST);
    MONITORINFO monitorInfo;
    monitorInfo.cbSize = sizeof(monitorInfo);
    GetMonitorInfo(hMonitor, &monitorInfo);

    *pX      = monitorInfo.rcMonitor.left;
    *pY      = monitorInfo.rcMonitor.top;
    *pWidth  = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
    *pHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
}

// // Returns true if the CPUT window is currently the 'focused' window on the 
// // desktop
// //-----------------------------------------------------------------------------
bool CPUTQtWindowWin::DoesWindowHaveFocus()
{
	return mWindow->hasFocus();
}

void CPUTQtWindowWin::SetFullscreenState(bool fullscreen)
{
    if (mFullscreen == fullscreen) {
        return;
    }

    if (fullscreen)
    {
        int x, y, windowWidth, windowHeight;
		windowWidth = mWindow->width();
		windowHeight = mWindow->height();
		x = mWindow->pos().x();
		y = mWindow->pos().y();
        RECT windowRect = { x, y, windowWidth + x, windowHeight + y };
        mWindowedRect = windowRect;

        int desktopX, desktopY;
        int desktopWidth, desktopHeight;
        GetDesktopDimensions(&desktopX, &desktopY, &desktopWidth, &desktopHeight);

        mWindowedStyle = GetWindowLong((HWND)mWindow->winId(), GWL_STYLE);

        SetWindowLong((HWND)mWindow->winId(), GWL_STYLE, (mWindowedStyle & ~WS_OVERLAPPEDWINDOW) | WS_POPUP);

        SetWindowPos((HWND)mWindow->winId(), HWND_TOP, desktopX, desktopY, desktopWidth, desktopHeight,
            SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING | SWP_NOZORDER);

        mFullscreen = true;
    }
    else
    {
        int width = mWindowedRect.right - mWindowedRect.left;
        int height = mWindowedRect.bottom - mWindowedRect.top;
        SetWindowLong((HWND)mWindow->winId(), GWL_STYLE, mWindowedStyle);
        SetWindowPos((HWND)mWindow->winId(), HWND_TOP, mWindowedRect.left, mWindowedRect.top, width, height,
            SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING | SWP_NOZORDER | SWP_FRAMECHANGED);
        mFullscreen = false;
    }
}

bool CPUTQtWindowWin::GetFullscreenState()
{
    return mFullscreen;
}

// mouse
// this function 'captures' the mouse and makes it ONLY available to this app
// User cannot click on any other app until you call ReleaseMouse, so use this
// carefully
//-----------------------------------------------------------------------------
void CPUTQtWindowWin::CaptureMouse()
{
	mWindow->grabMouse();
}

// Releases a captured mouse
//-----------------------------------------------------------------------------
void CPUTQtWindowWin::ReleaseMouse()
{
	mWindow->releaseMouse();
}

// //
// // WndProc
// // Handles the main message loop's events/messages
// //-----------------------------------------------------------------------------
LRESULT CALLBACK CPUTQtWindowWin::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CPUTEventHandledCode handledCode = CPUT_EVENT_UNHANDLED;
    LRESULT res;
    static bool sizing = false;

	if(!CPUTQtWindowWin::mWndProc.empty()) {
		for(auto c : CPUTQtWindowWin::mWndProc) {
			if (c(hWnd, message, wParam, lParam)) {
				return 1;
			}
		}
	}

    switch (message)
    {
    case WM_COMMAND:
    {
        int     wmId, wmEvent;
        wmId = LOWORD(wParam);
        wmEvent = HIWORD(wParam);

        // handle any menu item events here
        // see reference code in file history for examples
    }
        break;
    case WM_KEYDOWN:  // WM_KEYDOWN: gives you EVERY key - including shifts/etc
    {
        CPUTKeyState state = CPUT_KEY_DOWN;
        CPUTKey key = ConvertVirtualKeyToCPUTKey(wParam);
        if (KEY_NONE != key)
        {
            for (const auto &callBack : mKeyboardEventCallbacks) {
                handledCode = callBack(key, state);
            }
        }
    }
        break;
    case WM_KEYUP:
    {
        CPUTKeyState state = CPUT_KEY_UP;
        CPUTKey key = ConvertVirtualKeyToCPUTKey(wParam);
        if (KEY_NONE != key)
        {

            for (const auto &callBack : mKeyboardEventCallbacks) {
                handledCode = callBack(key, state);
            }
        }
    }
        break;
    case WM_CHAR:
    {
        CPUTKeyState state = CPUT_KEY_DOWN;
        CPUTKey key = ConvertCharacterToCPUTKey(wParam);
        if (KEY_NONE != key)
        {
            for (const auto &callBack : mKeyboardEventCallbacks) {
                handledCode = callBack(key, state);
            }
        }
    }
        break;
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
        // handle double-click events
        break;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
        CPUTMouseState state = ConvertMouseState(wParam);

        short xPos = LOWORD(lParam);
        short yPos = HIWORD(lParam);

        for (const auto &callBack : mMouseEventCallbacks) {
            handledCode = callBack(xPos, yPos, 0, state, CPUT_EVENT_DOWN);
        }
    }
        break;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    {
        CPUTMouseState state = ConvertMouseState(wParam);

        short xPos = LOWORD(lParam);
        short yPos = HIWORD(lParam);

        for (const auto &callBack : mMouseEventCallbacks) {
            handledCode = callBack(xPos, yPos, 0, state, CPUT_EVENT_UP);
        }
    }
        break;
    case WM_MOUSEMOVE:
    {
        CPUTMouseState state = ConvertMouseState(wParam);

        short xPos = LOWORD(lParam);
        short yPos = HIWORD(lParam);

        for (const auto &callBack : mMouseEventCallbacks) {
            handledCode = callBack(xPos, yPos, 0, state, CPUT_EVENT_MOVE);
        }
    }
        break;

    case WM_MOUSEWHEEL:
        {
            // get mouse position
            short xPos = LOWORD(lParam);
            short yPos = HIWORD(lParam);

            // get wheel delta
            int wheel = GET_WHEEL_DELTA_WPARAM(wParam);  // one 'click'

            for (const auto &callBack : mMouseEventCallbacks) {
                handledCode = callBack(xPos, yPos, wheel, CPUT_MOUSE_WHEEL, CPUT_EVENT_WHEEL);
            }
        }
        return 0;
        break;

    case WM_PAINT:
        PAINTSTRUCT ps;
        HDC hdc;
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;
    case WM_SIZING:
        sizing = true;
        break;
    case WM_MOVING:
    case WM_ERASEBKGND:
        // overriding this to do nothing avoids flicker and
        // the expense of re-creating tons of gfx contexts as it resizes
        break;

    //case WM_ACTIVATE:
        // check for maximize/minimize
      //  break;

    case WM_SIZE:
        int width, height;
        height = HIWORD(lParam);
        width  = LOWORD(lParam);
        RECT windowRect;
        if(0==GetClientRect(hWnd, &windowRect)) // this gets the client area inside the window frame *excluding* frames/menu bar/etc
            break;
        width = windowRect.right - windowRect.left;
        height = windowRect.bottom - windowRect.top;

        // if we have shrunk to 0 width/height - do not pass on this kind of resize - leads to 
        // various render target resizing warnings
        if(0==width || 0==height)
        {
            break;
        }

        {
            if (!sizing)
            {
                // maximize/minimize effect
                if ((SIZE_MAXIMIZED == wParam))
                {
                    // resize for new max/min size
                    for (const auto &callBack : mResizeEventCallbacks) {
                        callBack(width, height);
                    }
                    mbMaxMinFullScreen = true;
                }
                else if (SIZE_RESTORED == wParam)
                {
                    if (true == mbMaxMinFullScreen)
                    {
                        for (const auto &callBack : mResizeEventCallbacks) {
                            callBack(width, height);
                        }
                        mbMaxMinFullScreen = false;
                    }
                    else
                    {
                        for (const auto &callBack : mResizeEventCallbacks) {
                            callBack(width, height);
                        }
                    }
                }
            }
            else
            {
                for (const auto &callBack : mResizeEventCallbacks) {
                    callBack(width, height);
                }
            }
        }
        break;
    case WM_EXITSIZEMOVE:
        sizing = false;
        // update the system's size and make callback
        {
            RECT windowRect;
            if(0==GetClientRect(hWnd, &windowRect)) // this gets the client area inside the window frame *excluding* frames/menu bar/etc
                break;

            width = windowRect.right - windowRect.left;
            height = windowRect.bottom - windowRect.top;

            // if we have shrunk to 0 width/height - do not pass on this kind of resize - leads to 
            // various render target resizing warnings
            if(0==width || 0==height)
            {
                break;
            }
            for (const auto &callBack : mResizeEventCallbacks) {
                callBack(width, height);
            }
        }
        break;


    case WM_DESTROY:
        // time to shut down the system
        PostQuitMessage(0);
        break;

    default:
        // we don't handle it - pass it on thru to parent
        res = DefWindowProc(hWnd, message, wParam, lParam);
        return res;
    }

    // translate handled code
    if(CPUT_EVENT_HANDLED == handledCode)
    {
        return 1;
    }

    return 0;
}

//
// Translates a Windows specific virtual key code to a CPUT key code
//
CPUTKey CPUTQtWindowWin::ConvertVirtualKeyToCPUTKey(WPARAM wParam)
{
	switch(wParam) {
	// numeric keys
	case 0x30:
		return KEY_0;
	case 0x31:
		return KEY_1;
	case 0x32:
		return KEY_2;
	case 0x33:
		return KEY_3;
	case 0x34:
		return KEY_4;
	case 0x35:
		return KEY_5;
	case 0x36:
		return KEY_6;
	case 0x37:
		return KEY_7;
	case 0x38:
		return KEY_8;
	case 0x39:
		return KEY_9;

	// letter keys
	case 0x41:
		return KEY_A;
	case 0x42:
		return KEY_B;
	case 0x43:
		return KEY_C;
	case 0x44:
		return KEY_D;
	case 0x45:
		return KEY_E;
	case 0x46:
		return KEY_F;
	case 0x47:
		return KEY_G;
	case 0x48:
		return KEY_H;
	case 0x49:
		return KEY_I;
	case 0x4A:
		return KEY_J;
	case 0x4B:
		return KEY_K;
	case 0x4C:
		return KEY_L;
	case 0x4D:
		return KEY_M;
	case 0x4E:
		return KEY_N;
	case 0x4F:
		return KEY_O;
	case 0x50:
		return KEY_P;
	case 0x51:
		return KEY_Q;
	case 0x52:
		return KEY_R;
	case 0x53:
		return KEY_S;
	case 0x54:
		return KEY_T;
	case 0x55:
		return KEY_U;
	case 0x56:
		return KEY_V;
	case 0x57:
		return KEY_W;
	case 0x58:
		return KEY_X;
	case 0x59:
		return KEY_Y;
	case 0x5A:
		return KEY_Z;

	// function keys
    case VK_F1:
        return KEY_F1;
    case VK_F2:
        return KEY_F2;
    case VK_F3:
        return KEY_F3;
    case VK_F4:
        return KEY_F4;
    case VK_F5:
        return KEY_F5;
    case VK_F6:
        return KEY_F6;
    case VK_F7:
        return KEY_F7;
    case VK_F8:
        return KEY_F8;
    case VK_F9:
        return KEY_F9;
    case VK_F10:
        return KEY_F10;
    case VK_F11:
        return KEY_F11;
    case VK_F12:
        return KEY_F12;

    // special keys
    case VK_HOME:
        return KEY_HOME;
    case VK_END:
        return KEY_END;
    case VK_PRIOR:
        return KEY_PAGEUP;
    case VK_NEXT:
        return KEY_PAGEDOWN;
    case VK_INSERT:
        return KEY_INSERT;
    case VK_DELETE:
        return KEY_DELETE;

    case VK_BACK:
        return KEY_BACKSPACE;
    case VK_TAB:
        return KEY_TAB;
    case VK_RETURN:
        return KEY_ENTER;

    case VK_PAUSE:
        return KEY_PAUSE;
    case VK_CAPITAL:
        return KEY_CAPSLOCK;
    case VK_ESCAPE:
        return KEY_ESCAPE;
	case VK_SHIFT:
		return KEY_SHIFT;

    case VK_UP:
        return KEY_UP;
    case VK_DOWN:
        return KEY_DOWN;
    case VK_LEFT:
        return KEY_LEFT;
    case VK_RIGHT:
        return KEY_RIGHT;
	}

	return KEY_NONE;
}

//
// Translates a character to a CPUT key code.
//
CPUTKey CPUTQtWindowWin::ConvertCharacterToCPUTKey(WPARAM wParam)
{
    switch(wParam)
    {
    case 'a':
    case 'A':
        return KEY_A;
    case 'b':
    case 'B':
        return KEY_B;
    case 'c':
    case 'C':
        return KEY_C;
    case 'd':
    case 'D':
        return KEY_D;
    case 'e':
    case 'E':
        return KEY_E;
    case 'f':
    case 'F':
        return KEY_F;
    case 'g':
    case 'G':
        return KEY_G;
    case 'h':
    case 'H':
        return KEY_H;
    case 'i':
    case 'I':
        return KEY_I;
    case 'j':
    case 'J':
        return KEY_J;
    case 'k':
    case 'K':
        return KEY_K;
    case 'l':
    case 'L':
        return KEY_L;
    case 'm':
    case 'M':
        return KEY_M;
    case 'n':
    case 'N':
        return KEY_N;
    case 'o':
    case 'O':
        return KEY_O;
    case 'p':
    case 'P':
        return KEY_P;
    case 'Q':
    case 'q':
        return KEY_Q;
    case 'r':
    case 'R':
        return KEY_R;
    case 's':
    case 'S':
        return KEY_S;
    case 't':
    case 'T':
        return KEY_T;
    case 'u':
    case 'U':
        return KEY_U;
    case 'v':
    case 'V':
        return KEY_V;
    case 'w':
    case 'W':
        return KEY_W;
    case 'x':
    case 'X':
        return KEY_X;
    case 'y':
    case 'Y':
        return KEY_Y;
    case 'z':
    case 'Z':
        return KEY_Z;


        // number keys
    case '1':
        return KEY_1;
    case '2':
        return KEY_2;
    case '3':
        return KEY_3;
    case '4':
        return KEY_4;
    case '5':
        return KEY_5;
    case '6':
        return KEY_6;
    case '7':
        return KEY_7;
    case '8':
        return KEY_8;
    case '9':
        return KEY_9;
    case '0':
        return KEY_0;


    // symbols
    case ' ':
        return KEY_SPACE;
    case '`':
        return KEY_BACKQUOTE;
    case '~':
        return KEY_TILDE;
    case '!':
        return KEY_EXCLAMATION;
    case '@':
        return KEY_AT;
    case '#':
        return KEY_HASH;
    case '$':
        return KEY_$;
    case '%':
        return KEY_PERCENT;
    case '^':
        return KEY_CARROT;
    case '&':
        return KEY_ANDSIGN;
    case '*':
        return KEY_STAR;
    case '(':
        return KEY_OPENPAREN;
    case ')':
        return KEY_CLOSEPARN;
    case '_':
        return KEY__;
    case '-':
        return KEY_MINUS;
    case '+':
        return KEY_PLUS;
    case '[':
        return KEY_OPENBRACKET;
    case ']':
        return KEY_CLOSEBRACKET;
    case '{':
        return KEY_OPENBRACE;
    case '}':
        return KEY_CLOSEBRACE;
    case '\\':
        return KEY_BACKSLASH;
    case '|':
        return KEY_PIPE;
    case ';':
        return KEY_SEMICOLON;
    case ':':
        return KEY_COLON;
    case '\'':
        return KEY_SINGLEQUOTE;
    case '\"':
        return KEY_QUOTE;
    case ',':
        return KEY_COMMA;
    case '.':
        return KEY_PERIOD;
    case '/':
        return KEY_SLASH;
    case '<':
        return KEY_LESS;
    case '>':
        return KEY_GREATER;
    case '?':
        return KEY_QUESTION;
    }

    return KEY_NONE;
}

// Convert mouse state to CPUT state
//-----------------------------------------------------------------------------
CPUTMouseState CPUTQtWindowWin::ConvertMouseState(WPARAM wParam)
{
    CPUTMouseState eState=CPUT_MOUSE_NONE;

    if( wParam & MK_CONTROL)
        eState = (CPUTMouseState) (eState | static_cast<int>(CPUT_MOUSE_CTRL_DOWN));

    if( wParam & MK_SHIFT)
        eState = (CPUTMouseState) (eState | static_cast<int>(CPUT_MOUSE_SHIFT_DOWN));

    if( wParam & MK_LBUTTON)
        eState = (CPUTMouseState) (eState | static_cast<int>(CPUT_MOUSE_LEFT_DOWN));

    if( wParam & MK_MBUTTON)
        eState = (CPUTMouseState) (eState | static_cast<int>(CPUT_MOUSE_MIDDLE_DOWN));

    if( wParam & MK_RBUTTON)
        eState = (CPUTMouseState) (eState | static_cast<int>(CPUT_MOUSE_RIGHT_DOWN));


    return eState;
}

// Main message pump
//-----------------------------------------------------------------------------
int CPUTQtWindowWin::StartMessageLoop() {
	return mApplication->exec();
}
