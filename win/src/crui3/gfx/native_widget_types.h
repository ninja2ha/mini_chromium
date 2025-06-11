// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_NATIVE_WIDGET_TYPES_H_
#define UI_GFX_NATIVE_WIDGET_TYPES_H_

#include <stdint.h>

#include "crbase/logging.h"
#include "crui/base/ui_export.h"
#include "crui/base/build_platform.h"

#if defined(MINI_CHROMIUM_OS_MACOSX)
#include <objc/objc.h>
#elif defined(MINI_CHROMIUM_OS_WIN)
#include <windows.h>
#endif

// This file provides cross platform typedefs for native widget types.
//   NativeWindow: this is a handle to a native, top-level window
//   NativeView: this is a handle to a native UI element. It may be the
//     same type as a NativeWindow on some platforms.
//   NativeViewId: Often, in our cross process model, we need to pass around a
//     reference to a "window". This reference will, say, be echoed back from a
//     renderer to the browser when it wishes to query its size. On Windows we
//     use an HWND for this.
//
//     As a rule of thumb - if you're in the renderer, you should be dealing
//     with NativeViewIds. This should remind you that you shouldn't be doing
//     direct operations on platform widgets from the renderer process.
//
//     If you're in the browser, you're probably dealing with NativeViews,
//     unless you're in the IPC layer, which will be translating between
//     NativeViewIds from the renderer and NativeViews.
//
// The name 'View' here meshes with OS X where the UI elements are called
// 'views' and with our Chrome UI code where the elements are also called
// 'views'.

#if defined(MINI_CHROMIUM_USE_AURA)
namespace crui {
namespace aura {
class Window;
} // namespace aura

class Cursor;
enum class CursorType;
class Event;
}  // namespace crui
#endif  // defined(MINI_CHROMIUM_OS_WIN) || defined(MINI_CHROMIUM_OS_LINUX)

#if defined(MINI_CHROMIUM_OS_WIN)
#include <windows.h>
typedef struct HFONT__* HFONT;
struct IAccessible;
#elif defined(MINI_CHROMIUM_OS_MACOSX)
struct CGContext;
#ifdef __OBJC__
@class NSCursor;
@class NSEvent;
@class NSFont;
@class NSImage;
@class NSView;
@class NSWindow;
@class NSTextField;
#else
class NSCursor;
class NSEvent;
class NSFont;
class NSImage;
struct NSView;
class NSWindow;
class NSTextField;
#endif  // __OBJC__
#endif // defined(MINI_CHROMIUM_OS_WIN)

class SkBitmap;

#if defined(MINI_CHROMIUM_USE_X11) // using x11 on linux
extern "C" {
struct _AtkObject;
typedef struct _AtkObject AtkObject;
}
#endif

namespace crui {
namespace gfx {

#if defined(MINI_CHROMIUM_USE_AURA)
typedef crui::Cursor NativeCursor;
typedef aura::Window* NativeView;
typedef aura::Window* NativeWindow;
typedef crui::Event* NativeEvent;
constexpr NativeView kNullNativeView = nullptr;
constexpr NativeWindow kNullNativeWindow = nullptr;
#elif defined(MINI_CHROMIUM_OS_WIN)
typedef HCURSOR NativeCursor;
typedef HWND NativeView;
typedef HWND NativeWindow;
typedef MSG NativeEvent;
constexpr NativeView kNullNativeView = nullptr;
constexpr NativeWindow kNullNativeWindow = nullptr;
#elif defined(MINI_CHROMIUM_OS_MACOSX)
typedef NSCursor* NativeCursor;
typedef NSEvent* NativeEvent;
// NativeViews and NativeWindows on macOS are not necessarily in the same
// process as the NSViews and NSWindows that they represent. Require an
// explicit function call (GetNativeNSView or GetNativeNSWindow) to retrieve
// the underlying NSView or NSWindow.
// https://crbug.com/893719
class CRUI_EXPORT NativeView {
 public:
  constexpr NativeView() {}
  // TODO(ccameron): Make this constructor explicit.
  constexpr NativeView(NSView* ns_view) : ns_view_(ns_view) {}

