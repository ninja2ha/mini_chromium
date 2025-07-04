/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFlattenableSerialization_DEFINED
#define SkFlattenableSerialization_DEFINED

#include "include/core/SkFlattenable.h"

class SkData;

SK_API SkData* SkValidatingSerializeFlattenable(SkFlattenable*);
SK_API SkFlattenable* SkValidatingDeserializeFlattenable(const void* data, size_t size,
                                                         SkFlattenable::Type type);

#endif
