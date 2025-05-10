// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/display/win/uwp_text_scale_factor.h"

#include <windows.h>

#include <windows.ui.viewmanagement.h>
#include <wrl/client.h>
#include <wrl/event.h>

#include <memory>

#include "crbase/memory/lazy_instance.h"
#include "crbase/strings/string_piece.h"
#include "crbase/threading/thread_checker.h"
#include "crbase/win/com/core_winrt_util.h"
#include "crbase/win/com/scoped_com_initializer.h"
#include "crbase/win/com/scoped_hstring.h"
#include "crbase/win/windows_version.h"

namespace crui {

namespace display {
namespace win {

namespace {

using Microsoft::WRL::ComPtr;
using Microsoft::WRL::Callback;
using ABI::Windows::Foundation::ITypedEventHandler;
using ABI::Windows::UI::ViewManagement::UISettings;
using ABI::Windows::UI::ViewManagement::IUISettings;
using ABI::Windows::UI::ViewManagement::IUISettings2;

typedef ITypedEventHandler<UISettings*, IInspectable*>
    TextScaleChangedEventHandler;

// A zero value indicates an invalid token.
constexpr EventRegistrationToken kInvalidEventRegistrationToken{0LL};

// Override the default instance for testing purposes.
UwpTextScaleFactor* g_implementation_for_testing = nullptr;

// Sanity check for access after destruction.
bool g_default_instance_cleaned_up = false;

// Constructs the UWP UI Settings COM object, or fails with as useful of a log
// message as possible given Windows error reporting.
//
// Lots of things could potentially go wrong so we want to be able to bail out
// when creating the UWP UI Settings object, so we've moved the initialization
// to a separate function.
bool CreateUiSettingsComObject(ComPtr<IUISettings2>& ptr) {
  CR_DCHECK(!ptr);

  // This is required setup before using ScopedHString.
  if (!(cr::win::ResolveCoreWinRTDelayload() &&
        cr::win::ScopedHString::ResolveCoreWinRTStringDelayload())) {
    CR_DLOG(Error) << "Failed loading functions from combase.dll";
    return false;
  }

  // Create the COM object.
  auto hstring = cr::win::ScopedHString::Create(
      RuntimeClass_Windows_UI_ViewManagement_UISettings);
  if (!hstring.is_valid()) {
    return false;
  }
  ComPtr<IInspectable> inspectable;
  HRESULT hr = cr::win::RoActivateInstance(hstring.get(), &inspectable);
  if (FAILED(hr)) {
    CR_VLOG(2) << "RoActivateInstance failed: "
               << cr::logging::SystemErrorCodeToString(hr);
    return false;
  }

  // Verify that it supports the correct interface.
  hr = inspectable.As(&ptr);
  if (FAILED(hr)) {
    CR_VLOG(2) << "As IUISettings2 failed: "
               << cr::logging::SystemErrorCodeToString(hr);
    return false;
  }

  return true;
}

// Implements the actual logic for getting screen metrics.
class UwpTextScaleFactorImpl : public UwpTextScaleFactor {
 public:
  UwpTextScaleFactorImpl()
      : text_scale_factor_changed_token_(kInvalidEventRegistrationToken) {
    // There's no point in doing this initialization if we're earlier than
    // Windows 10, since UWP is a Win10 feature.
    if (cr::win::GetVersion() < cr::win::Version::WIN10)
      return;

    // We want to bracket all use of our COM object with COM initialization
    // in order to be sure we don't leak COM listeners into the OS. This may
    // extend the lifetime of COM on this thread but we do not expect it to be
    // a problem.
    scoped_com_initializer_ =
        std::make_unique<cr::win::ScopedCOMInitializer>();
    if (!scoped_com_initializer_->succeeded())
      return;

    // Create our COM object. Again, if we fail, there's no reason to proceed.
    if (!CreateUiSettingsComObject(ui_settings_com_object_))
      return;

    // Set up a listener for the TextScaleChanged event. We'll do this here
    // so that we can use a member function as a callback.
    auto text_scale_changed_handler = Callback<TextScaleChangedEventHandler>(
        this, &UwpTextScaleFactorImpl::OnTextScaleFactorChanged);
    if (text_scale_changed_handler) {
      HRESULT hr = ui_settings_com_object_->add_TextScaleFactorChanged(
          text_scale_changed_handler.Get(), &text_scale_factor_changed_token_);
      if (FAILED(hr)) {
        CR_VLOG(2) << "Register text scale changed callback failed: "
                   << cr::logging::SystemErrorCodeToString(hr);
      }
    }
  }

