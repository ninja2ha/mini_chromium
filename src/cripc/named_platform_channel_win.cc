// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cripc/named_platform_channel.h"

#include <memory>

#include "crbase/logging/logging.h"
#include "crbase/rand_util.h"
#include "crbase/strings/stringprintf.h"
#include "crbase/strings/utf_string_conversions.h"
#include "crbase/win/windows_types.h"
#include "crbase/win/scoped_handle.h"
#include "crbase/win/windows_version.h"

// NOTE: This needs to be included *after* windows.h.
#include <sddl.h>

namespace mojo {

namespace {

// A DACL to grant:
// GA = Generic All
// access to:
// SY = LOCAL_SYSTEM
// BA = BUILTIN_ADMINISTRATORS
// OW = OWNER_RIGHTS
constexpr wchar_t kDefaultSecurityDescriptor[] =
    L"D:(A;;GA;;;SY)(A;;GA;;;BA)(A;;GA;;;OW)";

NamedPlatformChannel::ServerName GenerateRandomServerName() {
  return cr::StringPrintf(L"%lu.%lu.%I64u", ::GetCurrentProcessId(),
                          ::GetCurrentThreadId(), cr::RandUint64());
}

std::wstring GetPipeNameFromServerName(
    const NamedPlatformChannel::ServerName& server_name) {
  return L"\\\\.\\pipe\\mojo." + server_name;
}

}  // namespace

// static
PlatformChannelServerEndpoint NamedPlatformChannel::CreateServerEndpoint(
    const Options& options,
    ServerName* server_name) {
  ServerName name = options.server_name;
  if (name.empty())
    name = GenerateRandomServerName();

  PSECURITY_DESCRIPTOR security_desc = nullptr;
  ULONG security_desc_len = 0;
  CR_PCHECK(::ConvertStringSecurityDescriptorToSecurityDescriptor(
      options.security_descriptor.empty() ? kDefaultSecurityDescriptor
                                          : options.security_descriptor.c_str(),
      SDDL_REVISION_1, &security_desc, &security_desc_len));
  std::unique_ptr<void, decltype(::LocalFree)*> p(security_desc, ::LocalFree);
  SECURITY_ATTRIBUTES security_attributes = {sizeof(SECURITY_ATTRIBUTES),
                                             security_desc, FALSE};

  const DWORD kOpenMode = options.enforce_uniqueness
                              ? PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED |
                                    FILE_FLAG_FIRST_PIPE_INSTANCE
                              : PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED;
  const DWORD kPipeMode =
      PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_REJECT_REMOTE_CLIENTS;

  std::wstring pipe_name = GetPipeNameFromServerName(name);
  PlatformHandle handle(cr::win::ScopedHandle(::CreateNamedPipeW(
      pipe_name.c_str(), kOpenMode, kPipeMode,
      options.enforce_uniqueness ? 1 : 255,  // Max instances.
      4096,                                  // Out buffer size.
      4096,                                  // In buffer size.
      5000,                                  // Timeout in milliseconds.
      &security_attributes)));

  if (!handle.is_valid())
    CR_PLOG(Warning) << "CreateNamedPipeW failed.";

  *server_name = name;
  return PlatformChannelServerEndpoint(std::move(handle));
}

// static
PlatformChannelEndpoint NamedPlatformChannel::CreateClientEndpoint(
    const ServerName& server_name) {
  std::wstring pipe_name = GetPipeNameFromServerName(server_name);

  // Note: This may block.
  if (!::WaitNamedPipeW(pipe_name.c_str(), NMPWAIT_USE_DEFAULT_WAIT))
    return PlatformChannelEndpoint();

  const DWORD kDesiredAccess = GENERIC_READ | GENERIC_WRITE;
  // The SECURITY_ANONYMOUS flag means that the server side cannot impersonate
  // the client.
  const DWORD kFlags =
      SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS | FILE_FLAG_OVERLAPPED;
  PlatformHandle handle(cr::win::ScopedHandle(
      ::CreateFileW(pipe_name.c_str(), kDesiredAccess, 0, nullptr,
                    OPEN_EXISTING, kFlags, nullptr)));

  // The server may have stopped accepting a connection between the
  // WaitNamedPipe() and CreateFile(). If this occurs, an invalid handle is
  // returned.
  CR_DPLOG_IF(Error, !handle.is_valid())
      << "Named pipe " << cr::WideToUTF8(pipe_name)
      << " could not be opened after WaitNamedPipe succeeded";
  return PlatformChannelEndpoint(std::move(handle));
}

}  // namespace mojo