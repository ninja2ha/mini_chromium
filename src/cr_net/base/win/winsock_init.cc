// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_net/base/win/winsock_init.h"

#include "cr_base/memory/no_destructor.h"
#include "cr_base/logging/logging.h"
#include "cr_base/win/windows_types.h"

#include <winsock2.h>

namespace {

class WinsockInitSingleton {
 public:
  WinsockInitSingleton() {
    WORD winsock_ver = MAKEWORD(2, 2);
    WSAData wsa_data;
    bool did_init = (WSAStartup(winsock_ver, &wsa_data) == 0);
    if (did_init) {
      CR_DCHECK(wsa_data.wVersion == winsock_ver);

      // The first time WSAGetLastError is called, the delay load helper will
      // resolve the address with GetProcAddress and fixup the import.  If a
      // third party application hooks system functions without correctly
      // restoring the error code, it is possible that the error code will be
      // overwritten during delay load resolution.  The result of the first
      // call may be incorrect, so make sure the function is bound and future
      // results will be correct.
      WSAGetLastError();
    }
  }

  ~WinsockInitSingleton() {
    // Don't call WSACleanup() since the worker pool threads can continue to
    // call getaddrinfo() after Winsock has shutdown, which can lead to crashes.
  }

  static WinsockInitSingleton* GetInstance() {
    static cr::NoDestructor<WinsockInitSingleton> inst;
    return inst.get();
  }
};

}  // namespace

namespace crnet {

void EnsureWinsockInit() {
  WinsockInitSingleton::GetInstance();
}

}  // namespace crnet