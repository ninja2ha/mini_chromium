// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/events/win/keyboard_hook_win_base.h"

#include "crui/events/event.h"
#include "crui/events/keycodes/dom/dom_code.h"
#include "crui/events/win/events_win_utils.h"

namespace crui {

namespace {

bool IsMediaKey(DWORD vk) {
  return vk == VK_MEDIA_NEXT_TRACK || vk == VK_MEDIA_PREV_TRACK ||
         vk == VK_MEDIA_PLAY_PAUSE || vk == VK_MEDIA_STOP;
}

class MediaKeyboardHookWinImpl : public KeyboardHookWinBase {
 public:
  MediaKeyboardHookWinImpl(
      const MediaKeyboardHookWinImpl&) = delete;
  MediaKeyboardHookWinImpl& operator=(
      const MediaKeyboardHookWinImpl&) = delete;

  MediaKeyboardHookWinImpl(KeyEventCallback callback,
                           bool enable_hook_registration);
  ~MediaKeyboardHookWinImpl() override;

  // KeyboardHookWinBase implementation.
  bool ProcessKeyEventMessage(WPARAM w_param,
                              DWORD vk,
                              DWORD scan_code,
                              DWORD time_stamp) override;

  bool Register();

 private:
  static LRESULT CALLBACK ProcessKeyEvent(int code,
                                          WPARAM w_param,
                                          LPARAM l_param);

  static MediaKeyboardHookWinImpl* instance_;

  // Tracks the last non-located key down seen in order to determine if the
  // current key event should be marked as a repeated key press.
  DWORD last_key_down_ = 0;
};

// static
MediaKeyboardHookWinImpl* MediaKeyboardHookWinImpl::instance_ = nullptr;

MediaKeyboardHookWinImpl::MediaKeyboardHookWinImpl(
    KeyEventCallback callback,
    bool enable_hook_registration)
    : KeyboardHookWinBase(
          cr::Optional<cr::flat_set<DomCode>>(
              {DomCode::MEDIA_PLAY_PAUSE, DomCode::MEDIA_STOP,
               DomCode::MEDIA_TRACK_NEXT, DomCode::MEDIA_TRACK_PREVIOUS}),
          std::move(callback),
          enable_hook_registration) {}

MediaKeyboardHookWinImpl::~MediaKeyboardHookWinImpl() {
  if (!enable_hook_registration())
    return;

  CR_DCHECK(instance_ == this);
  instance_ = nullptr;
}

bool MediaKeyboardHookWinImpl::Register() {
  // Only one instance of this class can be registered at a time.
  CR_DCHECK(!instance_);
  instance_ = this;

  return KeyboardHookWinBase::Register(
      reinterpret_cast<HOOKPROC>(&MediaKeyboardHookWinImpl::ProcessKeyEvent));
}

// static
LRESULT CALLBACK MediaKeyboardHookWinImpl::ProcessKeyEvent(int code,
                                                           WPARAM w_param,
                                                           LPARAM l_param) {
  return KeyboardHookWinBase::ProcessKeyEvent(instance_, code, w_param,
                                              l_param);
}

bool MediaKeyboardHookWinImpl::ProcessKeyEventMessage(WPARAM w_param,
                                                      DWORD vk,
                                                      DWORD scan_code,
                                                      DWORD time_stamp) {
  if (!IsMediaKey(vk))
    return false;

  bool is_repeat = false;
  MSG msg = {nullptr, static_cast<UINT>(w_param), vk,
             GetLParamFromScanCode(static_cast<uint16_t>(scan_code)),
             time_stamp};
  EventType event_type = EventTypeFromMSG(msg);
  if (event_type == ET_KEY_PRESSED) {
    is_repeat = (last_key_down_ == vk);
    last_key_down_ = vk;
  } else {
    CR_DCHECK(event_type == ET_KEY_RELEASED);
    last_key_down_ = 0;
  }

  std::unique_ptr<KeyEvent> key_event =
      std::make_unique<KeyEvent>(KeyEventFromMSG(msg));
  if (is_repeat)
    key_event->set_flags(key_event->flags() | EF_IS_REPEAT);
  ForwardCapturedKeyEvent(key_event.get());

  // If the event is handled, don't propagate to the OS.
  return key_event->handled();
}

}  // namespace

// static
std::unique_ptr<KeyboardHook> KeyboardHook::CreateMediaKeyboardHook(
    KeyEventCallback callback) {
  std::unique_ptr<KeyboardHook> keyboard_hook =
      std::make_unique<MediaKeyboardHookWinImpl>(
          std::move(callback),
          /*enable_hook_registration=*/true);

  auto raw_ptr = static_cast<MediaKeyboardHookWinImpl*>(keyboard_hook.get());
  if (!raw_ptr->Register())
    return nullptr;

  return keyboard_hook;
}

std::unique_ptr<KeyboardHookWinBase>
KeyboardHookWinBase::CreateMediaKeyboardHookForTesting(
    KeyEventCallback callback) {
  return std::make_unique<MediaKeyboardHookWinImpl>(
      std::move(callback),
      /*enable_hook_registration=*/false);
}

}  // namespace crui
