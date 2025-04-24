// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// =============================================================================
// PLEASE READ
//
// In general, you should not be adding stuff to this file.
//
// - If your thing is only used in one place, just put it in a reasonable
//   location in or near that one place. It's nice you want people to be able
//   to re-use your function, but realistically, if it hasn't been necessary
//   before after so many years of development, it's probably not going to be
//   used in other places in the future unless you know of them now.
//
// - If your thing is used by multiple callers and is UI-related, it should
//   probably be in app/win/ instead. Try to put it in the most specific file
//   possible (avoiding the *_util files when practical).
//
// =============================================================================

#ifndef MINI_CHROMIUM_SRC_CRBASE_WIN_WIN_UTIL_H_
#define MINI_CHROMIUM_SRC_CRBASE_WIN_WIN_UTIL_H_

#include <windows.h>
#include <stdint.h>

#include <string>

#include "crbase/base_export.h"
#include "crbase/strings/string16.h"

struct IPropertyStore;
struct _tagpropertykey;

namespace cr {
namespace win {

typedef _tagpropertykey PROPERTYKEY;

// This is the same as NONCLIENTMETRICS except that the
// unused member |iPaddedBorderWidth| has been removed.
struct NONCLIENTMETRICS_XP {
    UINT    cbSize;
    int     iBorderWidth;
    int     iScrollWidth;
    int     iScrollHeight;
    int     iCaptionWidth;
    int     iCaptionHeight;
    LOGFONTW lfCaptionFont;
    int     iSmCaptionWidth;
    int     iSmCaptionHeight;
    LOGFONTW lfSmCaptionFont;
    int     iMenuWidth;
    int     iMenuHeight;
    LOGFONTW lfMenuFont;
    LOGFONTW lfStatusFont;
    LOGFONTW lfMessageFont;
};

inline uint32_t HandleToUint32(HANDLE h) {
  // Cast through uintptr_t and then unsigned int to make the truncation to
  // 32 bits explicit. Handles are size of-pointer but are always 32-bit values.
  // https://msdn.microsoft.com/en-us/library/aa384203(VS.85).aspx says:
  // 64-bit versions of Windows use 32-bit handles for interoperability.
  return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(h));
}

CRBASE_EXPORT void GetNonClientMetrics(NONCLIENTMETRICS_XP* metrics);

// Returns the string representing the current user sid.
CRBASE_EXPORT bool GetUserSidString(std::wstring* user_sid);

// Returns false if user account control (UAC) has been disabled with the
// EnableLUA registry flag. Returns true if user account control is enabled.
// NOTE: The EnableLUA registry flag, which is ignored on Windows XP
// machines, might still exist and be set to 0 (UAC disabled), in which case
// this function will return false. You should therefore check this flag only
// if the OS is Vista or later.
CRBASE_EXPORT bool UserAccountControlIsEnabled();

// Sets the boolean value for a given key in given IPropertyStore.
CRBASE_EXPORT bool SetBooleanValueForPropertyStore(
    IPropertyStore* property_store,
    const PROPERTYKEY& property_key,
    bool property_bool_value);

// Sets the string value for a given key in given IPropertyStore.
CRBASE_EXPORT bool SetStringValueForPropertyStore(
    IPropertyStore* property_store,
    const PROPERTYKEY& property_key,
    const wchar_t* property_string_value);

// Sets the application id in given IPropertyStore. The function is intended
// for tagging application/chromium shortcut, browser window and jump list for
// Win7.
CRBASE_EXPORT bool SetAppIdForPropertyStore(IPropertyStore* property_store,
                                            const wchar_t* app_id);

// Adds the specified |command| using the specified |name| to the AutoRun key.
// |root_key| could be HKCU or HKLM or the root of any user hive.
CRBASE_EXPORT bool AddCommandToAutoRun(HKEY root_key, 
                                       const cr::string16& name,
                                       const cr::string16& command);
// Removes the command specified by |name| from the AutoRun key. |root_key|
// could be HKCU or HKLM or the root of any user hive.
CRBASE_EXPORT bool RemoveCommandFromAutoRun(HKEY root_key, 
                                            const cr::string16& name);

// Reads the command specified by |name| from the AutoRun key. |root_key|
// could be HKCU or HKLM or the root of any user hive. Used for unit-tests.
CRBASE_EXPORT bool ReadCommandFromAutoRun(HKEY root_key,
                                          const cr::string16& name,
                                          cr::string16* command);

// Sets whether to crash the process during exit. This is inspected by DLLMain
// and used to intercept unexpected terminations of the process (via calls to
// exit(), abort(), _exit(), ExitProcess()) and convert them into crashes.
// Note that not all mechanisms for terminating the process are covered by
// this. In particular, TerminateProcess() is not caught.
CRBASE_EXPORT void SetShouldCrashOnProcessDetach(bool crash);
CRBASE_EXPORT bool ShouldCrashOnProcessDetach();

// Adjusts the abort behavior so that crash reports can be generated when the
// process is aborted.
CRBASE_EXPORT void SetAbortBehaviorForCrashReporting();

// A tablet is a device that is touch enabled and also is being used
// "like a tablet". This is used by the following:-
// 1. Metrics:- To gain insight into how users use Chrome.
// 2. Physical keyboard presence :- If a device is in tablet mode, it means
//    that there is no physical keyboard attached.
// This function optionally sets the |reason| parameter to determine as to why
// or why not a device was deemed to be a tablet.
// Returns true if the device is in tablet mode.
CRBASE_EXPORT bool IsTabletDevice(std::string* reason);

// A slate is a touch device that may have a keyboard attached. This function
// returns true if a keyboard is attached and optionally will set the reason
// parameter to the detection method that was used to detect the keyboard.
CRBASE_EXPORT bool IsKeyboardPresentOnSlate(std::string* reason);

// Get the size of a struct up to and including the specified member.
// This is necessary to set compatible struct sizes for different versions
// of certain Windows APIs (e.g. SystemParametersInfo).
#define SIZEOF_STRUCT_WITH_SPECIFIED_LAST_MEMBER(struct_name, member) \
    offsetof(struct_name, member) + \
    (sizeof static_cast<struct_name*>(NULL)->member)

// Returns true if the current process can make USER32 or GDI32 calls such as
// CreateWindow and CreateDC. Windows 8 and above allow the kernel component
// of these calls to be disabled (also known as win32k lockdown) which can
// cause undefined behaviour such as crashes. This function can be used to
// guard areas of code using these calls and provide a fallback path if
// necessary.
// Because they are not always needed (and not needed at all in processes that
// have the win32k lockdown), USER32 and GDI32 are delayloaded. Attempts to
// load them in those processes will cause a crash. Any code which uses USER32
// or GDI32 and may run in a locked-down process MUST be guarded using this
// method. Before the dlls were delayloaded, method calls into USER32 and GDI32
// did not work, so adding calls to this method to guard them simply avoids
// unnecessary method calls.
CRBASE_EXPORT bool IsUser32AndGdi32Available();

// Used by tests to mock any wanted state. Call with |state| set to true to
// simulate being in a domain and false otherwise.
CRBASE_EXPORT void SetDomainStateForTesting(bool state);

// Returns true if the current operating system has support for SHA-256
// certificates. As its name indicates, this function provides a best-effort
// answer, which is solely based on comparing version numbers. The function
// may be re-implemented in the future to return a reliable value, based on
// run-time detection of this capability.
CRBASE_EXPORT bool MaybeHasSHA256Support();

}  // namespace win
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_WIN_WIN_UTIL_H_