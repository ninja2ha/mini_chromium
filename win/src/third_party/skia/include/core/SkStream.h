/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStream_DEFINED
#define SkStream_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"

class SkData;

class SkStream;
class SkStreamRewindable;
class SkStreamSeekable;
class SkStreamAsset;
class SkStreamMemory;

/**
 *  SkStream -- abstraction for a source of bytes. Subclasses can be backed by
 *  memory, or a file, or something else.
 *
 *  NOTE:
 *
 *  Classic "streams" APIs are sort of async, in that on a request for N
 *  bytes, they may return fewer than N bytes on a given call, in which case
 *  the caller can "try again" to get more bytes, eventually (modulo an error)
 *  receiving their total N bytes.
 *
 *  Skia streams behave differently. They are effectively synchronous, and will
 *  always return all N bytes of the request if possible. If they return fewer
 *  (the read() call returns the number of bytes read) then that means there is
 *  no more data (at EOF or hit an error). The caller should *not* call again
 *  in hopes of fulfilling more of the request.
 */
class SK_API SkStream : public SkNoncopyable {
public:
    virtual ~SkStream() {}

    /**
     *  Attempts to open the specified file, and return a stream to it (using
     *  mmap if available). On success, the caller is responsible for deleting.
     *  On failure, returns NULL.
     */
    static SkStreamAsset* NewFromFile(const char path[]);

    /** Reads or skips size number of bytes.
     *  If buffer == NULL, skip size bytes, return how many were skipped.
     *  If buffer != NULL, copy size bytes into buffer, return how many were copied.
     *  @param buffer when NULL skip size bytes, otherwise copy size bytes into buffer
     *  @param size the number of bytes to skip or copy
     *  @return the number of bytes actually read.
     */
    virtual size_t read(void* buffer, size_t size) = 0;

    /** Skip size number of bytes.
     *  @return the actual number bytes that could be skipped.
     */
    size_t skip(size_t size) {
        return this->read(NULL, size);
    }

    /**
     *  Attempt to peek at size bytes.
     *  If this stream supports peeking, copy min(size, peekable bytes) into
     *  buffer, and return the number of bytes copied.
     *  If the stream does not support peeking, or cannot peek any bytes,
     *  return 0 and leave buffer unchanged.
     *  The stream is guaranteed to be in the same visible state after this
     *  call, regardless of success or failure.
     *  @param buffer Must not be NULL, and must be at least size bytes. Destination
     *      to copy bytes.
     *  @param size Number of bytes to copy.
     *  @return The number of bytes peeked/copied.
     */
    virtual size_t peek(void* /*buffer*/, size_t /*size*/) const { return 0; }

    /** Returns true when all the bytes in the stream have been read.
     *  This may return true early (when there are no more bytes to be read)
     *  or late (after the first unsuccessful read).
     */
    virtual bool isAtEnd() const = 0;

    int8_t   readS8();
    int16_t  readS16();
    int32_t  readS32();

    uint8_t  readU8() { return (uint8_t)this->readS8(); }
    uint16_t readU16() { return (uint16_t)this->readS16(); }
    uint32_t readU32() { return (uint32_t)this->readS32(); }

    bool     readBool() { return this->readU8() != 0; }
    SkScalar readScalar();
    size_t   readPackedUInt();

//SkStreamRewindable
    /** Rewinds to the beginning of the stream. Returns true if the stream is known
     *  to be at the beginning after this call returns.
     */
    virtual bool rewind() { return false; }

    /** Duplicates this stream. If this cannot be done, returns NULL.
     *  The returned stream will be positioned at the beginning of its data.
     */
    virtual SkStreamRewindable* duplicate() const { return NULL; }

//SkStreamSeekable
    /** Returns true if this stream can report it's current position. */
    virtual bool hasPosition() const { return false; }
    /** Returns the current position in the stream. If this cannot be done, returns 0. */
    virtual size_t getPosition() const { return 0; }

    /** Seeks to an absolute position in the stream. If this cannot be done, returns false.
     *  If an attempt is made to seek past the end of the stream, the position will be set
     *  to the end of the stream.
     */
    virtual bool seek(size_t /*position*/) { return false; }

    /** Seeks to an relative offset in the stream. If this cannot be done, returns false.
     *  If an attempt is made to move to a position outside the stream, the position will be set
     *  to the closest point within the stream (beginning or end).
     */
    virtual bool move(long /*offset*/) { return false; }

