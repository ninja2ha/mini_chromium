
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkRefDict_DEFINED
#define SkRefDict_DEFINED

#include "include/core/SkRefCnt.h"

/**
 *  A dictionary of string,refcnt pairs. The dictionary is also an owner of the
 *  refcnt objects while they are contained.
 */
class SK_API SkRefDict : SkNoncopyable {
public:
    SkRefDict();
    ~SkRefDict();

    /**
     *  Return the data associated with name[], or nullptr if no matching entry
     *  is found. The reference-count of the entry is not affected.
     */
    SkRefCnt* find(const char name[]) const;

    /**
     *  If data is nullptr, remove (if present) the entry matching name and call
     *  prev_data->unref() on the data for the matching entry.
     *  If data is not-nullptr, replace the existing entry matching name and
     *  call (prev_data->unref()), or add a new one. In either case,
     *  data->ref() is called.
     */
    void set(const char name[], SkRefCnt* data);

    /**
     *  Remove the matching entry (if found) and unref its data.
     */
    void remove(const char name[]) { this->set(name, nullptr); }

    /**
     *  Remove all entries, and unref() their associated data.
     */
    void removeAll();

private:
    struct Impl;
    Impl* fImpl;
};

#endif
