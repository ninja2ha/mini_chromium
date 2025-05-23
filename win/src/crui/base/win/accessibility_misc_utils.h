// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef UI_BASE_WIN_ACCESSIBILITY_MISC_UTILS_H_
#define UI_BASE_WIN_ACCESSIBILITY_MISC_UTILS_H_

#include "crbase/win/com/atl.h"  // Must be before UIAutomationCore.h

#include <UIAutomationCore.h>

#include "crbase/compiler_specific.h"
#include "crbase/strings/string16.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace win {

  // UIA Text provider implementation for edit controls.
class CRUI_EXPORT UIATextProvider
    : public CComObjectRootEx<CComMultiThreadModel>,
      public ITextProvider {
 public:
  BEGIN_COM_MAP(UIATextProvider)
    COM_INTERFACE_ENTRY2(IUnknown, ITextProvider)
    COM_INTERFACE_ENTRY(ITextProvider)
  END_COM_MAP()

  UIATextProvider();
  ~UIATextProvider();

  // Creates an instance of the UIATextProvider class.
  // Returns true on success
  static bool CreateTextProvider(const cr::string16& value,
                                 bool editable,
                                 IUnknown** provider);

  void set_editable(bool editable) {
    editable_ = editable;
  }

  void set_value(const cr::string16& value) { value_ = value; }

  //
  // ITextProvider methods.
  //
  STDMETHOD(GetSelection)(SAFEARRAY** ret) override;

  STDMETHOD(GetVisibleRanges)(SAFEARRAY** ret) override;

  STDMETHOD(RangeFromChild)(IRawElementProviderSimple* child,
                            ITextRangeProvider** ret) override;

  STDMETHOD(RangeFromPoint)(struct UiaPoint point,
                            ITextRangeProvider** ret) override;

  STDMETHOD(get_DocumentRange)(ITextRangeProvider** ret) override;

  STDMETHOD(get_SupportedTextSelection)(
      enum SupportedTextSelection* ret) override;

 private:
  bool editable_;
  cr::string16 value_;
};

}  // win
}  // crui

#endif  // UI_BASE_WIN_ACCESSIBILITY_MISC_UTILS_H_
