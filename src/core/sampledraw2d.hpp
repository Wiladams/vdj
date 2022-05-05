#ifndef __sampledraw2d_hpp__
#define __sampledraw2d_hpp__

#include "draw2dcontext.hpp"

namespace alib {
    // Use a sampler to fill in a span
    // No clipping is performed
    // This is really a specialization of a one
    // line rectangle
    INLINE void sampleSpan(PixelView & pmap, const int x, const int y, const int len, ISample1D<PixelRGBA>& s)
    {
        for (int col = x; col < x + len; col++)
        {
            double u = alib::Map(col, x, (double)x + len - 1, 0, 1);
            PixelRGBA c = s.getValue(u);
            pmap.At<PixelRGBA>(col, y) = c;
        }
    }

    INLINE void sampleHLine2D(PixelView & pb, const GeoSpan<int>& span,
        double v, ISample2D<PixelRGBA>& src)
    {
        for (int x = span.x(); x < span.rightMost(); x++)
        {
            double u = alib::Map(x, span.x(), span.rightMost(), 0.0, 1.0);
            //pb.set(x, span.y(), src.getValue(u, v));
            pb.At<PixelRGBA>(x, span.y()) = src.getValue(u, v);
        }
    }


    INLINE void sampleTriangle(PixelView & pb, const ptrdiff_t x1, const ptrdiff_t y1,
        const int x2, const int y2,
        const int x3, const int y3,
        ISample2D<PixelRGBA>& src,
        const PixelRect& clipRect)
    {
        // Create a triangle object
        PixelTriangle tri(x1, y1, x2, y2, x3, y3);

        // sample polygon
    }

    INLINE void sampleCircle(PixelView & pmap, const ptrdiff_t centerX, const ptrdiff_t centerY, const ptrdiff_t radius, ISample2D<PixelRGBA>& fillStyle)
    {
        auto x1 = centerX - radius, y1 = centerY - radius;
        auto  x2 = centerX + radius, y2 = centerY + radius;
        // should check the clip region to optimize

        // The algorithm here is very straight forward
        // we have a bounding rectangle for the circle
        // so we just check each pixel within that rectangle
        // to see if it's inside, or outside the circle.
        // if it's inside, we set the color, otherwise not
        //
        for (int y = y1; y < y2; y++) {
            for (int x = x1; x < x2; x++) {
                auto distX = (x - centerX + 0.5), distY = (y - centerY + 0.5);
                auto distance = sqrt(distX * distX + distY * distY);
                if (distance <= radius) {
                    auto u = alib::Map(x, x1, x2, 0.0, 1.0);
                    auto v = alib::Map(y, y1, y2, 0.0, 1.0);
                    auto rgb = fillStyle.getValue(u, v);
                    pmap.getPixel(x, y) = rgb;
                }
            }
        }
    }

}
// namespace alib


#endif
