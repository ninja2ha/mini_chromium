// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/path_service.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#endif

#include <unordered_map>

#include "crbase/logging.h"
#include "crbase/files/file_path.h"
#include "crbase/files/file_util.h"
#include "crbase/memory/lazy_instance.h"
#include "crbase/synchronization/lock.h"
#include "crbase/build_platform.h"

namespace cr {

bool PathProvider(int key, FilePath* result);

#if defined(MINI_CHROMIUM_OS_WIN)
bool PathProviderWin(int key, FilePath* result);
#elif defined(MINI_CHROMIUM_OS_POSIX)
// PathProviderPosix is the default path provider on POSIX OSes other than
// Mac and Android.
bool PathProviderPosix(int key, FilePath* result);
#endif

namespace {

typedef std::unordered_map<int, FilePath> PathMap;

// We keep a linked list of providers.  In a debug build we ensure that no two
// providers claim overlapping keys.
struct Provider {
  PathService::ProviderFunc func;
  struct Provider* next;
#ifndef NDEBUG
  int key_start;
  int key_end;
#endif
  bool is_static;
};

Provider base_provider = {
  PathProvider,
  NULL,
#ifndef NDEBUG
  PATH_START,
  PATH_END,
#endif
  true
};

#if defined(MINI_CHROMIUM_OS_WIN)
Provider base_provider_win = {
  PathProviderWin,
  &base_provider,
#ifndef NDEBUG
  PATH_WIN_START,
  PATH_WIN_END,
#endif
  true
};
#endif

#if defined(MINI_CHROMIUM_OS_POSIX)
Provider base_provider_posix = {
  PathProviderPosix,
  &base_provider,
#ifndef NDEBUG
  PATH_POSIX_START,
  PATH_POSIX_END,
#endif
  true
};
#endif

struct PathData {
  Lock lock;
  PathMap cache;        // Cache mappings from path key to path value.
  PathMap overrides;    // Track path overrides.
  Provider* providers;  // Linked list of path service providers.
  bool cache_disabled;  // Don't use cache if true;

