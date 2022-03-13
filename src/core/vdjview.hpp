#pragma once

#include "pixeltypes.hpp"
#include "sampler.hpp"



#define blend_pixel(bg, fg) PixelRGBA(				\
	lerp255(bg.r(), fg.r(), fg.a()), \
	lerp255(bg.g(), fg.g(), fg.a()), \
	lerp255(bg.b(), fg.b(), fg.a()), fg.a())


namespace vdj
{


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

    struct PixelView 
    {
/*
        // Different pixel formats
        enum class Format {
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
        */
    protected:
        size_t      fWidth;     // Pixels wide
        size_t      fHeight;    // Pixels high
        ptrdiff_t   fStride;    // Bytes per row
        //const Format fFormat;    // The format of the pixels
        uint8_t* fData = nullptr;       // A pointer to the data
        bool        fOwner;     // Did we allocate the data

    public:
        PixelView();                     // Default constructor
        PixelView(const PixelView & view);  // copy constructor
        PixelView(const size_t w, const size_t h);


        // virtual destructor so base classes setup properly
        virtual ~PixelView() = default;

        // Useful attributes
        PixelView& ref();

        template <class T> const T* row(size_t row) const;
        template <class T> T* row(size_t r);

        //INLINE T* getData() { return (T*)fData; }
        //template <class T> 
        INLINE PixelRGBA * getPixelPointer(size_t x, size_t y) { return &((PixelRGBA*)fData)[(y * width()) + x]; }

        // regular things
        INLINE constexpr size_t width() const noexcept { return fWidth; }
        INLINE constexpr size_t height() const noexcept { return fHeight; }
        INLINE constexpr size_t rowStride() const noexcept { return fStride; }

        // Calculate whether a point is whithin our bounds
        INLINE bool contains(double x, double y) const { return getBounds().contains((int)x, (int)y); }
        INLINE PixelRect getBounds() const { return PixelRect(0, 0, width(), height()); }
        INLINE PixelRect frame() const { return getBounds(); }

        INLINE PixelRGBA& getPixel(size_t x, size_t y);

        // Get constant reference to pixel
        template <class T> const T & At(size_t x, size_t y) const;
        
        // Get reference to pixel
        template <class T> T& At(size_t, size_t y);

        // Get constant reference to pixel
        template <class T> const T& At(const Point<ptrdiff_t>& pt) const;

        // Get reference to pixel
        template <class T> T& At(const Point<ptrdiff_t>& pt);

        /*
        // Set a pixel without checking constraints
        template <class T>
        INLINE void setPixel(const size_t x, const size_t y, const T& c)
        {
            if (c.isOpaque())
                At(x, y) = c;
            else
                At(x, y) = blend_pixel(At(x, y), c);
        }
        */

        // set consecutive pixels in a row 
    // Assume the range has already been clipped
        INLINE void setSpan(size_t x, size_t y, size_t w, const PixelRGBA& c);

        // Set every pixel to a specified value
        // we can use this fast intrinsic to fill
        // the whole area
        INLINE void setAllPixels(const PixelRGBA& c);

        //
        // set(), and get() are the general purpose ways to get and set
        // pixel values.  They will do bounds checking.  If you want 
        // to avoid bounds checking,then use the getPixel(), and setPixel() forms
        // which MUST be implemented by a sub-class
        //virtual void set(const int x, const int y, const T& c) {
        //    if (!contains(x, y))
        //        return;
        //    
        //    if (c.isTransparent())
       //         return;

        //    setPixel(x, y, c);
        //}




    };


    // Constructors
    // 
    // Default constructor
    INLINE PixelView::PixelView()
        : fWidth(0)
        , fHeight(0)
        , fStride(0)
        //, fFormat(None)
        , fData(nullptr)
        , fOwner(false)
    {
    }

    // Copy constructor
    INLINE PixelView::PixelView(const PixelView& view)
        : fWidth(view.fWidth)
        , fHeight(view.fHeight)
        , fStride(view.fStride)
        //, fFormat(view.fFormat)
        , fData(view.fData)
        , fOwner(false)
    {
    }


