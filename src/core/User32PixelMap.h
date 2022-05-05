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
#include "vdjview.hpp"
#include "agg/agg_rendering_buffer.h"

#include <windows.h>
#include <cstdio>

namespace alib {
    
    class U32DIBSection : public agg::row_accessor<uint8_t>, public ISample2D<PixelRGBA>
    {
        // for interacting with win32
        BITMAPINFO fBMInfo{ {0} };
        HBITMAP fDIBHandle = nullptr;
        HDC     fBitmapDC = nullptr;

        // A couple of constants
        static const int bitsPerPixel = 32;
        static const int alignment = 4;
        
    public:
        U32DIBSection(const size_t awidth, const size_t aheight)
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

        virtual ~U32DIBSection()
        {
            // and destroy it
            ::DeleteObject(fDIBHandle);
        }

        INLINE BITMAPINFO getBitmapInfo() { return fBMInfo; }
        INLINE HDC getDC() { return fBitmapDC; }                // Memory DC we can use for GDI

        PixelRGBA getPixel(size_t x, size_t y)
        {
            return ((PixelRGBA*)row_ptr(y))[x];
        }

        PixelRGBA getValue(double u, double v) override
        {
            u = alib::Clamp(u, 0, 1);
            v = alib::Clamp(v, 0, 1);

            size_t px = size_t((u * ((double)width() - 1)) + 0.5);
            size_t py = size_t((v * ((double)height() - 1)) + 0.5);

            return ((PixelRGBA*)row_ptr(py))[px];
        }
    };
    
    /*
    class User32PixelMap : public agg::row_accessor<uint8_t>
    {
        // for interacting with win32
        BITMAPINFO fBMInfo{ {0} };
        HBITMAP fDIBHandle = nullptr;
        HGDIOBJ fOriginDIBHandle = nullptr;
        HDC     fBitmapDC = nullptr;

        uint8_t* fData = nullptr;
        size_t fDataSize = 0;       // How much data is allocated
        ptrdiff_t fStride=0;

        // A couple of constants
        static const int bitsPerPixel = 32;
        static const int alignment = 4;

    public:
        User32PixelMap(const size_t awidth, const size_t aheight)
        {
            init(awidth, aheight);
        }

        virtual ~User32PixelMap()
        {
            // BUGBUG
            // unload the dib section
            ::SelectObject(fBitmapDC, fOriginDIBHandle);

            // and destroy it
            ::DeleteObject(fDIBHandle);
        }

        bool init(size_t awidth, size_t aheight)
        {
            fStride = alib::GetAlignedByteCount(awidth, bitsPerPixel, alignment);

            fBMInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            fBMInfo.bmiHeader.biWidth = (LONG)awidth;
            fBMInfo.bmiHeader.biHeight = -(LONG)aheight;	// top-down DIB Section
            fBMInfo.bmiHeader.biPlanes = 1;
            fBMInfo.bmiHeader.biBitCount = bitsPerPixel;
            fBMInfo.bmiHeader.biSizeImage = (DWORD)(fStride * aheight);
            fBMInfo.bmiHeader.biClrImportant = 0;
            fBMInfo.bmiHeader.biClrUsed = 0;
            fBMInfo.bmiHeader.biCompression = BI_RGB;
            fDataSize = fBMInfo.bmiHeader.biSizeImage;

            // We'll create a DIBSection so we have an actual backing
            // storage for the context to draw into
            // BUGBUG - check for nullptr and fail if found
            fDIBHandle = ::CreateDIBSection(nullptr, &fBMInfo, DIB_RGB_COLORS, (void**)&fData, nullptr, 0);

            //Recreate(awidth, aheight, Simd::View<Simd::Allocator>::Format::Bgra32, data, alignment);

            // Create a GDI Device Context
            fBitmapDC = ::CreateCompatibleDC(nullptr);

            // select the DIBSection into the memory context so we can 
            // peform operations with it
            fOriginDIBHandle = ::SelectObject(fBitmapDC, fDIBHandle);

            // Do some setup to the DC to make it suitable
            // for drawing with GDI if we choose to do that
            ::SetBkMode(fBitmapDC, TRANSPARENT);
            ::SetGraphicsMode(fBitmapDC, GM_ADVANCED);


            attach(fData, awidth, aheight, fStride);

            return true;
        }

        INLINE BITMAPINFO getBitmapInfo() { return fBMInfo; }
        INLINE HDC getDC() { return fBitmapDC; }                // Memory DC we can use for GDI

    };
    */
} // namespace alib
