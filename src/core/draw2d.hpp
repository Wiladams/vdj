#pragma once

/*
    This is a simple drawing library that has functions ranging from
    setting a single pixel all the way up to convex polygon, triangle and ellipse.

    In general, the higher level functions will do bounds checking
    and deal with clipping primitives appropriately.

    The functions work against the PixelMap class.  They are not 
    the fastest routines possible, but they should be fairly complete.

    All operations are SRCCOPY.  It is relatively easy to add different
    pixel covering functions with these base routines.

    Reference
    https://magcius.github.io/xplain/article/rast1.html
    https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/rasterization-stage
    https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation?url=3d-basic-rendering%2Frasterization-practical-implementation
//

*/

#include "apphost.h"
#include "vdjview.hpp"
#include "maths.hpp"
#include "sampler.hpp"

#include <memory>
#include <algorithm>

using namespace maths;



namespace vdj 
{

    INLINE void setSpan(PixelView& pmap, const int x, const int y, const int w, const PixelRGBA& c)
    {
        pmap.setSpan(x, y, w, c);
    }
    
    INLINE void setSpan(PixelView& pmap, const PixelSpan& s, const PixelRGBA& c)
    {
        pmap.setSpan(s.x(), s.y(), s.w(), c);
    }

 
    //
    // line()
    // Stroke a line using the current stroking pixel.
    // Uses Bresenham line drawing.
    // clips line to frame of pixelmap
    //
    INLINE void line(PixelView& pmap, ptrdiff_t x1, ptrdiff_t y1, ptrdiff_t x2, ptrdiff_t y2, const PixelRGBA& color, size_t width = 1)
    {
        //assert(canvas.PixelSize() == sizeof(Color));

        const ptrdiff_t w = pmap.width() - 1;
        const ptrdiff_t h = pmap.height() - 1;

        // Figure out if we need to do some clipping, or the line is completely outside
        if (x1 < 0 || y1 < 0 || x1 > w || y1 > h || x2 < 0 || y2 < 0 || x2 > w || y2 > h)
        {
            // early reject
            if ((x1 < 0 && x2 < 0) || (y1 < 0 && y2 < 0) || (x1 > w && x2 > w) || (y1 > h && y2 > h))
                return;

            
            if (y1 == y2)       // horizontal line
            {
                x1 = std::min<ptrdiff_t>(std::max<ptrdiff_t>(x1, 0), w);
                x2 = std::min<ptrdiff_t>(std::max<ptrdiff_t>(x2, 0), w);
            }
            else if (x1 == x2)  // vertical line
            {
                y1 = std::min<ptrdiff_t>(std::max<ptrdiff_t>(y1, 0), h);
                y2 = std::min<ptrdiff_t>(std::max<ptrdiff_t>(y2, 0), h);
            }
            else                // Just a regular line
            {
                // orient things in the most favorable way
                ptrdiff_t x0 = (x1 * y2 - y1 * x2) / (y2 - y1);
                ptrdiff_t y0 = (y1 * x2 - x1 * y2) / (x2 - x1);
                ptrdiff_t xh = (x1 * y2 - y1 * x2 + h * (x2 - x1)) / (y2 - y1);
                ptrdiff_t yw = (y1 * x2 - x1 * y2 + w * (y2 - y1)) / (x2 - x1);

                if (x1 < 0)
                {
                    x1 = 0;
                    y1 = y0;
                }
                if (x2 < 0)
                {
                    x2 = 0;
                    y2 = y0;
                }
                if (x1 > w)
                {
                    x1 = w;
                    y1 = yw;
                }
                if (x2 > w)
                {
                    x2 = w;
                    y2 = yw;
                }
                if ((y1 < 0 && y2 < 0) || (y1 > h && y2 > h))
                    return;

                if (y1 < 0)
                {
                    x1 = x0;
                    y1 = 0;
                }
                if (y2 < 0)
                {
                    x2 = x0;
                    y2 = 0;
                }

                if (y1 > h)
                {
                    x1 = xh;
                    y1 = h;
                }
                if (y2 > h)
                {
                    x2 = xh;
                    y2 = h;
                }
            }
        }

        const bool inverse = std::abs(y2 - y1) > std::abs(x2 - x1);
        if (inverse)
        {
            std::swap(x1, y1);
            std::swap(x2, y2);
        }

        if (x1 > x2)
        {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }

        const double dx = double(x2 - x1);
        const double dy = (double)std::abs(y2 - y1);

        double error = dx / 2.0f;
        const ptrdiff_t ystep = (y1 < y2) ? 1 : -1;
        ptrdiff_t y0 = y1 - width / 2;

        for (ptrdiff_t x = x1; x <= x2; x++)
        {
            for (size_t i = 0; i < width; ++i)
            {
                ptrdiff_t y = y0 + i;
                if (y >= 0)
                {
                    if (inverse)
                    {
                        if (y < w) {
                            //At<A, Color>(canvas, y, x) = color;
                            pmap.At(y, x) = color;
                        }

                    }
                    else
                    {
                        if (y < h) {
                            //At<A, Color>(canvas, x, y) = color;
                            pmap.At(x, y) = color;
                        }

                    }
                }

            }

            error -= dy;
            if (error < 0)
            {
                y0 += ystep;
                error += dx;
            }
        }
    }