  PathData() : cache_disabled(false) {
#if defined(MINI_CHROMIUM_OS_WIN)
    providers = &base_provider_win;
#elif defined(MINI_CHROMIUM_OS_POSIX)
    providers = &base_provider_posix;
#endif
  }
};

static LazyInstance<PathData>::Leaky g_path_data = LAZY_INSTANCE_INITIALIZER;

static PathData* GetPathData() {
  return g_path_data.Pointer();
}

// Tries to find |key| in the cache. |path_data| should be locked by the caller!
bool LockedGetFromCache(int key, const PathData* path_data, FilePath* result) {
  if (path_data->cache_disabled)
    return false;
  // check for a cached version
  PathMap::const_iterator it = path_data->cache.find(key);
  if (it != path_data->cache.end()) {
    *result = it->second;
    return true;
  }
  return false;
}

// Tries to find |key| in the overrides map. |path_data| should be locked by the
// caller!
bool LockedGetFromOverrides(int key, PathData* path_data, FilePath* result) {
  // check for an overridden version.
  PathMap::const_iterator it = path_data->overrides.find(key);
  if (it != path_data->overrides.end()) {
    if (!path_data->cache_disabled)
      path_data->cache[key] = it->second;
    *result = it->second;
    return true;
  }
  return false;
}

}  // namespace

// TODO(brettw): this function does not handle long paths (filename > MAX_PATH)
// characters). This isn't supported very well by Windows right now, so it is
// moot, but we should keep this in mind for the future.
// static
bool PathService::Get(int key, FilePath* result) {
  PathData* path_data = GetPathData();
  CR_DCHECK(path_data != NULL);
  CR_DCHECK(result != NULL);
  CR_DCHECK(key >= DIR_CURRENT);

  // special case the current directory because it can never be cached
  if (key == DIR_CURRENT)
    return GetCurrentDirectory(result);

  Provider* provider = NULL;
  {
    AutoLock scoped_lock(path_data->lock);
    if (LockedGetFromCache(key, path_data, result))
      return true;

    if (LockedGetFromOverrides(key, path_data, result))
      return true;

    // Get the beginning of the list while it is still locked.
    provider = path_data->providers;
  }

  FilePath path;

  // Iterating does not need the lock because only the list head might be
  // modified on another thread.
  while (provider) {
    if (provider->func(key, &path))
      break;
    CR_DCHECK(path.empty()) << "provider should not have modified path";
    provider = provider->next;
  }

  if (path.empty())
    return false;

  if (path.ReferencesParent()) {
    // Make sure path service never returns a path with ".." in it.
    path = MakeAbsoluteFilePath(path);
    if (path.empty())
      return false;
  }
  *result = path;

  AutoLock scoped_lock(path_data->lock);
  if (!path_data->cache_disabled)
    path_data->cache[key] = path;

  return true;
}

// static
bool PathService::Override(int key, const FilePath& path) {
  // Just call the full function with true for the value of |create|, and
  // assume that |path| may not be absolute yet.
  return OverrideAndCreateIfNeeded(key, path, false, true);
}

// static
bool PathService::OverrideAndCreateIfNeeded(int key,
                                            const FilePath& path,
                                            bool is_absolute,
                                            bool create) {
  PathData* path_data = GetPathData();
  CR_DCHECK(path_data);
  CR_DCHECK(key > DIR_CURRENT) << "invalid path key";

  FilePath file_path = path;

  // For some locations this will fail if called from inside the sandbox there-
  // fore we protect this call with a flag.
  if (create) {
    // Make sure the directory exists. We need to do this before we translate
    // this to the absolute path because on POSIX, MakeAbsoluteFilePath fails
    // if called on a non-existent path.
    if (!PathExists(file_path) && !CreateDirectory(file_path))
      return false;
  }

  // We need to have an absolute path.
  if (!is_absolute) {
    file_path = MakeAbsoluteFilePath(file_path);
    if (file_path.empty())
      return false;
  }
  CR_DCHECK(file_path.IsAbsolute());

  AutoLock scoped_lock(path_data->lock);

  // Clear the cache now. Some of its entries could have depended
  // on the value we are overriding, and are now out of sync with reality.
  path_data->cache.clear();

  path_data->overrides[key] = file_path;

  return true;
}

// static
bool PathService::RemoveOverride(int key) {
  PathData* path_data = GetPathData();
  CR_DCHECK(path_data);

  AutoLock scoped_lock(path_data->lock);

  if (path_data->overrides.find(key) == path_data->overrides.end())
    return false;

  // Clear the cache now. Some of its entries could have depended on the value
  // we are going to remove, and are now out of sync.
  path_data->cache.clear();

  path_data->overrides.erase(key);

  return true;
}

// static
void PathService::RegisterProvider(ProviderFunc func, int key_start,
                                   int key_end) {
  PathData* path_data = GetPathData();
  CR_DCHECK(path_data);
  CR_DCHECK(key_end > key_start);

  Provider* p;

  p = new Provider;
  p->is_static = false;
  p->func = func;
#ifndef NDEBUG
  p->key_start = key_start;
  p->key_end = key_end;
#endif

  AutoLock scoped_lock(path_data->lock);

#ifndef NDEBUG
  Provider *iter = path_data->providers;
  while (iter) {
    CR_DCHECK(key_start >= iter->key_end || key_end <= iter->key_start)
      << "path provider collision";
    iter = iter->next;
  }
#endif

  p->next = path_data->providers;
  path_data->providers = p;
}

// static
void PathService::DisableCache() {
  PathData* path_data = GetPathData();
  CR_DCHECK(path_data);

  AutoLock scoped_lock(path_data->lock);
  path_data->cache.clear();
  path_data->cache_disabled = true;
}

}  // namespace cr
