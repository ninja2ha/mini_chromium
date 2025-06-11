// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/base/win/shell.h"

#include <dwmapi.h>
#include <shlobj.h>  // Must be before propkey.

#include <propkey.h>
#include <shellapi.h>
#include <wrl/client.h>

///#include "crbase/command_line.h"
#include "crbase/debug/alias.h"
#include "crbase/files/file.h"
#include "crbase/files/file_path.h"
#include "crbase/native_library.h"
#include "crbase/strings/string_util.h"
#include "crbase/strings/stringprintf.h"
#include "crbase/threading/thread_restrictions.h"
///#include "crbase/threading/scoped_thread_priority.h"
#include "crbase/win/win_util.h"
#include "crbase/win/windows_version.h"
///#include "crui/base/ui_base_switches.h"

namespace crui {
namespace win {

namespace {

// Default ShellExecuteEx flags used with "openas", "explore", and default
// verbs.
//
// SEE_MASK_NOASYNC is specified so that ShellExecuteEx can be invoked from a
// thread whose message loop may not wait around long enough for the
// asynchronous tasks initiated by ShellExecuteEx to complete. Using this flag
// causes ShellExecuteEx() to block until these tasks complete.
const DWORD kDefaultShellExecuteFlags = SEE_MASK_NOASYNC;

// Invokes ShellExecuteExW() with the given parameters.
bool InvokeShellExecute(const cr::string16 path,
                        const cr::string16 working_directory,
                        const cr::string16 args,
                        const cr::string16 verb,
                        DWORD mask) {
  cr::ThreadRestrictions::AssertWaitAllowed();

  SHELLEXECUTEINFO sei = {sizeof(sei)};
  sei.fMask = mask;
  sei.nShow = SW_SHOWNORMAL;
  sei.lpVerb = (verb.empty() ? nullptr : verb.c_str());
  sei.lpFile = path.c_str();
  sei.lpDirectory =
      (working_directory.empty() ? nullptr : working_directory.c_str());
  sei.lpParameters = (args.empty() ? nullptr : args.c_str());

  // Mitigate the issues caused by loading DLLs on a background thread
  // (http://crbug/973868).
  ///SCOPED_MAY_LOAD_LIBRARY_AT_BACKGROUND_PRIORITY();

  return !!::ShellExecuteExW(&sei);
}

}  // namespace

bool OpenFileViaShell(const cr::FilePath& full_path) {
  // Invoke the default verb on the file with no arguments.
  return InvokeShellExecute(full_path.value(), full_path.DirName().value(),
                            cr::string16(), cr::string16(),
                            kDefaultShellExecuteFlags);
}

bool OpenFolderViaShell(const cr::FilePath& full_path) {
  // The "explore" verb causes the folder at |full_path| to be displayed in a
  // file browser. This will fail if |full_path| is not a directory.
  return InvokeShellExecute(full_path.value(), full_path.value(),
                            cr::string16(), L"explore",
                            kDefaultShellExecuteFlags);
}

bool PreventWindowFromPinning(HWND hwnd) {
  CR_DCHECK(hwnd);

  Microsoft::WRL::ComPtr<IPropertyStore> pps;
  if (FAILED(
          SHGetPropertyStoreForWindow(hwnd, IID_PPV_ARGS(pps.GetAddressOf()))))
    return false;

  return cr::win::SetBooleanValueForPropertyStore(
      pps.Get(), PKEY_AppUserModel_PreventPinning, true);
}

// TODO(calamity): investigate moving this out of the UI thread as COM
// operations may spawn nested run loops which can cause issues.
void SetAppDetailsForWindow(const cr::string16& app_id,
                            const cr::FilePath& app_icon_path,
                            int app_icon_index,
                            const cr::string16& relaunch_command,
                            const cr::string16& relaunch_display_name,
                            HWND hwnd) {
  CR_DCHECK(hwnd);

  Microsoft::WRL::ComPtr<IPropertyStore> pps;
  if (FAILED(
          SHGetPropertyStoreForWindow(hwnd, IID_PPV_ARGS(pps.GetAddressOf()))))
    return;

  if (!app_id.empty())
    cr::win::SetAppIdForPropertyStore(pps.Get(), app_id.c_str());
  if (!app_icon_path.empty()) {
    // Always add the icon index explicitly to prevent bad interaction with the
    // index notation when file path has commas.
    cr::win::SetStringValueForPropertyStore(
        pps.Get(), PKEY_AppUserModel_RelaunchIconResource,
        cr::StringPrintf(L"%ls,%d", app_icon_path.value().c_str(),
                         app_icon_index)
            .c_str());
  }
  if (!relaunch_command.empty()) {
    cr::win::SetStringValueForPropertyStore(
        pps.Get(), PKEY_AppUserModel_RelaunchCommand,
        relaunch_command.c_str());
  }
  if (!relaunch_display_name.empty()) {
    cr::win::SetStringValueForPropertyStore(
        pps.Get(), PKEY_AppUserModel_RelaunchDisplayNameResource,
        relaunch_display_name.c_str());
  }
}

void SetAppIdForWindow(const cr::string16& app_id, HWND hwnd) {
  SetAppDetailsForWindow(app_id, cr::FilePath(), 0, cr::string16(),
                         cr::string16(), hwnd);
}

void SetAppIconForWindow(const cr::FilePath& app_icon_path,
                         int app_icon_index,
                         HWND hwnd) {
  SetAppDetailsForWindow(cr::string16(), app_icon_path, app_icon_index,
                         cr::string16(), cr::string16(), hwnd);
}

void SetRelaunchDetailsForWindow(const cr::string16& relaunch_command,
                                 const cr::string16& display_name,
                                 HWND hwnd) {
  SetAppDetailsForWindow(cr::string16(), cr::FilePath(), 0,
                         relaunch_command, display_name, hwnd);
}

void ClearWindowPropertyStore(HWND hwnd) {
  CR_DCHECK(hwnd);

  Microsoft::WRL::ComPtr<IPropertyStore> pps;
  if (FAILED(
          SHGetPropertyStoreForWindow(hwnd, IID_PPV_ARGS(pps.GetAddressOf()))))
    return;

  DWORD property_count;
  if (FAILED(pps->GetCount(&property_count)))
    return;

  PROPVARIANT empty_property_variant = {};
  for (DWORD i = property_count; i > 0; i--) {
    PROPERTYKEY key;
    if (SUCCEEDED(pps->GetAt(i - 1, &key))) {
      // Removes the value from |pps|'s array.
      pps->SetValue(key, empty_property_variant);
    }
  }
  if (FAILED(pps->Commit()))
    return;

  // Verify none of the keys are leaking.
  CR_DCHECK(FAILED(pps->GetCount(&property_count)) || property_count == 0);
}

bool IsAeroGlassEnabled() {
  // For testing in Win8 (where it is not possible to disable composition) the
  // user can specify this command line switch to mimic the behavior.  In this
  // mode, cross-HWND transparency is not supported and various types of
  // widgets fallback to more simplified rendering behavior.
  ///if (base::CommandLine::ForCurrentProcess()->HasSwitch(
  ///        switches::kDisableDwmComposition))
  ///  return false;

  // If composition is not enabled, we behave like on XP.
  return IsDwmCompositionEnabled();
}

bool IsDwmCompositionEnabled() {
  // As of Windows 8, DWM composition is always enabled.
  // In Windows 7 this can change at runtime.
  if (cr::win::GetVersion() >= cr::win::Version::WIN8) {
    return true;
  }
  BOOL is_enabled;
  return SUCCEEDED(DwmIsCompositionEnabled(&is_enabled)) && is_enabled;
}

}  // namespace win
}  // namespace crui