    INLINE void line(PixelView& pmap, const vdj::Point<ptrdiff_t> &p1, const vdj::Point<ptrdiff_t>& p2, const PixelRGBA& c, size_t width = 1)
    {
        line(pmap, p1.x(), p1.y(), p2.x(), p2.y(), c, width);
    }




    //
    // Polygons, and triangles
    // Triangles
    //
    INLINE void strokePolygon(PixelView& pb, const GeoPolygon<ptrdiff_t>& poly, const PixelRGBA& c, size_t width=1)
    {
        typedef vdj::Point<ptrdiff_t> Point;

        for (size_t i = 0; i < poly.fVertices.size(); ++i)
        {
            const Point& p1 = (i ? poly.fVertices[i - 1] : poly.fVertices.back()), p2 = poly.fVertices[i];
            line(pb, p1, p2, c, width);
        }
    }

    INLINE void fillPolygon(PixelView& pb, const GeoPolygon<ptrdiff_t>& poly, const PixelRGBA& c)
    {
        for (ptrdiff_t y = poly.fTop; y < poly.fBottom; ++y)
        {
            std::vector<ptrdiff_t> intersections;
            for (size_t i = 0; i < poly.fVertices.size(); ++i)
            {
                const vdj::Point<ptrdiff_t>& p0 = (i ? poly.fVertices[i - 1] : poly.fVertices.back()), p1 = poly.fVertices[i];
                if ((y >= p0.y() && y < p1.y()) || (y >= p1.y() && y < p0.y()))
                    intersections.push_back(p0.x() + (y - p0.y()) * (p1.x() - p0.x()) / (p1.y() - p0.y()));
            }
            assert(intersections.size() % 2 == 0);
            std::sort(intersections.begin(), intersections.end());
            for (size_t i = 0; i < intersections.size(); i += 2)
            {
                ptrdiff_t left = std::max<ptrdiff_t>(0, intersections[i + 0]);
                ptrdiff_t right = std::min<ptrdiff_t>(pb.width(), intersections[i + 1]);
                //PixelRGBA* dst = &At<A, PixelRGBA>(canvas, 0, y);
                //PixelRGBA* dst = &pb.At(0, y);

                pb.setSpan(left, y, right - left, c);
            }
        }

    }

    // Draw the outline of a rectangle
    INLINE void strokeRectangle(PixelView& pmap, const int x, const int y, const int w, const int h, const PixelRGBA& c)
    {
        // draw top and bottom
        line(pmap, x, y, x + w - 1, y, c);
        line(pmap, x, y + h - 1, x + w - 1, y + h - 1, c);

        // draw sides
        line(pmap, x, y, x, y + h - 1, c);
        line(pmap, x + w - 1, y, x + w - 1, y + h - 1, c);
    }

