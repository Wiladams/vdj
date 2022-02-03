#pragma once

#include "pixeltypes.h"

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
class PixelMap : public ISample2D<PixelRGBA>
{
protected:
    PixelRect fFrame;

public:
    PixelMap() :fFrame(0,0,0,0){}

    // virtual destructor so base classes setup properly
    PixelMap(int x, int y, int w, int h)
        :fFrame(x,y,w,h) {}

    virtual ~PixelMap() = default;

    // Things a sub-class MUST implement
    virtual bool init(int w, int h) = 0;
    virtual PixelRGBA* getPixelPointer(const int x, const int y) = 0;
    virtual size_t bytesPerRow() const = 0;
    virtual void copyPixel(const int x, const int y, const PixelRGBA &c) = 0;
    virtual void blendPixel(const int x, const int y, const PixelRGBA &c) = 0;
    virtual void setAllPixels(const PixelRGBA &c) = 0;
    virtual PixelRGBA getPixel(const int x, const int y) const = 0;

    // regular things
    INLINE constexpr int x() const noexcept { return fFrame.x(); }
    INLINE constexpr int y() const noexcept { return fFrame.y(); }
    INLINE constexpr int width() const noexcept { return fFrame.w(); }
    INLINE constexpr int height() const noexcept { return fFrame.h(); }
    
    // Calculate whether a point is whithin our bounds
    INLINE bool contains(double x, double y) const { return fFrame.contains((int)x, (int)y); }

    const PixelRect& getBounds() const { return fFrame; }
    INLINE const PixelRect& frame() const { return fFrame; }

    //
    // set(), and get() are the general purpose ways to get and set
    // pixel values.  They will do bounds checking.  If you want 
    // to avoid bounds checking,then use the getPixel(), and setPixel() forms
    // which MUST be implemented by a sub-class
    virtual void set(const int x, const int y, const PixelRGBA &c) {
        if (!fFrame.contains(x, y))
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
        if (!fFrame.contains(x, y))
            return PixelRGBA(0);

        return getPixel(x, y);
    }

    // ISample2D<PixelRGBA>
    // specify location in normalized space [0..1]
    // the PixelCoord tells you where in the caller the value
    // is intended to be located.
    PixelRGBA getValue(double u, double v) override
    {
        int px = int((u * ((double)width() - 1)) + 0.5);
        int py = int((v * ((double)height() - 1)) + 0.5);

        return getPixel(px, py);
        //return get(px, py);
    }
};


#define blend_pixel(bg, fg) PixelRGBA(				\
	lerp255(bg.r(), fg.r(), fg.a()), \
	lerp255(bg.g(), fg.g(), fg.a()), \
	lerp255(bg.b(), fg.b(), fg.a()), fg.a())
