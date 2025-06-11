// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/base/clipboard/clipboard.h"

#include <iterator>
#include <limits>
#include <memory>

#include "crbase/logging.h"
#include "crbase/memory/ptr_util.h"
#include "crbase/helper/stl_util.h"
///#include "third_party/skia/include/core/SkBitmap.h"
#include "crui/gfx/geometry/size.h"

namespace crui {

// static
void Clipboard::SetAllowedThreads(
    const std::vector<cr::PlatformThreadId>& allowed_threads) {
  cr::AutoLock lock(ClipboardMapLock());

  AllowedThreads().clear();
  std::copy(allowed_threads.begin(), allowed_threads.end(),
            std::back_inserter(AllowedThreads()));
}

// static
void Clipboard::SetClipboardForCurrentThread(
    std::unique_ptr<Clipboard> platform_clipboard) {
  cr::AutoLock lock(ClipboardMapLock());
  cr::PlatformThreadId id = Clipboard::GetAndValidateThreadID();

  ClipboardMap* clipboard_map = ClipboardMapPtr();
  // This shouldn't happen. The clipboard should not already exist.
  CR_DCHECK(!cr::Contains(*clipboard_map, id));
  clipboard_map->insert({id, std::move(platform_clipboard)});
}

// static
Clipboard* Clipboard::GetForCurrentThread() {
  cr::AutoLock lock(ClipboardMapLock());
  cr::PlatformThreadId id = GetAndValidateThreadID();

  ClipboardMap* clipboard_map = ClipboardMapPtr();
  auto it = clipboard_map->find(id);
  if (it != clipboard_map->end())
    return it->second.get();

  Clipboard* clipboard = Clipboard::Create();
  clipboard_map->insert({id, cr::WrapUnique(clipboard)});
  return clipboard;
}

// static
std::unique_ptr<Clipboard> Clipboard::TakeForCurrentThread() {
  cr::AutoLock lock(ClipboardMapLock());

  ClipboardMap* clipboard_map = ClipboardMapPtr();
  cr::PlatformThreadId id = cr::PlatformThread::CurrentId();

  Clipboard* clipboard = nullptr;

  auto it = clipboard_map->find(id);
  if (it != clipboard_map->end()) {
    clipboard = it->second.release();
    clipboard_map->erase(it);
  }

  return cr::WrapUnique(clipboard);
}

// static
void Clipboard::OnPreShutdownForCurrentThread() {
  cr::AutoLock lock(ClipboardMapLock());
  cr::PlatformThreadId id = GetAndValidateThreadID();

  ClipboardMap* clipboard_map = ClipboardMapPtr();
  auto it = clipboard_map->find(id);
  if (it != clipboard_map->end())
    it->second->OnPreShutdown();
}

// static
void Clipboard::DestroyClipboardForCurrentThread() {
  cr::AutoLock lock(ClipboardMapLock());

  ClipboardMap* clipboard_map = ClipboardMapPtr();
  cr::PlatformThreadId id = cr::PlatformThread::CurrentId();
  auto it = clipboard_map->find(id);
  if (it != clipboard_map->end())
    clipboard_map->erase(it);
}

cr::Time Clipboard::GetLastModifiedTime() const {
  return cr::Time();
}

void Clipboard::ClearLastModifiedTime() {}

Clipboard::Clipboard() = default;
Clipboard::~Clipboard() = default;

void Clipboard::DispatchPortableRepresentation(PortableFormat format,
                                               const ObjectMapParams& params) {
  // Ignore writes with empty parameters.
  for (const auto& param : params) {
    if (param.empty())
      return;
  }

  switch (format) {
    case PortableFormat::kText:
      WriteText(&(params[0].front()), params[0].size());
      break;

    case PortableFormat::kHtml:
      if (params.size() == 2) {
        if (params[1].empty())
          return;
        WriteHTML(&(params[0].front()), params[0].size(),
                  &(params[1].front()), params[1].size());
      } else if (params.size() == 1) {
        WriteHTML(&(params[0].front()), params[0].size(), nullptr, 0);
      }
      break;

    case PortableFormat::kRtf:
      WriteRTF(&(params[0].front()), params[0].size());
      break;

    case PortableFormat::kBookmark:
      WriteBookmark(&(params[0].front()), params[0].size(),
                    &(params[1].front()), params[1].size());
      break;

    case PortableFormat::kWebkit:
      WriteWebSmartPaste();
      break;

    case PortableFormat::kBitmap: {
      // Usually, the params are just UTF-8 strings. However, for images,
      // ScopedClipboardWriter actually sizes the buffer to sizeof(SkBitmap*),
      // aliases the contents of the vector to a SkBitmap**, and writes the
      // pointer to the actual SkBitmap in the clipboard object param.
      const char* packed_pointer_buffer = &params[0].front();
      ///WriteBitmap(**reinterpret_cast<SkBitmap* const*>(packed_pointer_buffer));
      break;
    }

    case PortableFormat::kData:
      WriteData(ClipboardFormatType::Deserialize(
                    std::string(&(params[0].front()), params[0].size())),
                &(params[1].front()), params[1].size());
      break;

    default:
      CR_NOTREACHED();
  }
}

void Clipboard::DispatchPlatformRepresentations(
    std::vector<Clipboard::PlatformRepresentation> platform_representations) {
  ///for (const auto& representation : platform_representations) {
  ///  WriteData(ClipboardFormatType::GetType(representation.format),
  ///            reinterpret_cast<const char*>(representation.data.data()),
  ///            representation.data.size());
  ///}
}

cr::PlatformThreadId Clipboard::GetAndValidateThreadID() {
  ClipboardMapLock().AssertAcquired();

  const cr::PlatformThreadId id = cr::PlatformThread::CurrentId();

  // A Clipboard instance must be allocated for every thread that uses the
  // clipboard. To prevented unbounded memory use, CHECK that the current thread
  // was whitelisted to use the clipboard. This is a CHECK rather than a DCHECK
  // to catch incorrect usage in production (e.g. https://crbug.com/872737).
  CR_CHECK(AllowedThreads().empty() || cr::Contains(AllowedThreads(), id));

  return id;
}

// static
std::vector<cr::PlatformThreadId>& Clipboard::AllowedThreads() {
  static cr::NoDestructor<std::vector<cr::PlatformThreadId>>
      allowed_threads;
  return *allowed_threads;
}

// static
Clipboard::ClipboardMap* Clipboard::ClipboardMapPtr() {
  static cr::NoDestructor<ClipboardMap> clipboard_map;
  return clipboard_map.get();
}

// static
cr::Lock& Clipboard::ClipboardMapLock() {
  static cr::NoDestructor<cr::Lock> clipboard_map_lock;
  return *clipboard_map_lock;
}

}  // namespace crui
