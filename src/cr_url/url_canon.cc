// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_url/url_canon.h"

#include "cr_url/url_export.h"

namespace crurl {

#if !defined(MINI_CHROMIUM_COMPILER_MSVC)
template class CRURL_EXPORT CanonOutputT<char>;
template class CRURL_EXPORT CanonOutputT<char16_t>;
#endif

}  // namespace crurl
