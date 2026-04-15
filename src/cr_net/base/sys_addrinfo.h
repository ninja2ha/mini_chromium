// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This is a convenience header to pull in the platform-specific headers
// that define at least:
//
//     struct addrinfo
//     struct sockaddr*
//     getaddrinfo()
//     freeaddrinfo()
//     AI_*
//     AF_*
//
// Prefer including this file instead of directly writing the #if / #else,
// since it avoids duplicating the platform-specific selections.

#ifndef MINI_CHROMIUM_SRC_CRNET_BASE_SYS_ADDRINFO_H_
#define MINI_CHROMIUM_SRC_CRNET_BASE_SYS_ADDRINFO_H_

#include "cr_build/build_config.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "cr_base/win/windows_types.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#elif defined(MINI_CHROMIUM_OS_POSIX)
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#endif  // MINI_CHROMIUM_SRC_CRNET_BASE_SYS_ADDRINFO_H_