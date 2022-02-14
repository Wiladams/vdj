#pragma once

#include "pixeltypes.h"

#include <intrin.h>

namespace vdj
{
    // Use this intrinsic for fast memory copies
#pragma intrinsic(__stosd)

#define blend_pixel(bg, fg) PixelRGBA(				\
	lerp255(bg.r(), fg.r(), fg.a()), \
	lerp255(bg.g(), fg.g(), fg.a()), \
	lerp255(bg.b(), fg.b(), fg.a()), fg.a())

/*
    A PixelMap is essentially a 2D array of pixels.  Pixels
    are a hard representation of a color value, suitable for display
    on the screen.  Pixels are RGBA values, not "color".  So they're not
    in a HSB, CYMK, or any other color space.  They're in the color space
    that's expected by the screen, or whatever is displaying the values.
*/

/*
    PixelMap is an abstraction for a 2D representation of Pixel values.
    We want this abstraction layer so that we can implement different
    forms of PixelMaps without changing the API for how to interact
    with them.

    The most essential functions related to a PixelMap are being able
    to set and get pixel values.  Beyond that, there might be some
    convenience functions for setting big chunks of pixels, like
    filling the whole map to a single color, or drawing
    lines.  This leaves space for implementations to optimize
    these functions in ways that are specific to them.
*/
    struct PixelView : public ISample2D<PixelRGBA>
    {
    public:

        // Different pixel formats
        enum Format {
            None = 0,
            Gray8,
            Uv16,
            Bgr24,
            Bgra32,
            Int16,
            Int32,
            Int64,
            Float,
            Double,
            BayerGrbg,
            BayerGbrg,
            BayerRggb,
            BayerBggr,
            Hsv24,          // Hue Saturation Value
            Hsl24,          // Hue Saturation Lightness
            Rgb24,          // (3 8-bit channels) RGB, Red Green Blue
            Rgba32,         // (4 8-bit channels) RGBA, Red Green Blue Alpha
            Uyvy16,         // (2 8-bit channels) UYVY422 

        };


        PixelView(const size_t w, const size_t h, ptrdiff_t stride, Format f)
            : fWidth(w)
            , fHeight(h)
            , fStride(stride)
            , fFormat(f)
        {
        }

        // virtual destructor so base classes setup properly
        virtual ~PixelView() = default;

        INLINE PixelRGBA* getData() { return (PixelRGBA*)fData; }
        INLINE PixelRGBA* getPixelPointer(size_t x, size_t y) { return &((PixelRGBA*)fData)[(y * width()) + x]; }

        INLINE const PixelRGBA& At(size_t x, size_t y) const
        {
            assert(x < fWidth&& y < fHeight);
            return ((const PixelRGBA*)(fData + y * fStride))[x];
        }

        INLINE PixelRGBA& At(size_t x, size_t y)
        {
            assert(x < fWidth&& y < fHeight);
            return ((PixelRGBA*)(fData + y * fStride))[x];
        }

        INLINE void copyPixel(const int x, const int y, const PixelRGBA& c)
        {
            At(x, y) = c;
        }

        // Perform SRCOVER operation on a pixel
        INLINE void blendPixel(size_t x, size_t y, const PixelRGBA& c)
        {
            At(x, y) = blend_pixel(At(x, y), c);
        }

        // Things a sub-class MUST implement
        virtual bool init(int w, int h) = 0;

        // set consecutive pixels in a row 
    // Assume the range has already been clipped
        INLINE void setPixels(size_t x, size_t y, size_t w, const PixelRGBA& src)
        {
            // do line clipping
            // copy actual pixel data
            uint32_t* pixelPtr = (uint32_t*)getPixelPointer(x, y);
            __stosd((unsigned long*)pixelPtr, src.value, w);
        }

        // Set every pixel to a specified value
        // we can use this fast intrinsic to fill
        // the whole area
        INLINE void setAllPixels(const PixelRGBA& c)
        {
            size_t nPixels = width() * height();
            __stosd((unsigned long*)fData, c.value, nPixels);
        }

        //virtual PixelRGBA getPixel(const int x, const int y) const = 0;
        INLINE PixelRGBA getPixel(size_t x, size_t y) const
        {
            return At(x, y);
            // Get data from BLContext
            //size_t offset = (size_t)(y * width()) + (size_t)x;
            //return ((PixelRGBA*)fData)[offset];
        }


        // regular things
        INLINE constexpr int width() const noexcept { return fWidth; }
        INLINE constexpr int height() const noexcept { return fHeight; }
        INLINE constexpr size_t rowStride() const noexcept { return fStride; }

        // Calculate whether a point is whithin our bounds
        INLINE bool contains(double x, double y) const { return getBounds().contains((int)x, (int)y); }
        const PixelRect& getBounds() const { return PixelRect(0, 0, width(), height()); }
        INLINE const PixelRect& frame() const { return getBounds(); }

        //
        // set(), and get() are the general purpose ways to get and set
        // pixel values.  They will do bounds checking.  If you want 
        // to avoid bounds checking,then use the getPixel(), and setPixel() forms
        // which MUST be implemented by a sub-class
        virtual void set(const int x, const int y, const PixelRGBA& c) {
            if (!contains(x, y))
                return;

            if (c.isTransparent())
                return;

            if (c.isOpaque())
                copyPixel(x, y, c);
            else
                blendPixel(x, y, c);
        }

        // when checking bounds, return totally transparent
        // when outside bounds
        // specify coordinates in pixel space [0..width,height]
        PixelRGBA get(const int x, const int y) const
        {
            // reject pixel if out of boundary
            if (!contains(x, y))
                return PixelRGBA(0);

            return getPixel(x, y);
        }

        // ISample2D<PixelRGBA>
        // specify location in normalized space [0..1]
        PixelRGBA getValue(double u, double v) override
        {
            int px = int((u * ((double)width() - 1)) + 0.5);
            int py = int((v * ((double)height() - 1)) + 0.5);

            return getPixel(px, py);
        }

    protected:
        size_t      fWidth;     // Pixels wide
        size_t      fHeight;    // Pixels high
        ptrdiff_t   fStride;    // Bytes per row
        const Format fFormat;    // The format of the pixels
        uint8_t* fData = nullptr;       // A pointer to the data
    };

}
// namespace vdj

