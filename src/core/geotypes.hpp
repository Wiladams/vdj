#pragma once

#include "vec.h"
#include "vdjPoint.hpp"

#include <vector>

namespace vdj {
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
    private:
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

        INLINE vec<2, T> center() const { return vec<2, T>({ fX + fWidth / 2.0, fY + fHeight / 2 }); }

        bool isEmpty() const { return (fWidth <= 0) || (fHeight <= 0); }

        bool contains(const T& x, const T& y) const override
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

        GeoRect<T>& operator*=(double s)
        {
            fX *= s;
            fY *= s;
            fWidth *= s;
            fHeight *= s;

            return *this;
        }

        GeoRect<T> operator+(const GeoRect<T>& rhs)
        {
            GeoRect<T> res(*this);
            return res += rhs;
        }

        GeoRect<T> operator * (double s) const
        {
            GeoRect<T> res(*this);
            return res *= s;
        }

    };


    // Simple description of an ellipse
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
        std::vector<vdj::Point<T> > fVertices;

        GeoPolygon()
            :fTop(65535)
            , fBottom(0)
        {
            clear();
        }

        // clear out existing commands and vertices
        void clear()
        {
            fVertices.clear();
            fTop = 65535;
            fBottom = 0;
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
                fTop = std::min(fTop, fVertices[i].y());
                fBottom = std::max(fBottom, fVertices[i].y());
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
        {
            GeoPolygon<T>::addPoint(Point<T>(x1, y1));
            GeoPolygon<T>::addPoint(Point<T>(x2, y2));
            GeoPolygon<T>::addPoint(Point<T>(x3, y3));

            GeoPolygon<T>::findTopmost();
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
        GeoCubicBezier(const Point<T>& pp1, const Point<T>& pp2, const Point<T>& pp3, const Point<T>& pp4)
            :p1(pp1), p2(pp2), p3(pp3), p4(pp4)
        {}

        GeoCubicBezier(const T x1, const T y1, const T x2, const T y2, const T x3, const T y3, const T x4, const T y4)
            :p1({ x1, y1 })
            , p2({ x2, y2 })
            , p3({ x3, y3 })
            , p4({ x4, y4 })

        {}

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


    //
    // A path should have commands and vertices
    //
    template <typename T>
    struct GeoPath
    {
    public:
        enum class ContourCommand
        {
            MoveTo = 0,

            Line,
            LineTo,

            HLine,
            HLineTo,

            VLine,
            VLineTo,

            CubicBezierTo,      // pt1, control1, control2, pt2
            QuadraticBezierTo,  // pt1, control, pt2

            ArcCircleTo,
            ArcEllipseTo,

            Rect,
            Circle,
            Ellipse,

            Close = 255,
        };

    protected:
        void addNumber(const T& value)
        {
            fNumbers.push_back(value);
        }

        void addPoint(const Point<T>& pt)
        {
            addNumber(pt.x());
            addNumber(pt.y());
        }


        void addCommand(const ContourCommand cmd)
        {
            fCommands.push_back(cmd);
        }


    public:
        std::vector<ContourCommand> fCommands;
        std::vector<T> fNumbers;  // vertices
        Point<T> fLastPosition;

        GeoPath()
            :fLastPosition(T(), T())
        {}

        // clear out existing commands and vertices
        void clear()
        {
            fCommands.clear();
            fNumbers.clear();
            fLastPosition = Point<T>();
        }

        //INLINE T numbers() { return fNumbers; }
        INLINE T getNumber(size_t& idx) const { idx++; return fNumbers[idx-1]; }
        INLINE Point<T> getPoint(size_t &idx) const 
        {

            T x = getNumber(idx);
            T y = getNumber(idx);

            return Point<T>(x, y);
        }

        void moveTo(const Point<T>& pt)
        {
            addCommand(ContourCommand::MoveTo);
            addPoint(pt);

            fLastPosition = pt;
        }

        void line(const Point<T>& p1, const Point<T>& p2)
        {
            addCommand(ContourCommand::Line);
            addPoint(p1);
            addPoint(p2);

            fLastPosition = p2;
        }

        void lineTo(const Point<T>& pt)
        {
            addCommand(ContourCommand::LineTo);
            addPoint(pt);
            fLastPosition = pt;
        }

        void hLineTo(const Point<T>& pt)
        {
            addCommand(ContourCommand::LineTo);
            addPoint(pt);
            fLastPosition = pt;
        }

        void vLineTo(const Point<T>& pt)
        {
            addCommand(ContourCommand::VLineTo);
            addPoint(pt);
            fLastPosition = pt;
        }

        void cubicTo(const Point<T>& c1, const Point<T>& c2, const Point<T>& p2)
        {
            addCommand(ContourCommand::CubicBezierTo);
            addPoint(c1);
            addPoint(c2);
            addPoint(p2);

            fLastPosition = p2;
        }

        void quadraticTo(const Point<T>& c1, const Point<T>& p2)
        {
            addCommand(ContourCommand::QuadraticBezierTo);
            addPoint(c1);
            addPoint(p2);

            fLastPosition = p2;
        }

        // Does not alter last point
        void rect(const T x, const T y, const T w, const T h)
        {
            addCommand(ContourCommand::Rect);
            addPoint(Point<T>(x, y));
            addPoint(Point<T>(w, h));
        }

        void ellipse(const T cx, const T cy, const T xRadius, const T yRadius)
        {
            addCommand(ContourCommand::Ellipse);
            addPoint(Point<T>(cx, cy));
            addPoint(Point<T>(xRadius, yRadius));
        }

        void ellipse(const T cx, const T cy, const T radius)
        {
            addCommand(ContourCommand::Circle);
            addPoint(Point<T>(cx, cy));
            addPoint(Point<T>(radius, radius));
        }

        void close()
        {
            addCommand(ContourCommand::Close);
        }
    };

}
// vdj namespace