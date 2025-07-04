
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_WIN32)

#include "src/ports/SkOSLibrary.h"
#include <windows.h>

void* DynamicLoadLibrary(const char* libraryName) {
    return LoadLibraryA(libraryName);
}

void* GetProcedureAddress(void* library, const char* functionName) {
    return ::GetProcAddress((HMODULE)library, functionName);
}

#endif//defined(SK_BUILD_FOR_WIN32)
