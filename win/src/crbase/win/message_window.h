// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_CRBASE_WIN_MESSAGE_WINDOW_H_
#define MINI_CHROMIUM_CRBASE_WIN_MESSAGE_WINDOW_H_

#include <windows.h>

#include "crbase/base_export.h"
#include "crbase/functional/callback.h"
#include "crbase/compiler_specific.h"
#include "crbase/strings/string16.h"
#include "crbase/threading/thread_checker.h"

namespace cr {
namespace win {

// Implements a message-only window.
class CRBASE_EXPORT MessageWindow {
 public:
  // Used to register a process-wide message window class.
  class WindowClass;

  // Implement this callback to handle messages received by the message window.
  // If the callback returns |false|, the first four parameters are passed to
  // DefWindowProc(). Otherwise, |*result| is returned by the window procedure.
  using MessageCallback = cr::RepeatingCallback<
      bool(UINT message, WPARAM wparam, LPARAM lparam, LRESULT* result)>;

  MessageWindow(const MessageWindow&) = delete;
  MessageWindow& operator=(const MessageWindow&) = delete;

  MessageWindow();
  ~MessageWindow();

  // Creates a message-only window. The incoming messages will be passed by
  // |message_callback|. |message_callback| must outlive |this|.
  bool Create(MessageCallback message_callback);

  // Same as Create() but assigns the name to the created window.
  bool CreateNamed(MessageCallback message_callback,
                   const string16& window_name);

  HWND hwnd() const { return window_; }

  // Retrieves a handle of the first message-only window with matching
  // |window_name|.
  static HWND FindWindow(const string16& window_name);

 private:
  // Give |WindowClass| access to WindowProc().
  friend class WindowClass;

  // Contains the actual window creation code.
  bool DoCreate(MessageCallback message_callback, const wchar_t* window_name);

  // Invoked by the OS to process incoming window messages.
  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam,
                                     LPARAM lparam);

  // Invoked to handle messages received by the window.
  MessageCallback message_callback_;

  // Handle of the input window.
  HWND window_;

#if CR_DCHECK_IS_ON()
  cr::ThreadChecker thread_checker_;
#endif
};

}  // namespace win
}  // namespace cr

#endif  // MINI_CHROMIUM_CRBASE_WIN_MESSAGE_WINDOW_H_
