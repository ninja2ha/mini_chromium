/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontConfigInterface_DEFINED
#define SkFontConfigInterface_DEFINED

#include "include/core/SkDataTable.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTArray.h"
#include "include/core/SkTypeface.h"

struct SkBaseMutex;

/**
 *  \class SkFontConfigInterface
 *
 *  A simple interface for remotable font management.
 *  The global instance can be found with RefGlobal().
 */
class SK_API SkFontConfigInterface : public SkRefCnt {
public:
    

    /**
     *  Returns the global SkFontConfigInterface instance, and if it is not
     *  NULL, calls ref() on it. The caller must balance this with a call to
     *  unref().
     */
    static SkFontConfigInterface* RefGlobal();

    /**
     *  Replace the current global instance with the specified one, safely
     *  ref'ing the new instance, and unref'ing the previous. Returns its
     *  parameter (the new global instance).
     */
    static SkFontConfigInterface* SetGlobal(SkFontConfigInterface*);

    /**
     *  This should be treated as private to the impl of SkFontConfigInterface.
     *  Callers should not change or expect any particular values. It is meant
     *  to be a union of possible storage types to aid the impl.
     */
    struct FontIdentity {
        FontIdentity() : fID(0), fTTCIndex(0) {}

        bool operator==(const FontIdentity& other) const {
            return fID == other.fID &&
                   fTTCIndex == other.fTTCIndex &&
                   fString == other.fString;
        }
        bool operator!=(const FontIdentity& other) const {
            return !(*this == other);
        }

        uint32_t    fID;
        int32_t     fTTCIndex;
        SkString    fString;
        SkFontStyle fStyle;

        // If buffer is NULL, just return the number of bytes that would have
        // been written. Will pad contents to a multiple of 4.
        size_t writeToMemory(void* buffer = NULL) const;

        // Recreate from a flattened buffer, returning the number of bytes read.
        size_t readFromMemory(const void* buffer, size_t length);
    };

    /**
     *  Given a familyName and style, find the best match.
     *
     *  If a match is found, return true and set its outFontIdentifier.
     *      If outFamilyName is not null, assign the found familyName to it
     *          (which may differ from the requested familyName).
     *      If outStyle is not null, assign the found style to it
     *          (which may differ from the requested style).
     *
     *  If a match is not found, return false, and ignore all out parameters.
     */
    virtual bool matchFamilyName(const char familyName[],
                                 SkTypeface::Style requested,
                                 FontIdentity* outFontIdentifier,
                                 SkString* outFamilyName,
                                 SkTypeface::Style* outStyle) = 0;

    /**
     *  Given a FontRef, open a stream to access its data, or return null
     *  if the FontRef's data is not available. The caller is responsible for
     *  deleting the stream when it is done accessing the data.
     */
    virtual SkStreamAsset* openStream(const FontIdentity&) = 0;

    /**
     *  Return an SkTypeface for the given FontIdentity.
     *
     *  The default implementation simply returns a new typeface built using data obtained from
     *  openStream(), but derived classes may implement more complex caching schemes.
     *
     *  Callers are responsible for unref-ing the result.
     */
    virtual SkTypeface* createTypeface(const FontIdentity& identity) {
        return SkTypeface::CreateFromStream(this->openStream(identity), identity.fTTCIndex);
    }

    /**
     *  Return a singleton instance of a direct subclass that calls into
     *  libfontconfig. This does not affect the refcnt of the returned instance.
     *  The mutex may be used to guarantee the singleton is only constructed once.
     */
    static SkFontConfigInterface* GetSingletonDirectInterface(SkBaseMutex* mutex = NULL);

    // New APIS, which have default impls for now (which do nothing)

    virtual SkDataTable* getFamilyNames() { return SkDataTable::NewEmpty(); }
    virtual bool matchFamilySet(const char[] /*inFamilyName*/,
                                SkString* /*outFamilyName*/,
                                SkTArray<FontIdentity>*) {
        return false;
    }
    typedef SkRefCnt INHERITED;
};

#endif
