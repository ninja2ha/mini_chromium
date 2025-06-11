// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/focus/focus_manager_factory.h"

#include "crui/views/focus/focus_manager.h"
#include "crui/views/focus/focus_manager_delegate.h"

namespace crui {
namespace views {

namespace {

class DefaultFocusManagerFactory : public FocusManagerFactory {
 public:
  DefaultFocusManagerFactory(
      const DefaultFocusManagerFactory&) = delete;
  DefaultFocusManagerFactory& operator=(
      const DefaultFocusManagerFactory&) = delete;

  DefaultFocusManagerFactory() = default;
  ~DefaultFocusManagerFactory() override = default;

 protected:
  std::unique_ptr<FocusManager> CreateFocusManager(Widget* widget) override {
    return std::make_unique<FocusManager>(widget, nullptr /* delegate */);
  }
};

FocusManagerFactory* g_focus_manager_factory = nullptr;

}  // namespace

FocusManagerFactory::FocusManagerFactory() = default;

FocusManagerFactory::~FocusManagerFactory() = default;

// static
std::unique_ptr<FocusManager> FocusManagerFactory::Create(Widget* widget) {
  if (!g_focus_manager_factory)
    g_focus_manager_factory = new DefaultFocusManagerFactory();
  return g_focus_manager_factory->CreateFocusManager(widget);
}

// static
void FocusManagerFactory::Install(FocusManagerFactory* f) {
  if (f == g_focus_manager_factory)
    return;
  delete g_focus_manager_factory;
  g_focus_manager_factory = f ? f : new DefaultFocusManagerFactory();
}

}  // namespace views
}  // namespace crui
