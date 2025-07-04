/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTDict_DEFINED
#define SkTDict_DEFINED

#include "include/core/SkChunkAlloc.h"
#include "include/private/SkTSearch.h"
#include "include/core/SkTDArray.h"

template <typename T> class SkTDict : SkNoncopyable {
public:
    SkTDict(size_t minStringAlloc) : fStrings(minStringAlloc) {}

    void reset() {
        fArray.reset();
        fStrings.reset();
    }

    int count() const { return fArray.count(); }

    bool set(const char name[], const T& value) {
        return set(name, strlen(name), value);
    }

    bool set(const char name[], size_t len, const T& value) {
        SkASSERT(name);

        int index = this->find_index(name, len);

        if (index >= 0) {
            fArray[index].fValue = value;
            return false;
        } else {
            Pair*   pair = fArray.insert(~index);
            char*   copy = (char*)fStrings.alloc(len + 1, SkChunkAlloc::kThrow_AllocFailType);
            memcpy(copy, name, len);
            copy[len] = '\0';
            pair->fName = copy;
            pair->fValue = value;
            return true;
        }
    }

    bool find(const char name[]) const {
        return this->find_index(name) >= 0;
    }

    bool find(const char name[], size_t len) const {
        return this->find_index(name, len) >= 0;
    }

    bool find(const char name[], T* value) const {
        return find(name, strlen(name), value);
    }

    bool find(const char name[], size_t len, T* value) const {
        int index = this->find_index(name, len);

        if (index >= 0) {
            if (value) {
                *value = fArray[index].fValue;
            }
            return true;
        }
        return false;
    }

    bool findKey(T& value, const char** name) const {
        const Pair* end = fArray.end();
        for (const Pair* pair = fArray.begin(); pair < end; pair++) {
            if (pair->fValue != value) {
                continue;
            }
            *name = pair->fName;
            return true;
        }
        return false;
    }

public:
    struct Pair {
        const char* fName;
        T           fValue;

        friend int operator<(const Pair& a, const Pair& b) {
            return strcmp(a.fName, b.fName);
        }

        friend int operator!=(const Pair& a, const Pair& b) {
            return strcmp(a.fName, b.fName);
        }
    };
    friend class Iter;

public:
    class Iter {
    public:
        Iter(const SkTDict<T>& dict) {
            fIter = dict.fArray.begin();
            fStop = dict.fArray.end();
        }

        const char* next(T* value) {
            const char* name = NULL;
            if (fIter < fStop) {
                name = fIter->fName;
                if (value) {
                    *value = fIter->fValue;
                }
                fIter += 1;
            }
            return name;
        }
    private:
        const Pair*   fIter;
        const Pair*   fStop;
    };

private:
    SkTDArray<Pair> fArray;
    SkChunkAlloc    fStrings;

    int find_index(const char name[]) const {
        return find_index(name, strlen(name));
    }

    int find_index(const char name[], size_t len) const {
        SkASSERT(name);

        int count = fArray.count();
        int index = ~0;

        if (count) {
            index = SkStrSearch(&fArray.begin()->fName, count, name, len, sizeof(Pair));
        }
        return index;
    }
    friend class Iter;
};

#endif
