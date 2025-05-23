// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_METADATA_METADATA_CACHE_H_
#define UI_VIEWS_METADATA_METADATA_CACHE_H_

#include <memory>
#include <utility>
#include <vector>

#include "crui/views/metadata/metadata_types.h"
#include "crui/base/ui_export.h"

namespace crui {

namespace views {
namespace metadata {

class ClassMetaData;
// The MetaDataCache speeds up frequent traversals over the meta data for
// various classes by caching one instance of ClassMetaData for each class on
// which metadata is required.
// MetaDataCache is implemented as a singleton. This also implies that each
// instance of ClassMetaData registered into the cache represents one and only
// one class type.
class CRUI_EXPORT MetaDataCache {
 public:
  MetaDataCache(const MetaDataCache&) = delete;
  MetaDataCache& operator=(const MetaDataCache&) = delete;

  MetaDataCache();

  static MetaDataCache* GetInstance();

  void AddClassMetaData(std::unique_ptr<ClassMetaData> class_data);
  std::vector<ClassMetaData*>& GetCachedTypes();

 private:
  ~MetaDataCache();

  std::vector<ClassMetaData*> class_data_cache_;
};

// These functions are rarely called directly, rather they are called from
// within the macros used to declare the metadata for a class. See the macros in
// reflections_macros.h
//
// Registers the class metadata into the global cache. Will DCHECK if the
// metadata for a class is already registered.
CRUI_EXPORT void RegisterClassInfo(std::unique_ptr<ClassMetaData> meta_data);

// Help function for creating and registering the metadata container into the
// global cache for a given class. The metadata information is owned by the
// given class.
template <typename TMetaData>
ClassMetaData* MakeAndRegisterClassInfo() {
  std::unique_ptr<TMetaData> class_meta_data = std::make_unique<TMetaData>();
  TMetaData* const ret = class_meta_data.get();
  RegisterClassInfo(std::move(class_meta_data));
  return ret;
}

}  // namespace metadata
}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_METADATA_METADATA_CACHE_H_
