#pragma once

#include "apidefs.h"
//#include "vdjview.hpp"
#include "agg/agg_rendering_buffer.h"
#include "agg/agg_renderer_base.h"
#include "agg/agg_pixfmt_rgba.h"

#include <memory>
#include <algorithm>


namespace alib {
    using Real = float;

    struct Draw2DState 
    {
        Real fStrokeWidth = 1;
        PixelRGBA fStrokePixel{};
        PixelRGBA fFillPixel{};
    };

	struct Draw2DContext
	{
        agg::row_accessor<uint8_t> fView;
        Draw2DState fDrawState;

        Draw2DContext()
        {}

        Draw2DContext(uint8_t *data, size_t awidth, size_t aheight, ptrdiff_t stride)
            : fView(data, awidth, aheight, stride)
        {}

        void attach(uint8_t* data, size_t awidth, size_t aheight, ptrdiff_t stride)
        {
            fView.attach(data, awidth, aheight, stride);
        }

        // Attribute state to be set
        void setStrokeWidth(Real sw);
        void setStrokePixel(const PixelRGBA &c);
        void setFillPixel(const PixelRGBA& c);

        // clear() - set all pixels to a specified value
        void clear(const PixelRGBA& c);

        void copyHLine(const Real x, const Real y, const Real l, const PixelRGBA& c);
        void blendHLine(const Real x, const Real y, const Real l, const PixelRGBA& c);
        //void hLine(const Real x, const Real y, const Real l, const PixelRGBA& c);
        //void hLine(const GeoSpan<Real>& s, const PixelRGBA& c);

        
        void copyVLine(const Real x, const Real y, const Real l, const PixelRGBA& c);
        void copyVLine(const GeoSpan<Real>& s, const PixelRGBA& c);

        void line(Real x1, Real y1, Real x2, Real y2, const PixelRGBA& color, Real swidth = 1);
        void line(const Point<Real>& p1, const Point<Real>& p2, const PixelRGBA& c, Real swidth = 1);

        void strokePolygon(const GeoPolygon<Real>& poly, const PixelRGBA& c, Real swidth = 1);
        void fillPolygon(const GeoPolygon<Real>& poly, const PixelRGBA& c);

        void strokeRectangle(const Real x, const Real y, const Real w, const Real h, const PixelRGBA& c);
        void strokeRectangle(const GeoRect<Real>& r, const PixelRGBA& c);
        void fillRectangle(const Real x, const Real y, const Real w, const Real h, const PixelRGBA& c);
        void fillRectangle(const GeoRect<Real>& r, const PixelRGBA& c);
        void fillRectangle(const PixelRect& dstisect, const RectD& srcExt, ISample2D<PixelRGBA>& src);

        void strokeEllipse(const Real cx, const Real cy, const Real xradius, Real yradius, const PixelRGBA& color);
        void fillEllipse(Real centerx, Real centery, Real xRadius, Real yRadius, const PixelRGBA& c);
        void fillCircle(Real centerX, Real centerY, Real radius, const PixelRGBA& c);

        void strokeCubicBezier(const GeoCubicBezier<Real>& bez, size_t segments, const PixelRGBA& c);
        void sampledBezier(const PixelBezier& bez, const int segments, ISample1D<PixelRGBA>& c)
        {
            // Get starting point
            auto lp = bez.eval(0);

            int i = 1;
            while (i <= segments) {
                double u = (double)i / segments;

                auto p = bez.eval(u);

                // draw line segment from last point to current point
                line(lp.x(), lp.y(), p.x(), p.y(), c.getValue(u));

                // Assign current to last
                lp = p;

                i = i + 1;
            }
        }
        void sampleRect(const PixelRect& dstisect, const RectD& srcExt, ISample2D<PixelRGBA>& src);
        
        void blit(const Real x, const Real y, agg::rendering_buffer& src);

	};


    // Setting attribute state
    INLINE void Draw2DContext::setStrokeWidth(Real sw) { fDrawState.fStrokeWidth = sw; }
    INLINE void Draw2DContext::setStrokePixel(const PixelRGBA& c) { fDrawState.fStrokePixel = c; }
    INLINE void Draw2DContext::setFillPixel(const PixelRGBA& c) { fDrawState.fFillPixel = c; }
    
    // Drawing
    INLINE void Draw2DContext::clear(const PixelRGBA& c)
    {
        agg::pixfmt_bgra32 pixf(fView);
        agg::renderer_base<agg::pixfmt_bgra32> rbase(pixf);

        rbase.fill(c);
    }

    INLINE void Draw2DContext::copyHLine(const Real x_, const Real y_, const Real w_, const PixelRGBA& c)
    {
        // quick reject on y-axis
        if ((y_ < 0) || (y_ > fView.height()-1))
            return;

        Real y = y_;
        Real x = x_ < 0 ? 0 : x_;
        Real w = x + w_ > fView.width() ? fView.width() - x : w_;
        
        agg::pixfmt_bgra32 pixf(fView);
        pixf.copy_hline(x, y, w, c);
 
    }

