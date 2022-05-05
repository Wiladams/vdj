#pragma once

/*
	The primary benefit of the apphost.h/appmain.cpp combination is to turn
	windows specifics into simpler concepts.

	Typical Windows messages, such as mouse and keyboard, and turned into 
	topics, that other applications can subscribe to.

	A 'main()' is provided, so the application can implement a 'setup()'
	instead, and not have to worry about whether it is a console or Window
	target.

	All other aspects of the application are up to the application environment,
	but at least all the Win32 specific stuff is wrapped up.
*/


#include <SDKDDKVer.h>



#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1

#include <windows.h>

#include <windowsx.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mmsystem.h>

#include "apidefs.h"

#include <memory>
#include <deque>
#include <functional>
#include <stdio.h>
#include <string>

#include "pubsub.h"
#include "NativeWindow.hpp"
#include "User32PixelMap.h"
#include "joystick.h"
#include "uievent.h"

/*
	The interface the implementing application sees is 'C'.  This 
	makes binding much easier for scripting languages, and allows
	usage across different languages, without having to worry about
	the C++ name mangling that would occur otherwise.
*/

#ifdef __cplusplus
extern "C" {
#endif


typedef void (* VOIDROUTINE)();
typedef void (*PFNDOUBLE1)(const double param);

// Miscellaneous globals
APP_EXPORT extern int gargc;
APP_EXPORT extern char **gargv;

// The window that controls the application
APP_EXPORT extern User32Window * gAppWindow;

// The surface that is in the content area of the application window
// this can be drawn to directly.
APP_EXPORT extern std::shared_ptr<alib::U32DIBSection> gAppSurface;



// Globals we expect the user to consume
// These values are available once onLoad() is called
// diplayWidth/Height, the number of pixels on the primary monitor
APP_EXPORT extern int displayWidth;
APP_EXPORT extern int displayHeight;

// The actual physical dots per inch of the primary monitor
APP_EXPORT extern unsigned int systemDpi;
APP_EXPORT extern unsigned int systemPpi;

APP_EXPORT extern ptrdiff_t canvasWidth;
APP_EXPORT extern ptrdiff_t canvasHeight;



// The various 'onxxx' routines are meant to be implemented by
// application environment code.  If they are implemented
// the runtime will load them in and call them at appropriate times
// if they are not implemented, they simply won't be called.

// onLoad()
// When the runtime has done all that it needs to do, it will
// call onLoad() and give the application a chance to do whatever
// it wants assuming a setup environment.  
// onLoad() should be brief and allow the application to 
// continue running.  It should not be long lived.
APP_EXPORT void onLoad();

// onUnload()
// This is called once the application runtime is about to stop.
// This gives the application a chance to clean up anything it
// needs to before the application finally closes.
APP_EXPORT void onUnload();

// onLoop()
// called each time through application main loop.  This is not
// timed, and will execute after Windows messages have been processed
APP_EXPORT void onLoop();	


// The following routines are part of the application runtime, 
// and are meant to be called by the implementing application.
// They control the lifetime of the environment, creation 
// of primary window and whether various parts of the IO system 
// are present



// halt()
// Stop the application main loop from continuing
// immediately.
APP_EXPORT void halt();

// refreshScreen()
// tell the application window to draw its
// contents on the screen immediately
APP_EXPORT void refreshScreen();

// layered()
// makes the window a popup, which removes its title bar
// and makes opacity a per pixel thing
APP_EXPORT void layered();
APP_EXPORT void noLayered();
APP_EXPORT bool isLayered();

// Various Application window routines
// 
// windowOpacity()
// window opacity can be set with a value between [0..1]
// if it's set to 1.0, the window will fully opaque
// and no longer layered
// with values down to 0.0, the window will become a 
// layered window and transparent.  The transparency here
// is for the entire window, not for individual pixels
APP_EXPORT void windowOpacity(float o);	// set overall opacity of window
APP_EXPORT void showAppWindow();
APP_EXPORT void show();
APP_EXPORT void hide();
APP_EXPORT void setWindowPosition(int x, int y);
APP_EXPORT bool setWindowSize(size_t aWidth, size_t aHeight);


// Dealing with raw input
// enabling rawInput() will cause the runtime
// to call the rawxxx() functions if they exist in the
// implementing application
APP_EXPORT void rawInput();
APP_EXPORT void noRawInput();
APP_EXPORT void rawMouse();
APP_EXPORT void rawKeyboard();
APP_EXPORT void rawJoystick();

// Managing joystick input
// different than rawJoystick(), legacyJoystick() will
// implement a more limited joystick interface
APP_EXPORT void legacyJoystick();
APP_EXPORT void noJoystick();

// Touch routines apps can implement
APP_EXPORT bool touch();
APP_EXPORT bool noTouch();
APP_EXPORT bool isTouch();

// Turn on/off file drop handling
APP_EXPORT bool enableFileDrops();
APP_EXPORT bool disableFileDrops();

// Turn cursor on
APP_EXPORT void cursor();
// Turn cursor off
APP_EXPORT void noCursor();


#ifdef __cplusplus
}
#endif

// Make Topic publishers available
using SignalEventTopic = Topic<intptr_t>;


// Doing C++ pub/sub
using MouseEventTopic = Topic<MouseEvent&>;
using KeyboardEventTopic = Topic<KeyboardEvent&>;
using JoystickEventTopic = Topic<JoystickEvent&>;
using FileDropEventTopic = Topic<FileDropEvent&>;
using TouchEventTopic = Topic<TouchEvent&>;
using PointerEventTopic = Topic<PointerEvent&>;

// Convenience functions for registering subscribers
APP_EXPORT void subscribe(SignalEventTopic::Subscriber s);
APP_EXPORT void subscribe(MouseEventTopic::Subscriber s);
APP_EXPORT void subscribe(KeyboardEventTopic::Subscriber s);
APP_EXPORT void subscribe(JoystickEventTopic::Subscriber s);
APP_EXPORT void subscribe(FileDropEventTopic::Subscriber s);
APP_EXPORT void subscribe(TouchEventTopic::Subscriber s);
APP_EXPORT void subscribe(PointerEventTopic::Subscriber s);
