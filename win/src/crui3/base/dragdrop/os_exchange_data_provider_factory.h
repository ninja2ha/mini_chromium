// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_DRAGDROP_OS_EXCHANGE_DATA_PROVIDER_FACTORY_H_
#define UI_BASE_DRAGDROP_OS_EXCHANGE_DATA_PROVIDER_FACTORY_H_

#include <memory>

#include "crui/base/dragdrop/os_exchange_data.h"
#include "crui/base/ui_export.h"

namespace crui {

// Builds platform specific OSExchangeDataProviders.
class CRUI_EXPORT OSExchangeDataProviderFactory {
 public:
  // Creates a Provider based on the current platform.
  static std::unique_ptr<OSExchangeData::Provider> CreateProvider();
};

}  // namespace crui

#endif  // UI_BASE_DRAGDROP_OS_EXCHANGE_DATA_PROVIDER_FACTORY_H_
