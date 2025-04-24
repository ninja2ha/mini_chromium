// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/at_exit.h"

#include <stddef.h>
#include <ostream>
#include <utility>

#include "crbase/logging.h"
#include "crbase/functional/bind.h"
#include "crbase/functional/callback.h"

namespace cr {

// Keep a stack of registered AtExitManagers.  We always operate on the most
// recent, and we should never have more than one outside of testing (for a
// statically linked version of this library).  Testing may use the shadow
// version of the constructor, and if we are building a dynamic library we may
// end up with multiple AtExitManagers on the same process.  We don't protect
// this for thread-safe access, since it will only be modified in testing.
static AtExitManager* g_top_manager = nullptr;

static bool g_disable_managers = false;

AtExitManager::AtExitManager() : next_manager_(g_top_manager) {
// If multiple modules instantiate AtExitManagers they'll end up living in this
// module... they have to coexist.
#if !defined(MINI_CHROMIUM_COMPONENT_BUILD)
  CR_DCHECK(!g_top_manager);
#endif
  g_top_manager = this;
}

AtExitManager::~AtExitManager() {
  if (!g_top_manager) {
    CR_NOTREACHED() << "Tried to ~AtExitManager without an AtExitManager";
    return;
  }
  CR_DCHECK(this == g_top_manager);

  if (!g_disable_managers)
    ProcessCallbacksNow();
  g_top_manager = next_manager_;
}

// static
void AtExitManager::RegisterCallback(AtExitCallbackType func, void* param) {
  CR_DCHECK(func);
  RegisterTask(cr::BindOnce(func, param));
}

// static
void AtExitManager::RegisterTask(cr::OnceClosure task) {
  if (!g_top_manager) {
    CR_NOTREACHED() << "Tried to RegisterCallback without an AtExitManager";
    return;
  }

  AutoLock lock(g_top_manager->lock_);
  g_top_manager->stack_.push(std::move(task));
}

// static
void AtExitManager::ProcessCallbacksNow() {
  if (!g_top_manager) {
    CR_NOTREACHED() << "Tried to ProcessCallbacksNow without an AtExitManager";
    return;
  }

  // Callbacks may try to add new callbacks, so run them without holding
  // |lock_|. This is an error and caught by the DCHECK in RegisterTask(), but
  // handle it gracefully in release builds so we don't deadlock.
  std::stack<cr::OnceClosure> tasks;
  {
    AutoLock lock(g_top_manager->lock_);
    tasks.swap(g_top_manager->stack_);
  }

  while (!tasks.empty()) {
    std::move(tasks.top()).Run();
    tasks.pop();
  }
}

void AtExitManager::DisableAllAtExitManagers() {
  AutoLock lock(g_top_manager->lock_);
  g_disable_managers = true;
}

AtExitManager::AtExitManager(bool shadow) : next_manager_(g_top_manager) {
  CR_DCHECK(shadow || !g_top_manager);
  g_top_manager = this;
}

}  // namespace cr