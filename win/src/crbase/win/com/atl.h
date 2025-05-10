// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef MINI_CHROMIUM_SRC_CRBASE_WIN_COM_ATL_H_
#define MINI_CHROMIUM_SRC_CRBASE_WIN_COM_ATL_H_

// Check no prior poisonous defines were made.
#define StrCat StrCat
// Undefine before windows header will make the poisonous defines
#undef StrCat

#ifndef _ATL_NO_EXCEPTIONS
#define _ATL_NO_EXCEPTIONS
#endif

// atlwin.h relies on std::void_t, but libc++ doesn't define it unless
// _LIBCPP_STD_VER > 14.  Workaround this by manually defining it.
#include <type_traits>
#if defined(_LIBCPP_STD_VER) && _LIBCPP_STD_VER <= 14
namespace std {
template <class...>
using void_t = void;
}
#endif

#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#include <atlhost.h>
#include <atlsecurity.h>
#include <atlwin.h>

// Undefine the poisonous defines
#undef StrCat
// Check no poisonous defines follow this include
#define StrCat StrCat

#endif  // MINI_CHROMIUM_SRC_CRBASE_WIN_COM_ATL_H_
