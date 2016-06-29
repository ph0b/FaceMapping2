#include "CPUTQtWindowWin.h"

#include <QGridLayout>

#include <QAbstractEventDispatcher>
#include <QAbstractNativeEventFilter>

#include <QWidget>

#include <QResizeEvent>
#include <QDebug>
#include <QImageReader>

QDXWidget::QDXWidget(QWidget *parent) :
    QWidget(parent)
	//,  m_d3d(nullptr)
{
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);

    setAttribute( Qt::WA_NativeWindow, true );


this->setAttribute(Qt::WA_OpaquePaintEvent, true);
this->setAttribute(Qt::WA_PaintOnScreen, true);
this->setAttribute(Qt::WA_DontCreateNativeAncestors, true);
this->setAttribute(Qt::WA_NativeWindow, true);
this->setAttribute(Qt::WA_NoSystemBackground, true);
this->setAttribute(Qt::WA_MSWindowsUseDirect3D, true);

	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);
	grabKeyboard();
	grabMouse();
}

QDXWidget::~QDXWidget()
{
	releaseKeyboard();
	releaseMouse();

}

bool QDXWidget::init()
{
	return true;
}

void QDXWidget::update()
{
	// trigger render and other calls
	for (const auto &callBack : mLoopEventCallbacks) {
		callBack();
	}
}

	void QDXWidget::keyPressEvent(QKeyEvent* event) {

		CPUTEventHandledCode handledCode = CPUT_EVENT_UNHANDLED;
        CPUTKeyState state = CPUT_KEY_DOWN;
        CPUTKey key = ConvertVirtualKeyToCPUTKey(event->nativeVirtualKey());
        if (KEY_NONE != key)
        {
            for (const auto &callBack : CPUTWindow::mKeyboardEventCallbacks) {
                handledCode = callBack(key, state);
            }
        }

		if(handledCode == CPUT_EVENT_UNHANDLED) {
			QWidget::keyReleaseEvent(event);
		}
	}

bool QDXWidget::nativeEvent( QByteArray const& eventType, void* message, long* result ) {

	MSG* msg = ( MSG* )message;

	if( !mWndProc.empty() ) {
		for( auto c : mWndProc ) {
            if( c( msg->hwnd, msg->message, msg->wParam, msg->lParam ) ) {
				return 1;
			}
		}
	}

	return false;
}

void QDXWidget::resizeEvent( QResizeEvent* event ) {

	int width, height;
	height = event->size().height();
	width = event->size().width();

	for( const auto& callBack : mResizeEventCallbacks ) {
		callBack( width, height );
	}
}

CPUTMouseState QDXWidget::ConvertMouseState( Qt::MouseButtons buttons, Qt::KeyboardModifiers wParam ) {
	CPUTMouseState eState = CPUT_MOUSE_NONE;
	if( wParam & Qt::ControlModifier )
		eState = ( CPUTMouseState ) (eState | static_cast<int>( CPUT_MOUSE_CTRL_DOWN ));

	if( wParam & Qt::ShiftModifier )
		eState = ( CPUTMouseState ) (eState | static_cast<int>( CPUT_MOUSE_SHIFT_DOWN ));

	if( buttons & Qt::LeftButton )
		eState = ( CPUTMouseState ) (eState | static_cast<int>( CPUT_MOUSE_LEFT_DOWN ));

	if( buttons & Qt::MiddleButton )
		eState = ( CPUTMouseState ) (eState | static_cast<int>( CPUT_MOUSE_MIDDLE_DOWN ));

	if( buttons & Qt::RightButton )
		eState = ( CPUTMouseState ) (eState | static_cast<int>( CPUT_MOUSE_RIGHT_DOWN ));


	return eState;
}

CPUTKey QDXWidget::ConvertVirtualKeyToCPUTKey( int wParam ) {
	switch( wParam ) {
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

void QDXWidget::keyReleaseEvent( QKeyEvent* event ) {
	CPUTEventHandledCode handledCode = CPUT_EVENT_UNHANDLED;
	CPUTKeyState state = CPUT_KEY_UP;
	CPUTKey key = ConvertVirtualKeyToCPUTKey( event->nativeVirtualKey() );
	if( KEY_NONE != key ) {
		for( const auto& callBack : mKeyboardEventCallbacks ) {
			handledCode = callBack( key, state );
		}
	}

	if( handledCode == CPUT_EVENT_UNHANDLED ) {
		QWidget::keyReleaseEvent( event );
	}
}

void QDXWidget::mouseMoveEvent( QMouseEvent* event ) {
	// }

	CPUTEventHandledCode handledCode = CPUT_EVENT_UNHANDLED;
	CPUTMouseState state = ConvertMouseState( event->buttons(), event->modifiers() );
	short xPos = event->x();
	short yPos = event->y();

	for( const auto& callBack : mMouseEventCallbacks ) {
		handledCode = callBack( xPos, yPos, 0, state, CPUT_EVENT_MOVE );
	}

	if( handledCode == CPUT_EVENT_UNHANDLED ) {
		QWidget::mouseMoveEvent( event );
	}
}

void QDXWidget::mousePressEvent( QMouseEvent* event ) {

	CPUTEventHandledCode handledCode = CPUT_EVENT_UNHANDLED;
	CPUTMouseState state = ConvertMouseState( event->buttons(), event->modifiers() );

	short xPos = event->x();
	short yPos = event->y();

	for( const auto& callBack : mMouseEventCallbacks ) {
		handledCode = callBack( xPos, yPos, 0, state, CPUT_EVENT_DOWN );
	}
}

void QDXWidget::mouseReleaseEvent( QMouseEvent* event ) {
	CPUTEventHandledCode handledCode = CPUT_EVENT_UNHANDLED;
	CPUTMouseState state = ConvertMouseState( event->buttons(), event->modifiers() );

	short xPos = event->x();
	short yPos = event->y();

	for( const auto& callBack : mMouseEventCallbacks ) {
		handledCode = callBack( xPos, yPos, 0, state, CPUT_EVENT_UP );
	}
}

void QDXWidget::wheelEvent( QWheelEvent* event ) {
	CPUTEventHandledCode handledCode = CPUT_EVENT_UNHANDLED;

	// get mouse position
	short xPos = event->x();
	short yPos = event->y();

	// get wheel delta
	int wheel = event->delta();

	for( const auto& callBack : mMouseEventCallbacks ) {
		handledCode = callBack( xPos, yPos, wheel, CPUT_MOUSE_WHEEL, CPUT_EVENT_WHEEL );
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

void* QDXWidget::GetNativeWindowHandle() const {
	auto id = winId();
	return reinterpret_cast<HWND>( id );
}

// Create window
//-----------------------------------------------------------------------------
CPUTResult QDXWidget::Create(const std::string WindowTitle, CPUTWindowCreationParams windowParams)
{
    return CPUT_SUCCESS;
}

// // Get the OS window dimensions - just working "client" area
// //-----------------------------------------------------------------------------
void QDXWidget::GetClientDimensions(int *pX, int *pY, int *pWidth, int *pHeight)
{
	*pX = pos().x();
	*pY = pos().y();
	*pWidth = width();
	*pHeight = height();
}


void QDXWidget::GetClientDimensions(int *pWidth, int *pHeight)
{
	*pWidth = width();
	*pHeight = height();
}

// Main message pump
//-----------------------------------------------------------------------------
int QDXWidget::StartMessageLoop() {
	return 0;
}
