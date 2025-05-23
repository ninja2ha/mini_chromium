// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_ACCELERATORS_SYSTEM_MEDIA_CONTROLS_MEDIA_KEYS_LISTENER_H_
#define UI_BASE_ACCELERATORS_SYSTEM_MEDIA_CONTROLS_MEDIA_KEYS_LISTENER_H_

#include "crbase/containers/flat_set.h"
#include "components/system_media_controls/system_media_controls_observer.h"
#include "crui/base/accelerators/media_keys_listener.h"
#include "crui/base/ui_export.h"
#include "crui/events/keycodes/keyboard_codes.h"

namespace system_media_controls {
class SystemMediaControls;
}  // namespace system_media_controls

namespace crui {

// Implementation of MediaKeysListener that uses the SystemMediaControls API to
// listen for media key presses. It only allows for a single instance to be
// created in order to prevent conflicts from multiple listeners.
class CRUI_EXPORT SystemMediaControlsMediaKeysListener
    : public MediaKeysListener/*,
      public system_media_controls::SystemMediaControlsObserver*/ {
 public:
  SystemMediaControlsMediaKeysListener(
      const SystemMediaControlsMediaKeysListener&) = delete;
  SystemMediaControlsMediaKeysListener& operator=(
      const SystemMediaControlsMediaKeysListener&) = delete;

  explicit SystemMediaControlsMediaKeysListener(
      MediaKeysListener::Delegate* delegate);
  ~SystemMediaControlsMediaKeysListener() override;

  static bool has_instance() { return has_instance_; }

  bool Initialize();

  // MediaKeysListener implementation.
  bool StartWatchingMediaKey(KeyboardCode key_code) override;
  void StopWatchingMediaKey(KeyboardCode key_code) override;
  void SetIsMediaPlaying(bool is_playing) override;

  // system_media_controls::SystemMediaControlsObserver implementation.
  void OnServiceReady() override {}
  void OnNext() override;
  void OnPrevious() override;
  void OnPlay() override;
  void OnPause() override;
  void OnPlayPause() override;
  void OnStop() override;

  void SetSystemMediaControlsForTesting(
      system_media_controls::SystemMediaControls* service) {
    service_ = service;
  }

 private:
  static bool has_instance_;

  // Sends the key code to the delegate if the delegate has asked for it.
  void MaybeSendKeyCode(KeyboardCode key_code);

  MediaKeysListener::Delegate* delegate_;

  // Set of keys codes that we're currently listening for.
  cr::flat_set<KeyboardCode> key_codes_;

  system_media_controls::SystemMediaControls* service_ = nullptr;

  bool is_media_playing_ = false;
};

}  // namespace crui

#endif  // UI_BASE_ACCELERATORS_SYSTEM_MEDIA_CONTROLS_MEDIA_KEYS_LISTENER_H_
