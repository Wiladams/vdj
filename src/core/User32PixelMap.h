#pragma once

/*
    Simple representation of a bitmap we can use to draw into a window.
    We use a DIBSection because that's the easiest object within Windows 
    to get a pointer to the pixel data.

    we assume a format of 32-bit RGBA to make life very simple.

    A DC Context is also associated with the bitmap, to make it easier 
    to interact with other User32 and GDI interfaces.
*/


#include "bitbang.hpp"
#include "pixeltypes.hpp"
#include "sampler.hpp"

#include "agg/agg_rendering_buffer.h"


#include <windows.h>
#include <cstdio>

namespace alib {
    //rendering_buffer
    class U32DIBSection : public agg::rendering_buffer, public ISample2D<PixelRGBA>
    {
        // for interacting with win32
        BITMAPINFO fBMInfo{ {0} };
        HBITMAP fDIBHandle = nullptr;
        HDC     fBitmapDC = nullptr;

        // A couple of constants
        static const int bitsPerPixel = 32;
        static const int alignment = 4;
        
    public:
        U32DIBSection(const size_t awidth, const size_t aheight);
        virtual ~U32DIBSection();

        BITMAPINFO getBitmapInfo();
        HDC getDC();

        PixelRGBA getPixel(size_t x, size_t y);
        PixelRGBA getValue(double u, double v) override;
    };
    
    INLINE U32DIBSection::U32DIBSection(const size_t awidth, const size_t aheight)
    {
        int stride = alib::GetAlignedByteCount(awidth, bitsPerPixel, alignment);

        fBMInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        fBMInfo.bmiHeader.biWidth = (LONG)awidth;
        fBMInfo.bmiHeader.biHeight = -(LONG)aheight;	// top-down DIB Section
        fBMInfo.bmiHeader.biPlanes = 1;
        fBMInfo.bmiHeader.biBitCount = bitsPerPixel;
        fBMInfo.bmiHeader.biSizeImage = (DWORD)(stride * aheight);
        fBMInfo.bmiHeader.biClrImportant = 0;
        fBMInfo.bmiHeader.biClrUsed = 0;
        fBMInfo.bmiHeader.biCompression = BI_RGB;

        // We'll create a DIBSection so we have an actual backing
        // storage for the context to draw into
        // BUGBUG - check for nullptr and fail if found
        uint8_t* data;
        fDIBHandle = ::CreateDIBSection(nullptr, &fBMInfo, DIB_RGB_COLORS, (void**)&data, nullptr, 0);

        // Create a GDI Device Context
        fBitmapDC = ::CreateCompatibleDC(nullptr);

        // select the DIBSection into the memory context so we can 
        // peform operations with it
        ::SelectObject(fBitmapDC, fDIBHandle);

        attach(data, awidth, aheight, stride);
    }

    INLINE U32DIBSection::~U32DIBSection()
    {
        // and destroy it
        ::DeleteObject(fDIBHandle);
    }

    INLINE BITMAPINFO U32DIBSection::getBitmapInfo() { return fBMInfo; }
    INLINE HDC U32DIBSection::getDC() { return fBitmapDC; }

    INLINE PixelRGBA U32DIBSection::getPixel(size_t x, size_t y)
    {
        // Data is in BGRA format
        auto data = row_ptr(y) + x * 4;
        return PixelRGBA(data[2], data[1], data[0], data[3]);
    }

    INLINE PixelRGBA U32DIBSection::getValue(double u, double v)
    {
        u = alib::Clamp(u, 0, 1);
        v = alib::Clamp(v, 0, 1);

        size_t px = size_t((u * ((double)width() - 1)) + 0.5);
        size_t py = size_t((v * ((double)height() - 1)) + 0.5);

        // Data is in BGRA format
        auto data = row_ptr(py) + px * 4;
        return PixelRGBA(data[2], data[1], data[0], data[3]);
    }

} // namespace alib
