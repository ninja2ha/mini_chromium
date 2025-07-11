/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#include <stdlib.h>

#define SK_DEBUGFAILF(fmt, ...) \
    SkASSERT((SkDebugf(fmt"\n", __VA_ARGS__), false))

static inline void sk_out_of_memory(size_t size) {
    SK_DEBUGFAILF("sk_out_of_memory (asked for " SK_SIZE_T_SPECIFIER " bytes)",
                  size);
    abort();
}

static inline void* throw_on_failure(size_t size, void* p) {
    if (size > 0 && p == nullptr) {
        // If we've got a nullptr here, the only reason we should have failed is running out of RAM.
        sk_out_of_memory(size);
    }
    return p;
}

void sk_throw() {
    SkDEBUGFAIL("sk_throw");
    abort();
}

void sk_out_of_memory(void) {
    SkDEBUGFAIL("sk_out_of_memory");
    abort();
}

void* sk_malloc_throw(size_t size) {
    return sk_malloc_flags(size, SK_MALLOC_THROW);
}

void* sk_realloc_throw(void* addr, size_t size) {
    return throw_on_failure(size, realloc(addr, size));
}

void sk_free(void* p) {
    if (p) {
        free(p);
    }
}

void* sk_malloc_flags(size_t size, unsigned flags) {
    void* p = malloc(size);
    if (flags & SK_MALLOC_THROW) {
        return throw_on_failure(size, p);
    } else {
        return p;
    }
}

void* sk_calloc(size_t size) {
    return calloc(size, 1);
}

void* sk_calloc_throw(size_t size) {
    return throw_on_failure(size, sk_calloc(size));
}
