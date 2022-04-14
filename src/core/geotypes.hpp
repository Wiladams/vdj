#pragma once

#include "vec.h"
#include "vdjPoint.hpp"

#include <vector>

namespace alib 
{
    // Axis aligned bounding box
    template <typename T>
    struct AABB
    {
        T left;
        T top;
        T right;
        T bottom;
    };

    template <typename T>
    struct Geometry
    {
        virtual bool contains(const T& x, const T& y) const { return false; }
        //virtual GeoRect<T> getBoundingBox() const
    };

    // A starting of a line and a length
    template <typename T>
    struct GeoSpan
    {
    public:
        T fX;
        T fY;
        T fW;

    public:
        INLINE GeoSpan() noexcept = default;
        INLINE constexpr GeoSpan(const GeoSpan<T>& other) noexcept = default;
        INLINE GeoSpan(T x, T y, T w) noexcept :fX(x), fY(y), fW(w) {}

        INLINE GeoSpan& operator=(const GeoSpan<T>& other) noexcept = default;

        INLINE constexpr T x() const noexcept { return fX; }
        INLINE constexpr T y() const noexcept { return fY; }
        INLINE constexpr T w() const noexcept { return fW; }

        INLINE constexpr T rightMost() const noexcept { return fX + fW - 1; }
    };

    // Representation of a line segment
    template <typename T>
    struct GeoLine
    {
        Point<T> pt1;
        Point<T> pt2;
    };

    // General rectangle representation
    //
    template <typename T>
    struct GeoRect : public Geometry<T>
    {
    public:
        T fX;
        T fY;
        T fWidth;
        T fHeight;

    public:
        GeoRect() = default;
        GeoRect(T x, T y, T w, T h) :fX(x), fY(y), fWidth(w), fHeight(h) {}

        INLINE T constexpr x()const { return fX; }
        INLINE T constexpr y()const { return fY; }
        INLINE T constexpr w()const { return fWidth; }
        INLINE T constexpr h()const { return fHeight; }

        INLINE T constexpr left() const { return fX; }
        INLINE T constexpr top() const { return fY; }
        INLINE T constexpr right() const { return fX + fWidth; }
        INLINE T constexpr bottom() const { return fY + fHeight; }

        INLINE Point<T> center() const { return Point<T>({ fX + fWidth / 2.0, fY + fHeight / 2.0 }); }

        INLINE bool isEmpty() const { return (fWidth <= 0) || (fHeight <= 0); }

        INLINE bool contains(const T& x, const T& y) const override
        {
            if ((x < fX) || (y < fY))
                return false;

            if ((x >= (fX + fWidth)) || (y >= (fY + fHeight)))
                return false;

            return true;
        }

        // See if another rectangle is completely contained within us
        INLINE constexpr bool contains(const GeoRect<T>& other) const
        {
            if (!contains(other.x, other.y))
            {
                return false;
            }

            if (!contains(other.x + other.width - 1, other.y + other.height - 1))
            {
                return false;
            }

            return true;
        }

        INLINE GeoRect<T> intersection(const GeoRect<T>& b) const
        {
            T x = fX > b.x() ? fX : b.x();
            T y = fY > b.y() ? fY : b.y();
            T right = ((fX + w()) < (b.fX + b.w())) ? (fX + w()) : (b.fX + b.w());
            T bottom = ((fY + h()) < (b.fY + b.h())) ? (fY + h()) : (b.fY + b.h());

            T width = ((right - x) > 0) ? (right - x) : 0;
            T height = ((bottom - y) > 0) ? (bottom - y) : 0;

            return GeoRect<T>(x, y, width, height);
        }

        INLINE GeoRect<T>& operator=(const GeoRect<T>& rhs) noexcept
        {
            fX = rhs.fX;
            fY = rhs.fY;
            fWidth = rhs.fWidth;
            fHeight = rhs.fHeight;

            return *this;
        };

