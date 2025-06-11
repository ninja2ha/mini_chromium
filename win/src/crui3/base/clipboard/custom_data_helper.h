// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_CLIPBOARD_CUSTOM_DATA_HELPER_H_
#define UI_BASE_CLIPBOARD_CUSTOM_DATA_HELPER_H_

#include <stddef.h>

#include <unordered_map>
#include <vector>

#include "crbase/containers/flat_map.h"
#include "crbase/strings/string16.h"
#include "crui/base/ui_export.h"
#include "crui/base/build_platform.h"

// Due to restrictions of most operating systems, we don't directly map each
// type of custom data to a native data transfer type. Instead, we serialize
// each key-value pair into the pickle as a pair of string objects, and then
// write the binary data in the pickle to the native data transfer object.
namespace cr {
class Pickle;
}  // namespace cr

namespace crui {

CRUI_EXPORT
void ReadCustomDataTypes(const void* data,
                         size_t data_length,
                         std::vector<cr::string16>* types);
CRUI_EXPORT
void ReadCustomDataForType(const void* data,
                           size_t data_length,
                           const cr::string16& type,
                           cr::string16* result);
CRUI_EXPORT
void ReadCustomDataIntoMap(
    const void* data,
    size_t data_length,
    std::unordered_map<cr::string16, cr::string16>* result);

CRUI_EXPORT
void WriteCustomDataToPickle(
    const std::unordered_map<cr::string16, cr::string16>& data,
    cr::Pickle* pickle);

CRUI_EXPORT
void WriteCustomDataToPickle(
    const cr::flat_map<cr::string16, cr::string16>& data,
    cr::Pickle* pickle);

}  // namespace crui

#endif  // UI_BASE_CLIPBOARD_CLIPBOARD_H_
