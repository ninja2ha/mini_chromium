// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/base/win/scoped_ole_initializer.h"

#include <ole2.h>

#include "crbase/logging.h"

namespace crui {

ScopedOleInitializer::ScopedOleInitializer() : hr_(OleInitialize(NULL)) {
  CR_DCHECK(OLE_E_WRONGCOMPOBJ != hr_) << "Incompatible DLLs on machine";
  CR_DCHECK(RPC_E_CHANGED_MODE != hr_) << "Invalid COM thread model change";
}

ScopedOleInitializer::~ScopedOleInitializer() {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  if (SUCCEEDED(hr_))
    OleUninitialize();
}

}  // namespace crui
