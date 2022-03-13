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
#include "maths.hpp"
#include "sampler.hpp"
#include "vdjview.hpp"

#include <memory>
#include <algorithm>

using namespace maths;


namespace vdj 
{

    INLINE void setSpan(PixelView &pmap, const size_t x, const size_t y, const size_t w, const PixelRGBA& c)
    {
        // do the loop

        pmap.setSpan(x, y, w, c);
    }
    
    INLINE void setSpan(PixelView & pmap, const PixelSpan& s, const PixelRGBA& c)
    {
        setSpan(pmap, s.x(), s.y(), s.w(), c);
    }

 
    //
    // line()
    // Stroke a line using Bresenham line drawing.
    // clips line to frame of pixelmap
    //
    INLINE void line(PixelView & pmap, ptrdiff_t x1, ptrdiff_t y1, ptrdiff_t x2, ptrdiff_t y2, const PixelRGBA& color, size_t width = 1)
    {
        //assert(pmap.PixelSize() == sizeof(Color));

        const ptrdiff_t w = pmap.width() - 1;
        const ptrdiff_t h = pmap.height() - 1;

        if (x1 < 0 || y1 < 0 || x1 > w || y1 > h || x2 < 0 || y2 < 0 || x2 > w || y2 > h)
        {
            if ((x1 < 0 && x2 < 0) || (y1 < 0 && y2 < 0) || (x1 > w && x2 > w) || (y1 > h && y2 > h))
                return;

            if (y1 == y2)
            {
                x1 = std::min<ptrdiff_t>(std::max<ptrdiff_t>(x1, 0), w);
                x2 = std::min<ptrdiff_t>(std::max<ptrdiff_t>(x2, 0), w);
            }
            else if (x1 == x2)
            {
                y1 = std::min<ptrdiff_t>(std::max<ptrdiff_t>(y1, 0), h);
                y2 = std::min<ptrdiff_t>(std::max<ptrdiff_t>(y2, 0), h);
            }
            else
            {
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
                        if (y < w)
                            pmap.At<PixelRGBA>(y, x) = color;
                    }
                    else
                    {
                        if (y < h)
                            pmap.At<PixelRGBA>(x, y) = color;
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

    INLINE void line(PixelView & pmap, const vdj::Point<ptrdiff_t> &p1, const vdj::Point<ptrdiff_t>& p2, const PixelRGBA& c, size_t width = 1)
    {
        line(pmap, p1.x(), p1.y(), p2.x(), p2.y(), c, width);
    }

    //
    // Polygons, and triangles
    // Triangles
    //
    INLINE void strokePolygon(PixelView & pb, const GeoPolygon<ptrdiff_t>& poly, const PixelRGBA& c, size_t width=1)
    {
        typedef vdj::Point<ptrdiff_t> Point;

        for (size_t i = 0; i < poly.fVertices.size(); ++i)
        {
            const Point& p1 = (i ? poly.fVertices[i - 1] : poly.fVertices.back()), p2 = poly.fVertices[i];
            line(pb, p1.x(), p1.y(), p2.x(), p2.y(), c, width);
            //aaline(pb, p1.x(), p1.y(), p2.x(), p2.y(), c, width);
        }
    }

    INLINE void fillPolygon(PixelView & pb, const GeoPolygon<ptrdiff_t>& poly, const PixelRGBA& c)
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

                setSpan(pb, left, y, right - left, c);
            }
        }

    }

    // Draw the outline of a rectangle
    INLINE void strokeRectangle(PixelView & pmap, const int x, const int y, const int w, const int h, const PixelRGBA& c)
    {
        // draw top and bottom
        line(pmap, x, y, x + w - 1, y, c);
        line(pmap, x, y + h - 1, x + w - 1, y + h - 1, c);

        // draw sides
        line(pmap, x, y, x, y + h - 1, c);
        line(pmap, x + w - 1, y, x + w - 1, y + h - 1, c);
    }

    // fill the inside of a rectangle
    INLINE void fillRectangle(PixelView & pmap, const int x, const int y, const int w, const int h, const PixelRGBA &c)
    {
        // We calculate clip area up front
        // so we don't have to do clipLine for every single line
        //auto psize = pmap.Size();
        PixelRect pmapRect(0, 0, pmap.width() - 1, pmap.height() - 1);
        PixelRect dstRect = pmapRect.intersection(PixelRect{ x,y,w,h });

        // If the rectangle is outside the frame of the pixel map
        // there's nothing to be drawn
        if (dstRect.isEmpty())
            return;

        // Do a line by line draw
        for (int row = dstRect.top(); row < dstRect.top() + dstRect.h(); row++)
        {
            //setSpan(pmap, dstRect.left(), row, dstRect.w(), c);
            pmap.setSpan(dstRect.x(), row, dstRect.w(), c);
        }
    }

    INLINE void fillRectangle(PixelView & pmap, const PixelRect& r, const PixelRGBA& c)
    {
        fillRectangle(pmap, r.x(), r.y(), r.w(), r.h(), c);

    }

    INLINE void strokeEllipse(PixelView & pb, const int cx, const int cy, const size_t xradius, size_t yradius, const PixelRGBA& color)
    {
        //raster_ellipse(pb, cx, cy, xradius, yradius, color, Plot4EllipsePoints);
    }