    // fill the inside of a rectangle
    INLINE void fillRectangle(PixelView& pmap, const int x, const int y, const int w, const int h, const PixelRGBA &c)
    {
        // We calculate clip area up front
        // so we don't have to do clipLine for every single line
        PixelRect dstRect = pmap.frame().intersection({ x,y,w,h });

        // If the rectangle is outside the frame of the pixel map
        // there's nothing to be drawn
        if (dstRect.isEmpty())
            return;

        // Do a line by line draw
        for (int row = dstRect.y(); row < dstRect.y() + dstRect.h(); row++)
        {
            //setSpan(pmap, dstRect.x(), row, dstRect.w(), c);
            pmap.setSpan(dstRect.x(), row, dstRect.w(), c);
        }
    }

    INLINE void fillRectangle(PixelView& pmap, const PixelRect& r, const PixelRGBA& c)
    {
        fillRectangle(pmap, r.x(), r.y(), r.w(), r.h(), c);

    }

    INLINE void strokeEllipse(PixelView& pb, const int cx, const int cy, const size_t xradius, size_t yradius, const PixelRGBA& color)
    {
        //raster_ellipse(pb, cx, cy, xradius, yradius, color, Plot4EllipsePoints);
    }

    // A rather simple ellipse drawing routine
    // not the most performant, but uses the 
    // already available polygon filling routine
    INLINE void fillEllipse(PixelView& pmap, int centerx, int centery, int xRadius, int yRadius, const PixelRGBA &c)
    {
        static const int nverts = 72;
        int steps = nverts;
        GeoPolygon<ptrdiff_t> poly;

        int awidth = xRadius * 2;
        int aheight = yRadius * 2;

        for (int i = 0; i < steps; i++) {
            auto u = (double)i / steps;
            auto angle = u * (2 * maths::Pi);

            int x = (int)Floor((awidth / 2.0) * cos(angle));
            int y = (int)Floor((aheight / 2.0) * sin(angle));
            poly.addPoint(Point<ptrdiff_t>(x + centerx, y + centery));
        }
        poly.findTopmost();
        fillPolygon(*gAppSurface, poly, c);
    }

    // filling a circle with a fixed color
    INLINE void fillCircle(PixelView& pmap, int centerX, int centerY, int radius, const PixelRGBA& c)
    {
        fillEllipse(pmap, centerX, centerY, radius, radius, c);
    }


    INLINE void strokeCubicBezier(PixelView& pmap, const PixelBezier &bez, int segments, const PixelRGBA &c)
    {
        // Get starting point
        auto lp = bez.eval(0);

        int i = 1;
        double uadv = 1.0 / segments;
        double u = uadv;

        while (i <= segments) {
            auto p = bez.eval(u);

            // draw line segment from last point to current point
            line(pmap, lp.x(), lp.y(), p.x(), p.y(), c);

            // Assign current to last
            lp = p;

            i = i + 1;
            u += uadv;
        }
    }


    // We can create a generalized form of blit which can take
    // a sampler instead of a pixelmap.  Then, it will be possible
    // to easily handle scaling, as well as a wealth of different
    // kinds of images.
    // blit is an optimization to deal with fixed sizes and fixed
    // images.

    // This is a straight up pixel copy
    // no scaling, no alpha blending
    // it will deal with clipping so we don't
    // crash when going off the edges
    INLINE void blit(PixelView& pb, const int x, const int y, PixelView& src)
    {
        PixelRect bounds(0, 0, pb.width(), pb.height());
        PixelRect dstFrame(x, y, src.width(), src.height());

        // Intersection of boundary and destination frame
        PixelRect dstisect = bounds.intersection(dstFrame);

        if (dstisect.isEmpty())
            return;

        int dstX = dstisect.x();
        int dstY = dstisect.y();

        int srcX = dstX - x;
        int srcY = dstY - y;

        // we're trying to avoid knowing the internal details of the
        // pixel maps, so we use getPixelPointer() to get a pointer
        // realistically, the blit should be implemented in PixelMap
        uint32_t* dstPtr = (uint32_t*)pb.getPixelPointer(dstX, dstY);

        int rowCount = 0;
        for (int srcrow = srcY; srcrow < srcY + dstisect.h(); srcrow++)
        {
            uint32_t* srcPtr = (uint32_t*)src.getPixelPointer(srcX, srcrow);
            memcpy(dstPtr, srcPtr, dstisect.w() * 4);
            rowCount++;
            dstPtr = (uint32_t*)pb.getPixelPointer(dstisect.x(), dstisect.y() + rowCount);
        }
    }


}
