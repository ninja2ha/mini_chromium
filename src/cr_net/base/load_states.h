// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_BASE_LOAD_STATES_H_
#define MINI_CHROMIUM_SRC_CRNET_BASE_LOAD_STATES_H_

#include <string>

namespace crnet {

// These states correspond to the lengthy periods of time that a resource load
// may be blocked and unable to make progress.
enum LoadState {

#define LOAD_STATE(label, value) LOAD_STATE_##label,
#include "cr_net/base/internal/load_states_list.h"
#undef LOAD_STATE

};

// Some states, like LOAD_STATE_WAITING_FOR_DELEGATE, are associated with extra
// data that describes more precisely what the delegate (for example) is doing.
// This class provides an easy way to hold a load state with an extra parameter.
struct LoadStateWithParam {
  LoadState state;
  std::u16string param;
  LoadStateWithParam() : state(LOAD_STATE_IDLE) {}
  LoadStateWithParam(LoadState state, const std::u16string& param)
      : state(state), param(param) {}
};

}  // namespace crnet

#endif  // MINI_CHROMIUM_SRC_CRNET_BASE_LOAD_STATES_H_
