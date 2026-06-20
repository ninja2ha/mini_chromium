#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#pragma comment(lib, "winmm")

#include "cr_base/logging/logging.h"
#include "cr_base/memory/no_destructor.h"

#include "cr_winrt/hook.h"

namespace {

void InitializeLogging() {
  cr::logging::LoggingConfig& config = CR_DEFAULT_LOGGING_CONFIG;
  config.prefix = "ca";
  config.enable_timedate = false;
  config.logging_dest = cr::logging::LOG_TO_SYSTEM_DEBUG_LOG;
  config.verbose_lowest_level = 999;
  cr::logging::InitializeConfig(config);
}

//
void UninitializeLogging() {
  cr::logging::InitializeConfig(CR_DEFAULT_LOGGING_CONFIG);
}

}  // namespace

// -----------------------------------------------------------------------------

namespace ca {

namespace cn {
extern void InitializeNetMessageHook();
}  // namespace cn

namespace tw {
extern void InitializeNetMessageHook();
}  // namespace tw

void InitializeNetMessageHook() {
  //ca::cn::InitializeNetMessageHook();
  ca::tw::InitializeNetMessageHook();
}

}  // namespace ca

// -- D3D9 ---------------------------------------------------------------------

HMODULE GetSystemD3D9Module() {
  static cr::NoDestructor<HMODULE> module([]() -> HMODULE {
    wchar_t path[MAX_PATH] = {0};
    ::GetSystemDirectoryW(path, MAX_PATH);
    if (path[0] == 0)
      return nullptr;

    ::lstrcatW(path, L"\\d3d9.dll");
    return ::LoadLibraryW(path);
  }());
  return *module;
}

LPVOID WINAPI Direct3DCreate9(UINT SDKVersion) {
  ca::InitializeNetMessageHook();

  FARPROC proc = ::GetProcAddress(GetSystemD3D9Module(), "Direct3DCreate9");
  if (proc == nullptr)
    return nullptr;

  return decltype(&Direct3DCreate9)(proc)(SDKVersion);
}

HRESULT WINAPI Direct3DCreate9Ex(UINT SDKVersion, LPVOID* unnamedParam2) {
  ca::InitializeNetMessageHook();

  FARPROC proc = ::GetProcAddress(GetSystemD3D9Module(), "Direct3DCreate9Ex");
  if (proc == nullptr)
    return /* D3DERR_NOTAVAILABLE*/ 0x8876086A;

  return decltype(&Direct3DCreate9Ex)(proc)(SDKVersion, unnamedParam2);
}

// -- Main ---------------------------------------------------------------------

BOOL APIENTRY DllMain(HMODULE module, DWORD reason_for_call, LPVOID) {
  static cr::AtExitManager* g_at_exit_manager = nullptr;

  if (reason_for_call == DLL_PROCESS_ATTACH) {
    InitializeLogging();
    
    if (!g_at_exit_manager)
      g_at_exit_manager = new cr::AtExitManager;
  }
  else if (reason_for_call == DLL_PROCESS_DETACH) {
    if (g_at_exit_manager)
      delete g_at_exit_manager;
    g_at_exit_manager = nullptr;

    UninitializeLogging();
  }
  return TRUE;
}