    INLINE PixelView::PixelView(const size_t w, const size_t h)
        : fWidth(w)
        , fHeight(h)
        , fStride(0)
        //, fFormat(f)
        , fData(nullptr)
        , fOwner(false)
    {
        //if (fData == nullptr && fHeight && fWidth && fStride && (fFormat != None))
        //{
        //    *(void**)&fData = Allocator::Allocate(fHeight * fStride, Allocator::Alignment());
        //    _owner = true;
        //}
    }

    // Useful Properties 
    INLINE PixelView& PixelView::ref() { return *this; }

    // Getting and setting pixel values
    // Get constant reference to pixel
    template <class T> 
    INLINE const T& PixelView::At(size_t x, size_t y) const
    {
        assert(x < fWidth && (y < fHeight));
        return ((const T*)(fData + y * fStride))[x];
    }


    // Get reference to pixel
    template<class T>
    INLINE T& PixelView::At(size_t x, size_t y)
    {
        assert(x < fWidth&& y < fHeight);
        return ((T *)(fData + y * fStride))[x];
    }

    // Get constant reference to pixel
    template<class T>
    INLINE const T& PixelView::At(const Point<ptrdiff_t>& pt) const
    {
        return At(pt.x, pt.y);
    }

    // Get reference to pixel
    template<class T>
    INLINE T& PixelView::At(const Point<ptrdiff_t>& pt)
    {
        return At(pt.x(), pt.y());
    }


    // Get Pixel
    INLINE PixelRGBA& PixelView::getPixel(size_t x, size_t y) 
    { 
        return At<PixelRGBA>(x, y); 
    }


    template<class T> INLINE const T* PixelView::row(size_t r) const
    {
        assert(r < fHeight);
        return ((const T*)(fData + r * fStride));
    }

    template<class T> INLINE T* PixelView::row(size_t r)
    {
        assert(r < fHeight);
        return ((T*)(fData + r * fStride));
    }

    // Set all pixels to a given value
    INLINE void PixelView::setAllPixels(const PixelRGBA& c)
    {
        uint32_t* pixelPtr = (uint32_t*)getPixelPointer(0, 0);
        size_t nPixels = width() * height();

        for (int i = 0; i < nPixels; i++)
            pixelPtr[i] = c.value;
    }

    INLINE void PixelView::setSpan(size_t x, size_t y, size_t w, const PixelRGBA& c)
    {
        // Do an assert check
        // BUGBUG - should do line clipping
        PixelRGBA* pixelPtr = (PixelRGBA*)getPixelPointer(x, y);

        // do the opaqueness check only once instead
        // of inside the loop
        // this should be more easily optimized
        if (c.isOpaque()) {
            for (int i = 0; i < w; i++)
                pixelPtr[i] = c;
        }
        else {
            for (int i = 0; i < w; i++)
                pixelPtr[i] = blend_pixel(pixelPtr[i], c);
        }
    }
    

    struct ViewSampler : public ISample2D<PixelRGBA>
    {
        PixelView & fWrapped;

        ViewSampler(PixelView& wrapped)
            : fWrapped(wrapped)
        {}

        // ISample2D<PixelRGBA>
// specify location in normalized space [0..1]
        PixelRGBA getValue(double u, double v) override
        {
            size_t px = size_t((u * ((double)fWrapped.width() - 1)) + 0.5);
            size_t py = size_t((v * ((double)fWrapped.height() - 1)) + 0.5);

            return fWrapped.At<PixelRGBA>(px, py);
        }

        /*
        // when checking bounds, return totally transparent
        // when outside bounds
        // specify coordinates in pixel space [0..width,height]
        T get(const int x, const int y) const
        {
            // reject pixel if out of boundary
            if (!contains(x, y))
                return T();

            return At(x, y);
        }
        */
    };
    
}
// namespace vdj

