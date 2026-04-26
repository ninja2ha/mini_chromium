// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_ipc/connection_params.h"

#include <utility>

namespace cripc {

ConnectionParams::ConnectionParams() = default;

ConnectionParams::ConnectionParams(ChannelEndpoint endpoint)
    : endpoint_(std::move(endpoint)) {}

ConnectionParams::ConnectionParams(
    ChannelServerEndpoint server_endpoint)
    : server_endpoint_(std::move(server_endpoint)) {}

ConnectionParams::ConnectionParams(ConnectionParams&&) = default;

ConnectionParams::~ConnectionParams() = default;

ConnectionParams& ConnectionParams::operator=(ConnectionParams&& params) =
    default;

}  // namespace cripc
