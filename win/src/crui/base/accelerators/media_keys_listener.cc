// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/base/accelerators/media_keys_listener.h"

namespace crui {

MediaKeysListener::Delegate::~Delegate() = default;

MediaKeysListener::~MediaKeysListener() = default;

// static
bool MediaKeysListener::IsMediaKeycode(KeyboardCode key_code) {
  return key_code == VKEY_MEDIA_PLAY_PAUSE || key_code == VKEY_MEDIA_STOP ||
         key_code == VKEY_MEDIA_PREV_TRACK || key_code == VKEY_MEDIA_NEXT_TRACK;
}

}  // namespace crui
