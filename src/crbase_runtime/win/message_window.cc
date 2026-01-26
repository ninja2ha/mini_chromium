// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase_runtime/win/message_window.h"

#include "crbase/logging/logging.h"
#include "crbase/strings/string_util.h"
#include "crbase/memory/singleton.h"
#include "crbase/win/current_module.h"

const wchar_t kMessageWindowClassName[] = L"Chrome_MessageWindow";

namespace cr {
namespace win {

// Used along with LazyInstance to register a window class for message-only
// windows created by MessageWindow.
class MessageWindow::WindowClass {
 public:
  WindowClass(const WindowClass&) = delete;
  WindowClass& operator=(const WindowClass&) = delete;

  static WindowClass* GetInstance();

  WindowClass();
  ~WindowClass();

  ATOM atom() { return atom_; }
  HINSTANCE instance() { return instance_; }

 private:
  friend struct cr::DefaultSingletonTraits<WindowClass>;

  ATOM atom_ = 0;
  HINSTANCE instance_ = CR_CURRENT_MODULE();
};

MessageWindow::WindowClass* MessageWindow::WindowClass::GetInstance() {
  return cr::Singleton<MessageWindow::WindowClass>::get();
}

MessageWindow::WindowClass::WindowClass() {
  WNDCLASSEX window_class;
  window_class.cbSize = sizeof(window_class);
  window_class.style = 0;
  window_class.lpfnWndProc = WindowProc;
  window_class.cbClsExtra = 0;
  window_class.cbWndExtra = 0;
  window_class.hInstance = instance_;
  window_class.hIcon = nullptr;
  window_class.hCursor = nullptr;
  window_class.hbrBackground = nullptr;
  window_class.lpszMenuName = nullptr;
  window_class.lpszClassName = kMessageWindowClassName;
  window_class.hIconSm = nullptr;
  atom_ = ::RegisterClassExW(&window_class);
  if (atom_ == 0) {
    CR_PLOG(Error)
        << "Failed to register the window class for a message-only window";
  }
}

MessageWindow::WindowClass::~WindowClass() {
  if (atom_ != 0) {
    BOOL result = ::UnregisterClassW(MAKEINTATOMW(atom_), instance_);
    // Hitting this DCHECK usually means that some MessageWindow objects were
    // leaked. For example not calling
    // ui::Clipboard::DestroyClipboardForCurrentThread() results in a leaked
    // MessageWindow.
    CR_DCHECK(result);
  }
}

MessageWindow::MessageWindow() = default;

MessageWindow::~MessageWindow() {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  if (window_ != nullptr) {
    BOOL result = DestroyWindow(window_);
    CR_DCHECK(result);
  }
}

bool MessageWindow::Create(MessageCallback message_callback) {
  return DoCreate(std::move(message_callback), nullptr);
}

bool MessageWindow::CreateNamed(MessageCallback message_callback,
                                const std::wstring& window_name) {
  return DoCreate(std::move(message_callback), window_name.c_str());
}

// static
HWND MessageWindow::FindWindow(const std::wstring& window_name) {
  return ::FindWindowExW(HWND_MESSAGE, nullptr, kMessageWindowClassName,
                         window_name.c_str());
}

bool MessageWindow::DoCreate(MessageCallback message_callback,
                             const wchar_t* window_name) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  CR_DCHECK(message_callback_.is_null());
  CR_DCHECK(!window_);

  message_callback_ = std::move(message_callback);

  WindowClass* window_class = WindowClass::GetInstance();
  window_ = ::CreateWindowW(
      MAKEINTATOMW(window_class->atom()), window_name, 0, 0, 0, 0, 0,
      HWND_MESSAGE, nullptr, window_class->instance(), this);
  if (!window_) {
    CR_PLOG(Error) << "Failed to create a message-only window";
    return false;
  }

  return true;
}

// static
LRESULT CALLBACK MessageWindow::WindowProc(HWND hwnd,
                                           UINT message,
                                           WPARAM wparam,
                                           LPARAM lparam) {
  MessageWindow* self = reinterpret_cast<MessageWindow*>(
      ::GetWindowLongPtrW(hwnd, GWLP_USERDATA));

  switch (message) {
    // Set up the self before handling WM_CREATE.
    case WM_CREATE: {
      CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lparam);
      self = reinterpret_cast<MessageWindow*>(cs->lpCreateParams);

      // Make |hwnd| available to the message handler. At this point the control
      // hasn't returned from CreateWindow() yet.
      self->window_ = hwnd;

      // Store pointer to the self to the window's user data.
      SetLastError(ERROR_SUCCESS);
      LONG_PTR result = ::SetWindowLongPtrW(hwnd, GWLP_USERDATA,
                                            reinterpret_cast<LONG_PTR>(self));
      CR_CHECK(result != 0 || GetLastError() == ERROR_SUCCESS);
      break;
    }

    // Clear the pointer to stop calling the self once WM_DESTROY is
    // received.
    case WM_DESTROY: {
      SetLastError(ERROR_SUCCESS);
      LONG_PTR result = ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, NULL);
      CR_CHECK(result != 0 || GetLastError() == ERROR_SUCCESS);
      break;
    }
  }

  // Handle the message.
  if (self) {
    LRESULT message_result;
    if (self->message_callback_.Run(message, wparam, lparam, &message_result))
      return message_result;
  }

  return ::DefWindowProcW(hwnd, message, wparam, lparam);
}

}  // namespace win
}  // namespace cr