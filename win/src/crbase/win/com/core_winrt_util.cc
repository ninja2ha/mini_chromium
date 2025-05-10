// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/win/com/core_winrt_util.h"

namespace {

typedef HRESULT (WINAPI* RoInitializePtr)(RO_INIT_TYPE);
typedef VOID (WINAPI* RoUninitializePtr)();
typedef HRESULT(WINAPI* RoActivateInstancePtr)(HSTRING, IInspectable**);
typedef HRESULT(WINAPI* RoGetActivationFactoryPtr)(HSTRING, REFIID, void**);

FARPROC LoadComBaseFunction(const char* function_name) {
  static HMODULE const handle =
      ::LoadLibraryExW(L"combase.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
  return handle ? ::GetProcAddress(handle, function_name) : nullptr;
}

RoInitializePtr GetRoInitializeFunction() {
  static RoInitializePtr const function =
      reinterpret_cast<RoInitializePtr>(
          LoadComBaseFunction("RoInitialize"));
  return function;
}

RoUninitializePtr GetRoUninitializeFunction() {
  static RoUninitializePtr const function =
      reinterpret_cast<RoUninitializePtr>(
          LoadComBaseFunction("RoUninitialize"));
  return function;
}

RoActivateInstancePtr GetRoActivateInstanceFunction() {
  static RoActivateInstancePtr const function =
      reinterpret_cast<RoActivateInstancePtr>(
          LoadComBaseFunction("RoActivateInstance"));
  return function;
}

RoGetActivationFactoryPtr GetRoGetActivationFactoryFunction() {
  static RoGetActivationFactoryPtr const function =
      reinterpret_cast<RoGetActivationFactoryPtr>(
          LoadComBaseFunction("RoGetActivationFactory"));
  return function;
}

}  // namespace

namespace cr {
namespace win {

bool ResolveCoreWinRTDelayload() {
  // TODO(finnur): Add AssertIOAllowed once crbug.com/770193 is fixed.
  return GetRoInitializeFunction() && GetRoUninitializeFunction() &&
         GetRoActivateInstanceFunction() && GetRoGetActivationFactoryFunction();
}

HRESULT RoInitialize(RO_INIT_TYPE init_type) {
  auto ro_initialize_func = GetRoInitializeFunction();
  if (!ro_initialize_func)
    return E_FAIL;
  return ro_initialize_func(init_type);
}

void RoUninitialize() {
  auto ro_uninitialize_func = GetRoUninitializeFunction();
  if (ro_uninitialize_func)
    ro_uninitialize_func();
}

HRESULT RoGetActivationFactory(HSTRING class_id,
                               const IID& iid,
                               void** out_factory) {
  auto get_factory_func = GetRoGetActivationFactoryFunction();
  if (!get_factory_func)
    return E_FAIL;
  return get_factory_func(class_id, iid, out_factory);
}

HRESULT RoActivateInstance(HSTRING class_id, IInspectable** instance) {
  auto activate_instance_func = GetRoActivateInstanceFunction();
  if (!activate_instance_func)
    return E_FAIL;
  return activate_instance_func(class_id, instance);
}

}  // namespace win
}  // namespace cr