        // The arithmetic operators are good for doing
        // interpolation
        GeoRect<T>& operator+=(const GeoRect<T>& other)
        {
            fX += other.fX;
            fY += other.fY;
            fWidth += other.fWidth;
            fHeight += other.fHeight;

            return *this;
        }

        GeoRect<T>& operator *= (double s)
        {
            fX *= s;
            fY *= s;
            fWidth *= s;
            fHeight *= s;

            return *this;
        }

        GeoRect<T> operator+(const GeoRect<T>& rhs);
        GeoRect<T> operator * (double s) const;
    };

    // GeoRect<T> Implementation
    //
    template <typename T>
    GeoRect<T> GeoRect<T>::operator *(double s) const
    {
        GeoRect<T> res(*this);
        return res *= s;
    }

    template <typename T>
    GeoRect<T> GeoRect<T>::operator+(const GeoRect<T>& rhs)
    {
        GeoRect<T> res(*this);
        return res += rhs;
    }

    // GeoEllipse Definition
    template <typename T>
    struct GeoEllipse : public Geometry<T>
    {
        T cx, cy;   // center of ellipse
        T rx;       // radius in x axis
        T ry;       // radius in y axis
    };

    template <typename T>
    struct GeoCircle : public Geometry<T>
    {
        T fX, fY;
        T fR;

        GeoRect<T> getBoundingBox() override
        {
            auto x1 = fX - fR, y1 = fY - fR;
            auto w = fR * 2;

            return GeoRect<T>(x1, y1, w, w);
        }

        T cx() { return fX; }
        T cy() { return fY; }
        T r() { return fR; }
    };

    


    template <typename T>
    struct GeoPolygon
    {
        ptrdiff_t fTop = 65535;
        ptrdiff_t fBottom = 0;
        bool fIsClosed = true;
        std::vector<alib::Point<T> > fVertices;

        GeoPolygon(bool closed = false)
            :fTop(65535)
            , fBottom(0)
            , fIsClosed(closed)
        {
            clear();
        }

        bool isClosed() { return fIsClosed; }
        bool isEmpty() { return fVertices.size() == 0; }

        void setClose(bool toClose) { fIsClosed = toClose; }

        // clear out existing commands and vertices
        void clear()
        {
            fVertices.clear();
            fTop = 65535;
            fBottom = 0;
        }

        GeoRect<T> getBounds()
        {
            T minx = 65535;
            T maxx = -65535;
            T miny = 65535;
            T maxy = -65535;

            for (auto& pt : fVertices)
            {
                minx = alib::Min(pt.x(), minx);
                maxx = alib::Max(pt.x(), maxx);
                miny = alib::Min(pt.y(), miny);
                maxy = alib::Max(pt.y(), maxy);
            }

            return GeoRect<T>(minx, miny, maxx - minx, maxy - miny);
        }

        void addPoint(const Point<T>& pt)
        {
            fVertices.push_back(pt);
        }

        void findTopmost()
        {
            fTop = 65535;
            fBottom = 0;

            for (size_t i = 0; i < fVertices.size(); ++i)
            {
                fTop = fTop < fVertices[i].y() ? fTop : fVertices[i].y();
                fBottom = fBottom > fVertices[i].y() ? fBottom : fVertices[i].y();
                //fTop = std::min(fTop, fVertices[i].y());
                //fBottom = std::max(fBottom, fVertices[i].y());
            }
            fTop = std::max<ptrdiff_t>(0, fTop);
            fBottom = std::min<ptrdiff_t>(fBottom, 65535);    
        }
    };

    /*
      GeoTriangle

      A triangle is a specialization of a polygon
    */
    template <typename T>
    struct GeoTriangle : public GeoPolygon<T>
    {

