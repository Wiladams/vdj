#pragma once
// ScreenSnapshot
//
// Take a snapshot of a portion of the screen and hold
// it in a PixelMap (User32PixelMap)
// A sampler2D interface is also provided so you can 
// either use the pixel oriented 'get()' function, or the 
// parametric 'getValue()' function.
//
// When constructed, a single snapshot is taken.
// every time you want a new snapshot, just call 'next()'
// This is great for doing a live screen capture
//
//    ScreenSnapshot ss(x,y, width, height);
//
//    References:
//    https://www.codeproject.com/articles/5051/various-methods-for-capturing-the-screen
//    https://stackoverflow.com/questions/5069104/fastest-method-of-screen-capturing-on-windows
//  https://github.com/bmharper/WindowsDesktopDuplicationSample
//

#include "User32PixelMap.h"
#include "sampler.hpp"

namespace alib
{
    class ScreenSnapshot : public U32DIBSection
    {
        HDC fSourceDC = nullptr;  // Device Context we're going to snapshot

        // which location winthin the sourceDC are we capturing
        ptrdiff_t fOriginX=0;
        ptrdiff_t fOriginY=0;

    public:
        ScreenSnapshot(ptrdiff_t originX, ptrdiff_t originY, size_t awidth, size_t aheight, HDC sourceDC)
            : U32DIBSection(awidth, aheight)
            , fSourceDC(sourceDC)
            , fOriginX(originX)
            , fOriginY(originY)
        {
            // Take a snapshot to start
            next();
        }


        // take a snapshot of current screen
        bool next()
        {
            BitBlt(getDC(), 0, 0, width(), height(), fSourceDC, fOriginX, fOriginY, SRCCOPY | CAPTUREBLT);

            return true;
        }


        static std::shared_ptr<ScreenSnapshot> createForDisplay(int x, int y, int w, int h)
        {
            auto sourceDC = GetDC(nullptr);

            return std::make_shared<ScreenSnapshot>(x, y, w, h, sourceDC);
        }

        static std::shared_ptr<ScreenSnapshot> createForWindow(int x, int y, int w, int h, HWND hWnd)
        {
            auto sourceDC = GetWindowDC(hWnd);
            if (nullptr == sourceDC)
                return nullptr;

            return std::make_shared<ScreenSnapshot>(x, y, w, h, sourceDC);
        }
    };

}