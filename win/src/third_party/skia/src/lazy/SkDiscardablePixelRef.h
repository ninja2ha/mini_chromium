/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDiscardablePixelRef_DEFINED
#define SkDiscardablePixelRef_DEFINED

#include "src/core/SkDiscardableMemory.h"
#include "src/core/SkImageGeneratorPriv.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixelRef.h"

/**
 *  A PixelRef backed by SkDiscardableMemory, with the ability to
 *  re-generate the pixels (via a SkImageGenerator) if the DM is
 *  purged.
 */
class SkDiscardablePixelRef : public SkPixelRef {
public:
    
    SkDiscardableMemory* diagnostic_only_getDiscardable() const override {
        return fDiscardableMemory;
    }

protected:
    ~SkDiscardablePixelRef();

    bool onNewLockPixels(LockRec*) override;
    void onUnlockPixels() override;
    bool onLockPixelsAreWritable() const override { return false; }

    SkData* onRefEncodedData() override {
        return fGenerator->refEncodedData();
    }

    bool onIsLazyGenerated() const override { return true; }

private:
    SkImageGenerator* const fGenerator;
    SkDiscardableMemory::Factory* const fDMFactory;
    const size_t fRowBytes;
    // These const members should not change over the life of the
    // PixelRef, since the SkBitmap doesn't expect them to change.

    SkDiscardableMemory* fDiscardableMemory;
    bool                 fDiscardableMemoryIsLocked;
    SkAutoTUnref<SkColorTable> fCTable;

    /* Takes ownership of SkImageGenerator. */
    SkDiscardablePixelRef(const SkImageInfo&, SkImageGenerator*,
                          size_t rowBytes,
                          SkDiscardableMemory::Factory* factory);

    bool onGetYUV8Planes(SkISize sizes[3],
                         void* planes[3],
                         size_t rowBytes[3],
                         SkYUVColorSpace* colorSpace) override {
        // If the image was already decoded with lockPixels(), favor not
        // re-decoding to YUV8 planes.
        if (fDiscardableMemory) {
            return false;
        }
        return fGenerator->getYUV8Planes(sizes, planes, rowBytes, colorSpace);
    }

    friend bool SkDEPRECATED_InstallDiscardablePixelRef(SkImageGenerator*, const SkIRect*, SkBitmap*,
                                             SkDiscardableMemory::Factory*);

    typedef SkPixelRef INHERITED;
};

#endif  // SkDiscardablePixelRef_DEFINED
