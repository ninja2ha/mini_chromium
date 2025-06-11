// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_VIEW_PROP_H_
#define UI_BASE_VIEW_PROP_H_

#include "crbase/memory/ref_counted.h"
#include "crui/base/ui_export.h"
#include "crui/gfx/native_widget_types.h"

#if !defined(MINI_CHROMIUM_OS_WIN) && !defined(MINI_CHROMIUM_USE_AURA)
#error view_prop.h is only for windows and aura builds.
#endif

namespace crui {

// ViewProp maintains a key/value pair for a particular view. ViewProp is
// designed as a replacement for the Win32's SetProp, but does not make use of
// window manager memory. ViewProp shares similar semantics as SetProp, the
// value for a particular view/key pair comes from the last ViewProp created.
class CRUI_EXPORT ViewProp {
 public:
  // Associates data with a view/key pair. If a ViewProp has already been
  // created for the specified pair |data| replaces the current value.
  //
  // ViewProp does *not* make a copy of the char*, the pointer is used for
  // sorting.
  ViewProp(gfx::AcceleratedWidget view, const char* key, void* data);
  ~ViewProp();

  ViewProp(const ViewProp&) = delete;
  ViewProp& operator=(const ViewProp&) = delete;

  // Returns the value associated with the view/key pair, or NULL if there is
  // none.
  static void* GetValue(gfx::AcceleratedWidget view, const char* key);

  // Returns the key.
  const char* Key() const;

 private:
  class Data;

  // Stores the actual data.
  cr::RefPtr<Data> data_;
};

}  // namespace crui

#endif  // UI_BASE_VIEW_PROP_H_