    INLINE void Draw2DContext::blendHLine(const Real x_, const Real y_, const Real w_, const agg::rgba8& c)
    {
        // quick reject on y-axis
        if ((y_ < 0) || (y_ > fView.height() - 1))
            return;

        agg::pixfmt_bgra32 pixf(fView);

        Real y = y_;
        Real x = x_ < 0 ? 0 : x_;
        Real w = x + w_ > fView.width() ? fView.width() - x : w_;

        pixf.blend_hline(x, y, w, agg::rgba8(), 0);
    }


    INLINE void Draw2DContext::copyVLine(const Real x, const Real y, const Real l, const PixelRGBA& c)
    {
        // quick reject if out of range
        if (x < 0 || x >= fView.width())
            return;

        agg::pixfmt_bgra32 pixf(fView);
        pixf.copy_vline(x, y, l, c);
    }

    INLINE void Draw2DContext::copyVLine(const GeoSpan<Real>& s, const PixelRGBA& c)
    {
        copyVLine(s.x(), s.y(), s.w(), c);
    }

    INLINE
    void Draw2DContext::line(Real sx1, Real sy1, Real sx2, Real sy2, const PixelRGBA& color, Real swidth)
    {
        agg::pixfmt_bgra32 pixf(fView);
        
        const ptrdiff_t w = fView.width() - 1;
        const ptrdiff_t h = fView.height() - 1;

        ptrdiff_t x1 = alib::Round(sx1);
        ptrdiff_t y1 = alib::Round(sy1);
        ptrdiff_t x2 = alib::Round(sx2);
        ptrdiff_t y2 = alib::Round(sy2);

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

                agg::pixfmt_bgra32 pixf(fView);
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
        ptrdiff_t y0 = y1 - swidth / 2.0;


        for (ptrdiff_t x = x1; x <= x2; x++)
        {
            for (size_t i = 0; i < swidth; ++i)
            {
                ptrdiff_t y = y0 + i;
                if (y >= 0)
                {
                    if (inverse)
                    {
                        if (y < w)
                            pixf.copy_pixel(y, x, color);
                    }
                    else
                    {
                        if (y < h)
                            pixf.copy_pixel(x, y, color);
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

    INLINE void Draw2DContext::line(const alib::Point<Real>& p1, const alib::Point<Real>& p2, const PixelRGBA& c, Real swidth)
    {
        line(p1.x(), p1.y(), p2.x(), p2.y(), c, swidth);
    }

    INLINE void Draw2DContext::strokePolygon(const GeoPolygon<Real>& poly, const PixelRGBA& c, Real swidth)
    {
        typedef Point<Real> Point;

        for (size_t i = 0; i < poly.fVertices.size(); ++i)
        {
            const Point& p1 = (i ? poly.fVertices[i - 1] : poly.fVertices.back()), p2 = poly.fVertices[i];
            line(p1.x(), p1.y(), p2.x(), p2.y(), c, swidth);
        }
    }

    INLINE void Draw2DContext::fillPolygon(const GeoPolygon<Real>& poly, const PixelRGBA& c)
    {
        for (ptrdiff_t y = poly.fTop; y < poly.fBottom; ++y)
        {
            std::vector<ptrdiff_t> intersections;
            for (size_t i = 0; i < poly.fVertices.size(); ++i)
            {
                const Point<Real>& p0 = (i ? poly.fVertices[i - 1] : poly.fVertices.back()), p1 = poly.fVertices[i];
                if ((y >= p0.y() && y < p1.y()) || (y >= p1.y() && y < p0.y()))
                    intersections.push_back(p0.x() + (y - p0.y()) * (p1.x() - p0.x()) / (p1.y() - p0.y()));
            }
            assert(intersections.size() % 2 == 0);
            std::sort(intersections.begin(), intersections.end());
            for (size_t i = 0; i < intersections.size(); i += 2)
            {
                Real left = std::max<Real>(0, intersections[i + 0]);
                Real right = std::min<Real>(fView.width(), intersections[i + 1]);

                copyHLine(left, y, right - left, c);
            }
        }

    }

    // Draw the outline of a rectangle
    INLINE void Draw2DContext::strokeRectangle(const Real x, const Real y, const Real w, const Real h, const PixelRGBA& c)
    {
        // draw top and bottom
        line(x, y, x + w - 1, y, c);
        line(x, y + h - 1, x + w - 1, y + h - 1, c);

        // draw sides
        line(x, y, x, y + h - 1, c);
        line(x + w - 1, y, x + w - 1, y + h - 1, c);
    }

    INLINE void Draw2DContext::strokeRectangle(const GeoRect<Real>& r, const PixelRGBA& c)
    {
        strokeRectangle(r.x(), r.y(), r.w(), r.h(), c);
    }

    // fill the inside of a rectangle
    INLINE void Draw2DContext::fillRectangle(const Real x, const Real y, const Real w, const Real h, const PixelRGBA& c)
    {
        // We calculate clip area up front
        // so we don't have to do clipLine for every single line
        GeoRect<Real> pmapRect(0, 0, fView.width() - 1, fView.height() - 1);
        GeoRect<Real> dstRect = pmapRect.intersection(GeoRect<Real>{ x,y,w,h });

        // If the rectangle is outside the frame of the pixel map
        // there's nothing to be drawn
        if (dstRect.isEmpty())
            return;

        // Do a line by line draw
        agg::pixfmt_bgra32 pixf(fView);
 
        for (ptrdiff_t row = dstRect.top(); row < dstRect.bottom(); ++row)
        {
            copyHLine(dstRect.left(), row, dstRect.w(), c);
        }

    }

    INLINE void Draw2DContext::fillRectangle(const GeoRect<Real>& r, const PixelRGBA& c)
    {
        fillRectangle(r.x(), r.y(), r.w(), r.h(), c);
    }

    //
// fill in a rectangle using the specified 
// 2D sampler.  
// Here we assume clipping has already occured
// and the srcExt is already calculated  to capture
// the desired section of the sampler
    INLINE void Draw2DContext::fillRectangle(const PixelRect& dstisect, const RectD& srcExt, ISample2D<PixelRGBA>& src)
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
        agg::pixfmt_bgra32 pixf(fView);

        for (ptrdiff_t row = dstisect.y(); row < dstisect.y() + dstisect.h() - 1; row++)
        {
            for (ptrdiff_t col = dstisect.left(); col < dstisect.left() + dstisect.w() - 1; col++)
            {
                auto c = src.getValue(u, v);
                pixf.blend_pixel(col, row, c, 255);
                u += uadv;
            }
            v += vadv;
            u = srcExt.left();
        }
    }

    INLINE void Draw2DContext::strokeEllipse(const Real cx, const Real cy, const Real xradius, Real yradius, const PixelRGBA& color)
    {
        //raster_ellipse(pb, cx, cy, xradius, yradius, color, Plot4EllipsePoints);
    }

    INLINE void Draw2DContext::fillEllipse(Real centerx, Real centery, Real xRadius, Real yRadius, const PixelRGBA& c)
    {
        static const int nverts = 72;
        int steps = nverts;
        GeoPolygon<Real> poly;

        int awidth = xRadius * 2;
        int aheight = yRadius * 2;

        for (int i = 0; i < steps; i++) {
            auto u = (double)i / steps;
            auto angle = u * (2 * alib::Pi);

            int x = (int)alib::Floor((awidth / 2.0) * cos(angle));
            int y = (int)alib::Floor((aheight / 2.0) * sin(angle));
            poly.addPoint(Point<Real>(x + centerx, y + centery));
        }
        poly.findTopmost();
        fillPolygon(poly, c);
    }

    // filling a circle with a fixed color
    INLINE void Draw2DContext::fillCircle(Real centerX, Real centerY, Real radius, const PixelRGBA& c)
    {
        fillEllipse(centerX, centerY, radius, radius, c);
    }


    INLINE void Draw2DContext::strokeCubicBezier(const GeoCubicBezier<Real>& bez, size_t segments, const PixelRGBA& c)
    {
        // Get starting point
        auto lp = bez.eval(0);

        int i = 1;
        double uadv = 1.0 / segments;
        double u = uadv;

        while (i <= segments) {
            auto p = bez.eval(u);

            // draw line segment from last point to current point
            line(lp.x(), lp.y(), p.x(), p.y(), c);

            // Assign current to last
            lp = p;

            i = i + 1;
            u += uadv;
        }
    }

    INLINE void Draw2DContext::blit(const Real x, const Real y, agg::rendering_buffer & src)
    {
        PixelRect bounds(0, 0, fView.width(), fView.height());
        PixelRect dstFrame(x, y, src.width(), src.height());

        // Intersection of boundary and destination frame
        PixelRect dstisect = bounds.intersection(dstFrame);

        if (dstisect.isEmpty())
            return;

        int dstX = dstisect.x();
        int dstY = dstisect.y();

        int srcX = dstX - x;
        int srcY = dstY - y;

        agg::pixfmt_bgra32 pixf(fView);
        agg::renderer_base<agg::pixfmt_bgra32> rbase(pixf);

        rbase.blend_from(rbase, srcRect, dstX, dstY, 255);

    }

    //
// fill in a rectangle using the specified 
// 2D sampler.  
// Here we assume clipping has already occured
// and the srcExt is already calculated  to capture
// the desired section of the sampler
    INLINE void Draw2DContext::sampleRect(const PixelRect& dstisect, const RectD& srcExt, ISample2D<PixelRGBA>& src)
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
        
        agg::pixfmt_bgra32 pixf(fView);


        for (ptrdiff_t row = dstisect.y(); row < dstisect.y() + dstisect.h() - 1; row++)
        {
            for (ptrdiff_t col = dstisect.left(); col < dstisect.left() + dstisect.w() - 1; col++)
            {
                auto c = src.getValue(u, v);
                pixf.blend_pixel(col, row, c, 255);

                u += uadv;
            }
            v += vadv;
            u = srcExt.left();
        }
    }
}
