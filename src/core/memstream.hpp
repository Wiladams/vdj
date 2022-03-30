#pragma once

#include "apidefs.h"

#include <cstdint>
#include <algorithm>
#include <memory>

struct MemoryStream
{
    uint8_t*  fData;
    size_t          fSize;
    size_t          fPosition;

    // dealing with bits
    size_t          fBitCount;

#if defined(_M_X64)
    uint64_t fBitBuffer;
#else
    uint32_t fBitBuffer;
#endif

    // isGap() returns true if value is one of the 'gap' characters
    // ' ' - space
    // '\t' - tab
    // '\n' - newline
    // '\r' - return
    static INLINE bool isGap(uint8_t value)
    {
        return value == ' ' || value == '\t' || value == '\n' || value == '\r';
    }

public:
    INLINE MemoryStream(const uint8_t* data = nullptr, size_t size = 0)
    {
        init(data, size);
    }

    INLINE void init(const uint8_t* data, size_t size)
    {
        fPosition = 0;
        fData = data;
        fSize = size;
        fBitBuffer = 0;
        fBitCount = 0;
    }

    // Move the read head to the position indicated
    // only go as far as space allocated
    INLINE bool seek(size_t pos)
    {
        if (pos <= fSize)
        {
            fPosition = fSize;
            return true;
        }

        return false;
    }

    INLINE size_t size() const { return fSize; }
    INLINE size_t tell() const { return fPosition; }

    INLINE uint8_t* data() { return fData; }
    INLINE const uint8_t* data() const { return fData; }
    
    // Current - return pointer to the current position
    INLINE const uint8_t * current() const {return fData + fPosition;}

    // return character at current position without advancing cursor
    INLINE uint8_t peek() { return ((uint8_t *)(fData))[fPosition]; }





    // eof - end of file, no more data left
    INLINE bool eof() const { return fPosition >= fSize; }

    // returns true if the specified size can be read from stream
    INLINE bool canRead(size_t size) const { return fPosition + size <= fSize; }

    //
    // Utility functions 
    // 

    // skip
    // move position forward by specified amount
    // and return true
    // if can't move whole amount, don't move at all 
    // and return false
    bool skip(size_t size)
    {
        if (fPosition + size < fSize) {
            fPosition += size;
            return true;
        }
        else
            return false;
    }

    // skip over whitespace
    INLINE bool skipWhitespace()
    {
        while (isGap(fData[fPosition]) && fPosition < fSize)
            fPosition++;

        return fPosition < fSize;
    }

    //
    // skip everything that is not a gap character
    INLINE bool skipNonWhitespace()
    {
        while (!isGap(fData[fPosition]) && fPosition < fSize)
            fPosition++;

        return fPosition < fSize;
    }

    // the value specified is what we want to skip over
    // so skip until we no longer see that value
    // good for skipping over whitespace for example
    bool skipValue(uint8_t value)
    {
        while (fData[fPosition] == value && fPosition < fSize)
            fPosition++;

        return fPosition < fSize;
    }

    // Read the specified number of bytes from the stream
    // advancing the position
    // only read as many bytes as are available, and return
    // the actual number of bytes read
    INLINE size_t read(size_t size, void* data)
    {
        size_t s = std::min(fSize - fPosition, size);
        memcpy(data, fData + fPosition, s);
        fPosition += s;

        return s;
    }

    // Read a specific value
    template <class Value> INLINE bool read(Value& value)
    {
        return read(sizeof(Value), &value) == sizeof(Value);
    }

    INLINE bool read8u(uint8_t& value)
    {
        if (fPosition < fSize)
        {
            value = fData[fPosition++];
            return true;
        }
        else
            return false;
    }

    INLINE bool read16u(uint16_t& value)
    {
        if (fPosition + 2 <= fSize)
        {
            value = *(uint16_t*)(fData + fPosition);
            fPosition += 2;
            return true;
        }
        else
            return false;
    }

    INLINE bool read32u(uint32_t& value)
    {
        if (fPosition + 4 <= fSize) {
            value = *(uint32_t*)(fData + fPosition);
            fPosition += 4;
            return true;
        }
        else
            return false;
    }

#if defined(_M_X64)
    INLINE uint64_t& bitBuffer()
    {
        return fBitBuffer;
    }

    INLINE const uint64_t& bitBuffer() const
    {
        return fBitBuffer;
    }
#else
    INLINE uint32_t& bitBuffer()
    {
        return fBitBuffer;
    }

    INLINE const uint32_t& bitBuffer() const
    {
        return fBitBuffer;
    }
#endif

    INLINE size_t& bitCount() { return fBitCount; }
    INLINE const size_t& bitCount() const { return fBitCount; }

    INLINE void fillBits()
    {
        static const size_t canReadByte = (sizeof(fBitBuffer) - 1) * 8;
        while (fBitCount <= canReadByte && fPosition < fSize) {
            fBitBuffer |= (size_t)fData[fPosition++] << fBitCount;
            fBitCount += 8;
        }
    }

    INLINE void clearBits()
    {
        fPosition -= fBitCount / 8;
        fBitBuffer = 0;
        fBitCount = 0;
    }

    // Try to read the number of bits specified
    // if not enough bits available, then return false
    INLINE bool readBits(size_t& bits, size_t count)
    {
        if (fBitCount < count)
            fillBits();

        if (fBitCount < count)
            return false;

        bits = fBitBuffer & ((size_t(1) << count) - 1);
        fBitBuffer >>= count;
        fBitCount -= count;

        return true;
    }

    // read number of bits specified
    // if not enough bits, then value
    // is undefined
    INLINE size_t readBits(size_t count)
    {
        if (fBitCount < count)
            fillBits();

        size_t bits = fBitBuffer & ((size_t(1) << count) - 1);
        fBitBuffer >>= count;
        fBitCount -= count;

        return bits;
    }
};