        // Basic constructor take coordinates in any order, sorts them
        // so that subsequent drawing will be faster.
        GeoTriangle(const T x1, const T y1,
            const T x2, const T y2,
            const T x3, const T y3)
            :GeoPolygon<T>(true)
        {
            GeoPolygon<T>::addPoint(Point<T>(x1, y1));
            GeoPolygon<T>::addPoint(Point<T>(x2, y2));
            GeoPolygon<T>::addPoint(Point<T>(x3, y3));

            GeoPolygon<T>::findTopmost();
        }
    };

    // Bezier Reference
    // https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/Bezier/bezier-construct.html
    // https://pomax.github.io/bezierinfo/#tracing
    //
    // Quadratic Bezier, defined by 3 points
    template <typename T>
    struct GeoQuadraticBezier
    {
        Point<T> p1;
        Point<T> c1;
        Point<T> p2;

    public:
        GeoQuadraticBezier(const Point<T>& pp1, const Point<T>& pp2, const Point<T>& pp3)
            :p1(pp1), c1(pp2), p2(pp3)
        {}

        GeoQuadraticBezier(const T x1, const T y1, const T c1x, const T c1y, const T x3, const T y3)
            :p1(x1, y1)
            , c1(c1x, c1y)
            , p2(x3, y3)
        {}

        // Value of curve at parametric position 'u'
        INLINE Point<T> eval(const double u) const
        {
            double oneminusu = 1.0 - u;
            return c1 + ((oneminusu * oneminusu) * (p1 - c1)) + ((u * u) * (p2 - c1));
        }
    };

    // Cubic Bezier, defined by 4 points
    template <typename T>
    struct GeoCubicBezier
    {
        Point<T> p1;
        Point<T> p2;
        Point<T> p3;
        Point<T> p4;

    public:
        std::vector<T> tvals;

    public:
        GeoCubicBezier(const Point<T>& pp1, const Point<T>& pp2, const Point<T>& pp3, const Point<T>& pp4)
            :p1(pp1), p2(pp2), p3(pp3), p4(pp4)
        {}

        GeoCubicBezier(const T x1, const T y1, const T x2, const T y2, const T x3, const T y3, const T x4, const T y4)
            :p1({ x1, y1 })
            , p2({ x2, y2 })
            , p3({ x3, y3 })
            , p4({ x4, y4 })

        {}

        // generate a list of 't' values that will give 
        // equal distance
        INLINE double findUForX(ptrdiff_t x) const
        {
            double highEnd = 1.0;
            double lowEnd = 0.0;

            // Binary search to find the solution
            while (true)
            {
                double u = highEnd - ((highEnd - lowEnd) / 2.0);

                ptrdiff_t evalX = eval(u).fX;

                if (evalX == x)
                    return u;

                if (evalX > x)
                {
                    highEnd = u;
                }
                else if (evalX < x)
                {
                    lowEnd = u;
                }
            }
        }

        INLINE void calcSpeeds()
        {
            // Figure out lowest value for t==0
            // Figure out highest value for t == 1.0

            auto p1 = eval(0);
            auto p2 = eval(1.0);

            tvals.empty();

            // do the loop searching for best fits
            for (size_t x = p1.x(); x <= p2.x(); x++)
            {
                auto u = findUForX(x);

                tvals.push_back(u);
            }
        }

        
        // Value of curve at parametric position 'u'
        // control points are P0, P1, P2, P3
        INLINE Point<T> eval(const double u) const
        {
            double oneminusu = 1 - u;
            double BEZ03 = oneminusu * oneminusu * oneminusu;	// (1-u)^3
            double BEZ13 = 3 * u * (oneminusu * oneminusu);	    // 3u(1-u)^2
            double BEZ23 = 3 * (u * u) * oneminusu;				// 3u^2(1-u)
            double BEZ33 = u * u * u;							// u^3

            return ((p1 * BEZ03) + (p2 * BEZ13) + (p3 * BEZ23) + (p4 * BEZ33));
        }

    };


}
// alib namespace