  ~UwpTextScaleFactorImpl() override {
    CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

    // Release the callback if we've registered one and free our COM objects.
    if (ui_settings_com_object_) {
      if (text_scale_factor_changed_token_.value) {
        HRESULT hr = ui_settings_com_object_->remove_TextScaleFactorChanged(
            text_scale_factor_changed_token_);
        if (FAILED(hr)) {
          CR_VLOG(2) << "Failed to remove TextScaleFactorChanged listener: "
                     << cr::logging::SystemErrorCodeToString(hr);
        }
      }
      ui_settings_com_object_.Reset();
    }

    g_default_instance_cleaned_up = true;
  }

  float GetTextScaleFactor() const override {
    CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

    double result = 1.0;

    // This is just a null check, so if we don't have access to the text
    // scaling / service for any reason we'll just use 1x.
    if (ui_settings_com_object_) {
      HRESULT hr = ui_settings_com_object_->get_TextScaleFactor(&result);
      if (FAILED(hr)) {
        CR_VLOG(2) << "IUISettings2::TextScaleFactor failed: "
                   << cr::logging::SystemErrorCodeToString(hr);
        // COM calls overwrite their out-params, typically by zeroing them out.
        // Since we can't rely on this being a valid value on failure, we'll
        // reset it.
        result = 1.0;
      }
    }

    // Windows documents this property to always have a value greater than or
    // equal to 1. Let's make sure that's the case - if we don't, we could get
    // bizarre behavior and divide-by-zeros later on.
    CR_DCHECK(result >= 1.0);
    return static_cast<float>(result);
  }

 private:
  HRESULT OnTextScaleFactorChanged(IUISettings*, IInspectable*) {
    NotifyUwpTextScaleFactorChanged();
    return S_OK;
  }

  std::unique_ptr<cr::win::ScopedCOMInitializer> scoped_com_initializer_;
  ComPtr<IUISettings2> ui_settings_com_object_;
  EventRegistrationToken text_scale_factor_changed_token_;

  CR_THREAD_CHECKER(thread_checker_)
};

}  // namespace

//---------------------------------------------------------
// Base UwpScreenMetrics implementation:

UwpTextScaleFactor::UwpTextScaleFactor() = default;

UwpTextScaleFactor::~UwpTextScaleFactor() {
  for (auto& observer : observer_list_) {
    observer.OnUwpTextScaleFactorCleanup(this);
  }
}

float UwpTextScaleFactor::GetTextScaleFactor() const {
  return 1.0f;
}

void UwpTextScaleFactor::AddObserver(Observer* observer) {
  observer_list_.AddObserver(observer);
}

void UwpTextScaleFactor::RemoveObserver(Observer* observer) {
  observer_list_.RemoveObserver(observer);
}

void UwpTextScaleFactor::NotifyUwpTextScaleFactorChanged() {
  for (auto& observer : observer_list_) {
    observer.OnUwpTextScaleFactorChanged();
  }
}

void UwpTextScaleFactor::SetImplementationForTesting(
    UwpTextScaleFactor* mock_impl) {
  g_implementation_for_testing = mock_impl;
}

UwpTextScaleFactor* UwpTextScaleFactor::Instance() {
  static cr::LazyInstance<UwpTextScaleFactorImpl>::DestructorAtExit instance;

  CR_DCHECK(!g_default_instance_cleaned_up)
      << "Attempting to access UwpScreenMetrics after AtExit cleanup!";

  return g_implementation_for_testing ? g_implementation_for_testing
                                      : &instance.Get();
}

//---------------------------------------------------------
// UwpScreenMetrics::Observer implementation:

void UwpTextScaleFactor::Observer::OnUwpTextScaleFactorCleanup(
    UwpTextScaleFactor* source) {
  source->RemoveObserver(this);
}

}  // namespace win
}  // namespace display
}  // namespace crui
