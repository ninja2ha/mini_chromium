
/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkJpegUtility_DEFINED
#define SkJpegUtility_DEFINED

#include "include/core/SkImageDecoder.h"
#include "include/core/SkStream.h"

extern "C" {
    #include "jpeglib.h"
    #include "jerror.h"
}

#include <setjmp.h>

/* Our error-handling struct.
 *
*/
struct skjpeg_error_mgr : jpeg_error_mgr {
    jmp_buf fJmpBuf;
};


void skjpeg_error_exit(j_common_ptr cinfo);

///////////////////////////////////////////////////////////////////////////
/* Our source struct for directing jpeg to our stream object.
*/
struct skjpeg_source_mgr : jpeg_source_mgr {
    skjpeg_source_mgr(SkStream* stream, SkImageDecoder* decoder);

    // Unowned.
    SkStream*       fStream;
    // Unowned pointer to the decoder, used to check if the decoding process
    // has been cancelled.
    SkImageDecoder* fDecoder;
    enum {
        kBufferSize = 1024
    };
    char    fBuffer[kBufferSize];
};

/////////////////////////////////////////////////////////////////////////////
/* Our destination struct for directing decompressed pixels to our stream
 * object.
 */
struct skjpeg_destination_mgr : jpeg_destination_mgr {
    skjpeg_destination_mgr(SkWStream* stream);

    SkWStream*  fStream;

    enum {
        kBufferSize = 1024
    };
    uint8_t fBuffer[kBufferSize];
};

#endif
