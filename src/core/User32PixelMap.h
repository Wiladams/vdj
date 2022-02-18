#pragma once

/*
    Simple representation of a bitmap we can use to draw into a window.
    We use a DIBSection because that's the easiest object within Windows 
    to get a pointer to the pixel data.

    we assume a format of 32-bit RGBA to make life very simple.

    A DC Context is also associated with the bitmap, to make it easier 
    to interact with other User32 and GDI interfaces.
*/



#include "vdjview.hpp"

#include <windows.h>
#include <cstdio>

class User32PixelMap : public vdj::PixelView
{
    // for interacting with win32
    BITMAPINFO fBMInfo{ {0} };
    HBITMAP fDIBHandle = nullptr;
    HGDIOBJ fOriginDIBHandle = nullptr;
    HDC     fBitmapDC = nullptr;

    size_t fDataSize=0;       // How much data is allocated

    // A couple of constants
    static const int bitsPerPixel = 32;
    static const int alignment = 4;

public:

    User32PixelMap(const long awidth, const long aheight)
        : vdj::PixelView(awidth,aheight,awidth*4, vdj::PixelView::Format::Bgra32)
    {
        init(awidth, aheight);
    }

    virtual ~User32PixelMap()
    {
        // BUGBUG
        // unload the dib section
        //::SelectObject(fBitmapDC, fOriginDIBHandle);

        // and destroy it
        ::DeleteObject(fDIBHandle);
    }

    bool init(int awidth, int aheight)
    {
        fStride = vdj::GetAlignedByteCount(awidth, bitsPerPixel, alignment);

        fBMInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        fBMInfo.bmiHeader.biWidth = awidth;
        fBMInfo.bmiHeader.biHeight = -(LONG)aheight;	// top-down DIB Section
        fBMInfo.bmiHeader.biPlanes = 1;
        fBMInfo.bmiHeader.biBitCount = bitsPerPixel;
        fBMInfo.bmiHeader.biSizeImage = fStride * aheight;
        fBMInfo.bmiHeader.biClrImportant = 0;
        fBMInfo.bmiHeader.biClrUsed = 0;
        fBMInfo.bmiHeader.biCompression = BI_RGB;
        fDataSize = fBMInfo.bmiHeader.biSizeImage;

        // We'll create a DIBSection so we have an actual backing
        // storage for the context to draw into
        // BUGBUG - check for nullptr and fail if found
        fDIBHandle = ::CreateDIBSection(nullptr, &fBMInfo, DIB_RGB_COLORS, (void **) & fData, nullptr, 0);


        // Create a GDI Device Context
        fBitmapDC = ::CreateCompatibleDC(nullptr);

        // select the DIBSection into the memory context so we can 
        // peform operations with it
        fOriginDIBHandle = ::SelectObject(fBitmapDC, fDIBHandle);

        // Do some setup to the DC to make it suitable
        // for drawing with GDI if we choose to do that
        ::SetBkMode(fBitmapDC, TRANSPARENT);
        ::SetGraphicsMode(fBitmapDC, GM_ADVANCED);

        return true;
    }

    // Make sure all GDI drawing, if any, has completed
    void flush()
    {
        ::GdiFlush();
    }

    INLINE BITMAPINFO getBitmapInfo() { return fBMInfo; }
    INLINE HDC getDC() { return fBitmapDC; }                // Memory DC we can use for GDI

 };