    /** Duplicates this stream. If this cannot be done, returns NULL.
     *  The returned stream will be positioned the same as this stream.
     */
    virtual SkStreamSeekable* fork() const { return NULL; }

//SkStreamAsset
    /** Returns true if this stream can report it's total length. */
    virtual bool hasLength() const { return false; }
    /** Returns the total length of the stream. If this cannot be done, returns 0. */
    virtual size_t getLength() const { return 0; }

//SkStreamMemory
    /** Returns the starting address for the data. If this cannot be done, returns NULL. */
    //TODO: replace with virtual const SkData* getData()
    virtual const void* getMemoryBase() { return NULL; }
};

/** SkStreamRewindable is a SkStream for which rewind and duplicate are required. */
class SK_API SkStreamRewindable : public SkStream {
public:
    bool rewind() override = 0;
    SkStreamRewindable* duplicate() const override = 0;
};

/** SkStreamSeekable is a SkStreamRewindable for which position, seek, move, and fork are required. */
class SK_API SkStreamSeekable : public SkStreamRewindable {
public:
    SkStreamSeekable* duplicate() const override = 0;

    bool hasPosition() const override { return true; }
    size_t getPosition() const override = 0;
    bool seek(size_t position) override = 0;
    bool move(long offset) override = 0;
    SkStreamSeekable* fork() const override = 0;
};

/** SkStreamAsset is a SkStreamSeekable for which getLength is required. */
class SK_API SkStreamAsset : public SkStreamSeekable {
public:
    SkStreamAsset* duplicate() const override = 0;
    SkStreamAsset* fork() const override = 0;

    bool hasLength() const override { return true; }
    size_t getLength() const override = 0;
};

/** SkStreamMemory is a SkStreamAsset for which getMemoryBase is required. */
class SK_API SkStreamMemory : public SkStreamAsset {
public:
    SkStreamMemory* duplicate() const override = 0;
    SkStreamMemory* fork() const override = 0;

    const void* getMemoryBase() override = 0;
};

class SK_API SkWStream : SkNoncopyable {
public:
    virtual ~SkWStream();

    /** Called to write bytes to a SkWStream. Returns true on success
        @param buffer the address of at least size bytes to be written to the stream
        @param size The number of bytes in buffer to write to the stream
        @return true on success
    */
    virtual bool write(const void* buffer, size_t size) = 0;
    virtual void newline();
    virtual void flush();

    virtual size_t bytesWritten() const = 0;

    // helpers

    bool    write8(U8CPU);
    bool    write16(U16CPU);
    bool    write32(uint32_t);

    bool    writeText(const char text[]);
    bool    writeDecAsText(int32_t);
    bool    writeBigDecAsText(int64_t, int minDigits = 0);
    bool    writeHexAsText(uint32_t, int minDigits = 0);
    bool    writeScalarAsText(SkScalar);

    bool    writeBool(bool v) { return this->write8(v); }
    bool    writeScalar(SkScalar);
    bool    writePackedUInt(size_t);

    bool    writeStream(SkStream* input, size_t length);

    /**
     * This returns the number of bytes in the stream required to store
     * 'value'.
     */
    static int SizeOfPackedUInt(size_t value);
};

////////////////////////////////////////////////////////////////////////////////////////

#include "include/core/SkString.h"
#include <stdio.h>

/** A stream that wraps a C FILE* file stream. */
class SK_API SkFILEStream : public SkStreamAsset {
public:
    /** Initialize the stream by calling sk_fopen on the specified path.
     *  This internal stream will be closed in the destructor.
     */
    explicit SkFILEStream(const char path[] = NULL);

    enum Ownership {
        kCallerPasses_Ownership,
        kCallerRetains_Ownership
    };
    /** Initialize the stream with an existing C file stream.
     *  While this stream exists, it assumes exclusive access to the C file stream.
     *  The C file stream will be closed in the destructor unless the caller specifies
     *  kCallerRetains_Ownership.
     */
    explicit SkFILEStream(FILE* file, Ownership ownership = kCallerPasses_Ownership);

    virtual ~SkFILEStream();

    /** Returns true if the current path could be opened. */
    bool isValid() const { return fFILE != NULL; }

    /** Close the current file, and open a new file with the specified path.
     *  If path is NULL, just close the current file.
     */
    void setPath(const char path[]);

    size_t read(void* buffer, size_t size) override;
    bool isAtEnd() const override;

    bool rewind() override;
    SkStreamAsset* duplicate() const override;

    size_t getPosition() const override;
    bool seek(size_t position) override;
    bool move(long offset) override;
    SkStreamAsset* fork() const override;

    size_t getLength() const override;

    const void* getMemoryBase() override;

private:
    FILE*     fFILE;
    SkString    fName;
    Ownership   fOwnership;
    // fData is lazilly initialized when needed.
    mutable SkAutoTUnref<SkData> fData;