    // A rather simple ellipse drawing routine
    // not the most performant, but uses the 
    // already available polygon filling routine
    INLINE void fillEllipse(PixelView & pmap, int centerx, int centery, int xRadius, int yRadius, const PixelRGBA &c)
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
    INLINE void fillCircle(PixelView & pmap, int centerX, int centerY, int radius, const PixelRGBA& c)
    {
        fillEllipse(pmap, centerX, centerY, radius, radius, c);
    }


    INLINE void strokeCubicBezier(PixelView & pmap, const PixelBezier &bez, int segments, const PixelRGBA &c)
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
    INLINE void blit(PixelView & pb, const int x, const int y, PixelView & src)
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
            dstPtr = (uint32_t*)pb.getPixelPointer(dstisect.left(), dstisect.top() + rowCount);
        }
    }


}


/*
// anti-aliased line using Wu algorithm
// https://en.wikipedia.org/wiki/Xiaolin_Wu%27s_line_algorithm
//

    //
    // wu line drawing
    //
    INLINE int iPart(float x)
    {
        return (int)std::floor(x);
    }

    INLINE float fPart(float x)
    {
        return x - std::floor(x);

        //if (x > 0) return x - iPart(x);
        //return x - (iPart(x) + 1);
    }

    INLINE float rfPart(float x)
    {
        return 1.0f - fPart(x);
    }

INLINE void aaline(View& pmap, ptrdiff_t x0, ptrdiff_t y0, ptrdiff_t x1, ptrdiff_t y1, const PixelRGBA& color, size_t width = 1)
{
    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);

    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }

    if (x0 > x1)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    // compute the slope
    float dx = x1 - x0;
    float dy = y1 - y0;
    float gradient = dy / dx;

    if (dx == 0.0f)
        gradient = 1.0f;

    // handle first endpoint
    auto xend = std::round(x0);
    auto yend = y0 + gradient + (xend - x0);
    auto xgap = rfPart(x0 + 0.5);

    int xpxl1 = xend;
    int ypxl1 = iPart(yend);

    if (steep) {
        auto c1 = color * rfPart(yend)*xgap;
        auto c2 = color * fPart(yend) * xgap;
        pmap.set(ypxl1, xpxl1,c1);
        pmap.set(ypxl1 + 1, xpxl1, c2);
    }
    else {
        auto c1 = color * rfPart(yend) * xgap;
        auto c2 = color * fPart(yend) * xgap;
        pmap.set(xpxl1, ypxl1, c1);
        pmap.set(xpxl1, ypxl1 + 1, c2);
    }
    auto intery = yend + gradient;  // first y-intersection for main loop

    // Handle second endpoint
    xend = std::round(x1);
    yend = y1 + gradient * (xend - x1);
    xgap = fPart(x1 + 0.5);
    auto xpxl2 = xend;
    auto ypxl2 = iPart(yend);

    if (steep) {
        auto c1 = color * rfPart(yend) * xgap;
        auto c2 = color * fPart(yend) * xgap;
        pmap.set(ypxl2, xpxl2, c1);
        pmap.set(ypxl2 + 1, xpxl2, c2);
    }
    else {
        auto c1 = color * rfPart(yend) * xgap;
        auto c2 = color * fPart(yend) * xgap;
        pmap.At((ptrdiff_t)xpxl2, ypxl2) = c1;
        pmap.set(xpxl2, ypxl2 + 1, c2);
    }


    // Main loop
    if (steep) {
        for (int x = xpxl1 + 1; x < xpxl2; x++)
        {
            auto c1 = color * rfPart(intery);
            auto c2 = color * fPart(intery);
            pmap.set(iPart(intery), x, c1);
            pmap.set(iPart(intery) + 1, x, c2);
            intery += gradient;
        }
    }
    else {
        for (int x = xpxl1 + 1; x < xpxl2; x++)
        {
            auto c1 = color * rfPart(intery);
            auto c2 = color * fPart(intery);
            pmap.set(x, iPart(intery), c1);
            pmap.set(x, iPart(intery) + 1, c2);
            intery += gradient;
        }
    }

    int xpxl2 = x1;
    float intersectY = y0;

    if (steep) {
        int x;
        for (x = xpxl1; x <= xpxl2; x++)
        {
            auto rf = rfPart(intersectY);
            PixelRGBA pc1 = color * rf;

            pmap.set(iPart(intersectY), x, pc1);

            auto fp = fPart(intersectY);
            PixelRGBA pc2 = color * fp;
            pmap.set(iPart(intersectY) - 1, x, pc2);

            intersectY += gradient;
        }
    } else {
        int x;
        for (x = xpxl1; x <= xpxl2; x++)
        {
            auto rf = rfPart(intersectY);
            auto pc1 = color * rf;
            pc1.setA(rf * 255);
            pmap.set(x, iPart(intersectY), pc1);

            auto fp = fPart(intersectY);
            PixelRGBA pc2 = color * fp;
            pc2.setA(fp * 255);
            pmap.set(x, iPart(intersectY) - 1, pc2);

            intersectY += gradient;
        }
    }

}
*/
