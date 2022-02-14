#pragma once

#include "draw2d.h"

namespace vdj {
    // Use a sampler to fill in a span
    // No clipping is performed
    // This is really a specialization of a one
    // line rectangle
    inline void sampleSpan(PixelView& pmap, const int x, const int y, const int width, ISample1D<PixelRGBA>& s)
    {
        for (int col = x; col < x + width; col++)
        {
            double u = maths::Map(col, x, (double)x + width - 1, 0, 1);
            PixelRGBA c = s.getValue(u);
            pmap.set(col, y, c);
        }
    }

    inline void sampleHLine2D(PixelView& pb, const GeoSpan<int>& span,
        double v, ISample2D<PixelRGBA>& src)
    {
        for (int x = span.x(); x < span.rightMost(); x++)
        {
            double u = maths::Map(x, span.x(), span.rightMost(), 0.0, 1.0);
            pb.set(x, span.y(), src.getValue(u, v));
        }
    }

    //
    // fill in a rectangle using the specified 
    // 2D sampler.  
    // Here we assume clipping has already occured
    // and the srcExt is already calculated  to capture
    // the desired section of the sampler
    inline void sampleRect(PixelView& pmap, const PixelRect& dstisect, const RectD& srcExt, ISample2D<PixelRGBA>& src)
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

        for (int row = dstisect.y(); row < dstisect.y() + dstisect.h() - 1; row++)
        {
            for (int col = dstisect.x(); col < dstisect.x() + dstisect.w() - 1; col++)
            {
                auto c = src.getValue(u, v);
                pmap.set(col, row, c);
                u += uadv;
            }
            v += vadv;
            u = srcExt.left();
        }
    }

    /*
    void sampleRectangle(PixelMap& pmap, const PixelRect& dstFrame, ISample2D<PixelRGBA>& samp)
    {
        PixelRect dstisect = pmap.frame().intersection(dstFrame);
        RectD trex = RectD::create(dstisect, pmap.frame());

        // could we just do this?
        sampleRect(pmap, dstisect, trex, samp);
    }
    */

    // 
    // Draw a bezier line using a single line sampler
    INLINE void sampledBezier(PixelView& pmap, const GeoBezier<int>& bez, const int segments, ISample1D<PixelRGBA>& c)
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


    /*
    INLINE void sampleConvexPolygon(PixelView& pb,
        vdj::Point<int>* verts, const int nverts, int vmin,
        ISample2D<PixelRGBA>& src,
        const PixelRect& clipRect)
    {
        // set starting line
        APolyDda ldda, rdda;
        int y = verts[vmin].y();
        ldda.yend = rdda.yend = y;

        // find lowest and highest y values
        int miny = 65535;
        int maxy = -65536;
        for (int i = 0; i < nverts; i++)
        {
            if (verts[i].y() < miny)
                miny = verts[i].y();
            if (verts[i].y() > maxy)
                maxy = verts[i].y();
        }

        // setup polygon scanner for left side, starting from top
        ldda.setupPolyDda(verts, nverts, vmin, +1);

        // setup polygon scanner for right side, starting from top
        rdda.setupPolyDda(verts, nverts, vmin, -1);

        while (true)
        {
            if (y >= ldda.yend)
            {
                if (y >= rdda.yend)
                {
                    if (ldda.vertNext == rdda.vertNext) { // if same vertex, then done
                        break;
                    }

                    int vnext = rdda.vertNext - 1;

                    if (vnext < 0) {
                        vnext = nverts - 1;
                    }

                    if (vnext == ldda.vertNext)
                    {
                        break;
                    }
                }
                ldda.setupPolyDda(verts, nverts, ldda.vertNext, +1);	// reset left side
            }

            // check for right dda hitting end of polygon side
            // if so, reset scanner
            if (y >= rdda.yend) {
                rdda.setupPolyDda(verts, nverts, rdda.vertNext, -1);
            }

            // fill span between two line-drawers, advance drawers when
            // hit vertices
            if (y >= clipRect.y()) {
                int y1 = y;
                int y2 = y;
                int rx = round(rdda.x);
                int lx = round(ldda.x);
                int w = abs(rx - lx) + 1;
                int x1 = lx < rx ? lx : rx;
                int x2 = x1 + w - 1;

                if (clipLine({ 0,0,pb.width(), pb.height() }, x1, y1, x2, y2))
                {
                    double v = maths::Map(y1, miny, maxy, 0, 1);
                    w = x2 - x1;
                    sampleHLine2D(pb, GeoSpan<int>(x1, y1, w), v, src);
                }
            }

            ldda.x += ldda.dx;
            rdda.x += rdda.dx;

            // Advance y position.  Exit if run off its bottom
            y += 1;
            if (y >= clipRect.y() + clipRect.h())
            {
                break;
            }
        }
    }
    */

    INLINE void sampleTriangle(PixelView& pb, const int x1, const int y1,
        const int x2, const int y2,
        const int x3, const int y3,
        ISample2D<PixelRGBA>& src,
        const PixelRect& clipRect)
    {
        // Create a triangle object
        PixelTriangle tri(x1, y1, x2, y2, x3, y3);

        // find topmost vertex of the polygon
        int nverts = 3;
        int vmin = 0;

        //sampleConvexPolygon(pb, tri.verts, nverts, vmin, src, clipRect);
    }

    INLINE void sampleCircle(PixelView& pmap, const int centerX, const int centerY, const int radius, ISample2D<PixelRGBA>& fillStyle)
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
                    auto u = maths::Map(x, x1, x2, 0, 1);
                    auto v = maths::Map(y, y1, y2, 0, 1);
                    auto rgb = fillStyle.getValue(u, v);
                    pmap.set(x, y, rgb);
                }
            }
        }
    }

}
// namespace vdj
