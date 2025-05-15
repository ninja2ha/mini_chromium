// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_FOCUS_FOCUS_MANAGER_FACTORY_H_
#define UI_VIEWS_FOCUS_FOCUS_MANAGER_FACTORY_H_

#include <memory>

#include "crui/base/ui_export.h"

namespace crui {
namespace views {

class FocusManager;
class Widget;

// A factory to create FocusManager. This is used in unit tests
// to inject a custom factory.
class CRUI_EXPORT FocusManagerFactory {
 public:
  FocusManagerFactory(const FocusManagerFactory&) = delete;
  FocusManagerFactory& operator=(const FocusManagerFactory&) = delete;

  // Create a FocusManager for the given |widget| using the installed Factory.
  static std::unique_ptr<FocusManager> Create(Widget* widget);

  // Installs FocusManagerFactory. If |factory| is NULL, it resets
  // to the default factory which creates plain FocusManager.
  static void Install(FocusManagerFactory* factory);

 protected:
  FocusManagerFactory();
  virtual ~FocusManagerFactory();

  // Create a FocusManager for the given |widget|.
  virtual std::unique_ptr<FocusManager> CreateFocusManager(Widget* widget) = 0;
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_FOCUS_FOCUS_MANAGER_FACTORY_H_
