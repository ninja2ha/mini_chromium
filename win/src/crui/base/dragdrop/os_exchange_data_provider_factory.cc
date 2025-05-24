// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/base/dragdrop/os_exchange_data_provider_factory.h"

#include "crui/base/build_platform.h"

#if defined(MINI_CHROMIUM_USE_X11)
#include "crui/base/dragdrop/os_exchange_data_provider_aurax11.h"
#elif defined(MINI_CHROMIUM_OS_LINUX)
#include "crui/base/dragdrop/os_exchange_data_provider_aura.h"
#elif defined(MINI_CHROMIUM_OS_MACOSX)
#include "crui/base/dragdrop/os_exchange_data_provider_builder_mac.h"
#elif defined(MINI_CHROMIUM_OS_WIN)
///#include "crui/base/dragdrop/os_exchange_data_provider_win.h"
#endif

namespace crui {

//static
std::unique_ptr<OSExchangeData::Provider>
OSExchangeDataProviderFactory::CreateProvider() {
#if defined(MINI_CHROMIUM_USE_X11)
  return std::make_unique<OSExchangeDataProviderAuraX11>();
#elif defined(MINI_CHROMIUM_OS_LINUX)
  return std::make_unique<OSExchangeDataProviderAura>();
#elif defined(MINI_CHROMIUM_OS_MACOSX)
  return ui::BuildOSExchangeDataProviderMac();
#elif defined(MINI_CHROMIUM_OS_WIN)
  return nullptr;
  ///return std::make_unique<OSExchangeDataProviderWin>();
#else
#error "Unknown operating system"
#endif
}

}  // namespace crui