    typedef SkStreamAsset INHERITED;
};

class SK_API SkMemoryStream : public SkStreamMemory {
public:
    SkMemoryStream();

    /** We allocate (and free) the memory. Write to it via getMemoryBase() */
    SkMemoryStream(size_t length);

    /** If copyData is true, the stream makes a private copy of the data. */
    SkMemoryStream(const void* data, size_t length, bool copyData = false);

    /** Use the specified data as the memory for this stream.
     *  The stream will call ref() on the data (assuming it is not NULL).
     */
    SkMemoryStream(SkData*);

    virtual ~SkMemoryStream();

    /** Resets the stream to the specified data and length,
        just like the constructor.
        if copyData is true, the stream makes a private copy of the data
    */
    virtual void setMemory(const void* data, size_t length,
                           bool copyData = false);
    /** Replace any memory buffer with the specified buffer. The caller
        must have allocated data with sk_malloc or sk_realloc, since it
        will be freed with sk_free.
    */
    void setMemoryOwned(const void* data, size_t length);

    /** Return the stream's data in a SkData.
     *  The caller must call unref() when it is finished using the data.
     */
    SkData* copyToData() const;

    /**
     *  Use the specified data as the memory for this stream.
     *  The stream will call ref() on the data (assuming it is not NULL).
     *  The function returns the data parameter as a convenience.
     */
    SkData* setData(SkData*);

    void skipToAlign4();
    const void* getAtPos();

    size_t read(void* buffer, size_t size) override;
    bool isAtEnd() const override;

    size_t peek(void* buffer, size_t size) const override;

    bool rewind() override;
    SkMemoryStream* duplicate() const override;

    size_t getPosition() const override;
    bool seek(size_t position) override;
    bool move(long offset) override;
    SkMemoryStream* fork() const override;

    size_t getLength() const override;

    const void* getMemoryBase() override;

private:
    SkData* fData;
    size_t  fOffset;

    typedef SkStreamMemory INHERITED;
};

/////////////////////////////////////////////////////////////////////////////////////////////

class SK_API SkFILEWStream : public SkWStream {
public:
    SkFILEWStream(const char path[]);
    virtual ~SkFILEWStream();

    /** Returns true if the current path could be opened.
    */
    bool isValid() const { return fFILE != NULL; }

    bool write(const void* buffer, size_t size) override;
    void flush() override;
    void fsync();
    size_t bytesWritten() const override;

private:
    FILE* fFILE;

    typedef SkWStream INHERITED;
};

class SkMemoryWStream : public SkWStream {
public:
    SkMemoryWStream(void* buffer, size_t size);
    bool write(const void* buffer, size_t size) override;
    size_t bytesWritten() const override { return fBytesWritten; }

private:
    char*   fBuffer;
    size_t  fMaxLength;
    size_t  fBytesWritten;

    typedef SkWStream INHERITED;
};

class SK_API SkDynamicMemoryWStream : public SkWStream {
public:
    SkDynamicMemoryWStream();
    virtual ~SkDynamicMemoryWStream();

    bool write(const void* buffer, size_t size) override;
    size_t bytesWritten() const override { return fBytesWritten; }
    // random access write
    // modifies stream and returns true if offset + size is less than or equal to getOffset()
    bool write(const void* buffer, size_t offset, size_t size);
    bool read(void* buffer, size_t offset, size_t size);
    size_t getOffset() const { return fBytesWritten; }

    // copy what has been written to the stream into dst
    void copyTo(void* dst) const;
    void writeToStream(SkWStream* dst) const;

    /**
     *  Return a copy of the data written so far. This call is responsible for
     *  calling unref() when they are finished with the data.
     */
    SkData* copyToData() const;

    /** Reset, returning a reader stream with the current content. */
    SkStreamAsset* detachAsStream();

    /** Reset the stream to its original, empty, state. */
    void reset();
    void padToAlign4();
private:
    struct Block;
    Block*  fHead;
    Block*  fTail;
    size_t  fBytesWritten;
    mutable SkData* fCopy;  // is invalidated if we write after it is created

    void invalidateCopy();

    // For access to the Block type.
    friend class SkBlockMemoryStream;
    friend class SkBlockMemoryRefCnt;

    typedef SkWStream INHERITED;
};


class SK_API SkDebugWStream : public SkWStream {
public:
    SkDebugWStream() : fBytesWritten(0) {}

    // overrides
    bool write(const void* buffer, size_t size) override;
    void newline() override;
    size_t bytesWritten() const override { return fBytesWritten; }

private:
    size_t fBytesWritten;
    typedef SkWStream INHERITED;
};

// for now
typedef SkFILEStream SkURLStream;

#endif
