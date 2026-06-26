// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_NETWORK_NETWORK_CHANGE_NOTIFIER_FACTORY_H_
#define MINI_CHROMIUM_SRC_CRNET_NETWORK_NETWORK_CHANGE_NOTIFIER_FACTORY_H_

#include <memory>

#include "cr_net/net_export.h"

namespace crnet {

class NetworkChangeNotifier;
// NetworkChangeNotifierFactory provides a mechanism for overriding the default
// instance creation process of NetworkChangeNotifier.
class CRNET_EXPORT NetworkChangeNotifierFactory {
 public:
  NetworkChangeNotifierFactory() {}
  virtual ~NetworkChangeNotifierFactory() {}
  virtual std::unique_ptr<NetworkChangeNotifier> CreateInstance() = 0;
};

}  // namespace crnet

#endif  // MINI_CHROMIUM_SRC_CRNET_NETWORK_NETWORK_CHANGE_NOTIFIER_FACTORY_H_