  // This function name is verbose (that is, not just GetNSView) so that it
  // is easily grep-able.
  NSView* GetNativeNSView() const { return ns_view_; }

  operator bool() const { return ns_view_ != 0; }
  bool operator==(const NativeView& other) const {
    return ns_view_ == other.ns_view_;
  }
  bool operator!=(const NativeView& other) const {
    return ns_view_ != other.ns_view_;
  }
  bool operator<(const NativeView& other) const {
    return ns_view_ < other.ns_view_;
  }

 private:
  NSView* ns_view_ = nullptr;
};
class CRUI_EXPORT NativeWindow {
 public:
  constexpr NativeWindow() {}
  // TODO(ccameron): Make this constructor explicit.
  constexpr NativeWindow(NSWindow* ns_window) : ns_window_(ns_window) {}

  // This function name is verbose (that is, not just GetNSWindow) so that it
  // is easily grep-able.
  NSWindow* GetNativeNSWindow() const { return ns_window_; }

  operator bool() const { return ns_window_ != 0; }
  bool operator==(const NativeWindow& other) const {
    return ns_window_ == other.ns_window_;
  }
  bool operator!=(const NativeWindow& other) const {
    return ns_window_ != other.ns_window_;
  }
  bool operator<(const NativeWindow& other) const {
    return ns_window_ < other.ns_window_;
  }

 private:
  NSWindow* ns_window_ = nullptr;
};
constexpr NativeView kNullNativeView = NativeView(nullptr);
constexpr NativeWindow kNullNativeWindow = NativeWindow(nullptr);
#else
#error Unknown build environment.
#endif

#if defined(MINI_CHROMIUM_OS_WIN)
typedef HFONT NativeFont;
typedef IAccessible* NativeViewAccessible;
#elif defined(MINI_CHROMIUM_OS_MACOSX)
typedef NSFont* NativeFont;
typedef id NativeViewAccessible;
#else  // Android, Linux, Chrome OS, etc.
// Linux doesn't have a native font type.
#if defined(MINI_CHROMIUM_USE_X11)
typedef AtkObject* NativeViewAccessible;
#else
typedef struct _UnimplementedNativeViewAccessible
    UnimplementedNativeViewAccessible;
typedef UnimplementedNativeViewAccessible* NativeViewAccessible;
#endif
#endif

// A constant value to indicate that gfx::NativeCursor refers to no cursor.
#if defined(MINI_CHROMIUM_USE_AURA)
const crui::CursorType kNullCursor = static_cast<crui::CursorType>(-1);
#else
const gfx::NativeCursor kNullCursor = static_cast<gfx::NativeCursor>(NULL);
#endif

// Note: for test_shell we're packing a pointer into the NativeViewId. So, if
// you make it a type which is smaller than a pointer, you have to fix
// test_shell.
//
// See comment at the top of the file for usage.
typedef intptr_t NativeViewId;

// AcceleratedWidget provides a surface to compositors to paint pixels.
#if defined(MINI_CHROMIUM_OS_WIN)
typedef HWND AcceleratedWidget;
constexpr AcceleratedWidget kNullAcceleratedWidget = NULL;
#elif defined(MINI_CHROMIUM_USE_X11)
typedef unsigned long AcceleratedWidget;
constexpr AcceleratedWidget kNullAcceleratedWidget = 0;
#elif defined(MINI_CHROMIUM_OS_MACOSX)
typedef uint64_t AcceleratedWidget;
constexpr AcceleratedWidget kNullAcceleratedWidget = 0;
#else
#error unknown platform
#endif

}  // namespace gfx
}  // namespace crui

#endif  // UI_GFX_NATIVE_WIDGET_TYPES_H_
