#ifndef __sampledraw2d_hpp__
#define __sampledraw2d_hpp__

#include "draw2d.hpp"

namespace alib {
    // Use a sampler to fill in a span
    // No clipping is performed
    // This is really a specialization of a one
    // line rectangle
    INLINE void sampleSpan(PixelView & pmap, const int x, const int y, const int width, ISample1D<PixelRGBA>& s)
    {
        for (int col = x; col < x + width; col++)
        {
            double u = alib::Map(col, x, (double)x + width - 1, 0, 1);
            PixelRGBA c = s.getValue(u);
            //pmap.set(col, y, c);
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

    //
    // fill in a rectangle using the specified 
    // 2D sampler.  
    // Here we assume clipping has already occured
    // and the srcExt is already calculated  to capture
    // the desired section of the sampler
    INLINE void sampleRect(PixelView & pmap, const PixelRect& dstisect, const RectD& srcExt, ISample2D<PixelRGBA>& src)
    {
        // find the intersection between the source rectangle
        // and the frame
        //PixelRect dstisect = pmap.frame().intersection(dstFrame);

        // if the intersection is empty, we have
        // nothing to draw, so return
        if (dstisect.isEmpty())
            return;

        double uadv = (srcExt.w()) / (dstisect.w());
        double vadv = (srcExt.h()) / (dstisect.h());

        // Initial u and v values
        double u = srcExt.left();
        double v = srcExt.top();

        for (ptrdiff_t row = dstisect.y(); row < dstisect.y() + dstisect.h() - 1; row++)
        {
            for (ptrdiff_t col = dstisect.left(); col < dstisect.left() + dstisect.w() - 1; col++)
            {
                auto c = src.getValue(u, v);
                pmap.At<PixelRGBA>(col, row) = c;
                u += uadv;
            }
            v += vadv;
            u = srcExt.left();
        }
    }



    INLINE void sampledBezier(PixelView & pmap, const PixelBezier& bez, const int segments, ISample1D<PixelRGBA>& c)
    {
        // Get starting point
        auto lp = bez.eval(0);

        int i = 1;
        while (i <= segments) {
            double u = (double)i / segments;

            auto p = bez.eval(u);

            // draw line segment from last point to current point
            line(pmap, lp.x(), lp.y(), p.x(), p.y(), c.getValue(u));

            // Assign current to last
            lp = p;

            i = i + 1;
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
