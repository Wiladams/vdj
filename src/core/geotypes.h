#pragma once

#include "vec.h"

using PixelCoord = vec<2, int>;

template <typename T>
using GeoCoord = vec<2, T>;



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
    vec<2,T> pt1;
    vec<2,T> pt2;
};

// General rectangle representation
//
template <typename T>
struct GeoRect {
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

    bool isEmpty() const { return (fWidth <= 0) || (fHeight <= 0); }

    bool contains(const T& x, const T& y) const
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
        if (!contain(other.x, other.y))
        {
            return false;
        }

        if (!containsPoint(other.x + other.width - 1, other.y + other.height - 1))
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


    static GeoRect<double> create(const GeoRect<int>& isect, const GeoRect<int>& constraint)
    {
        // If no intersection, there's no need to figure
        // out texture coordinates
        if (isect.isEmpty())
            return GeoRect<double>();

        // Figure out texture coordinates based on the intersection
        double left = maths::Map(isect.x(), constraint.x(), constraint.x() + constraint.w() - 1, 0, 1);
        double top = maths::Map(isect.y(), constraint.y(), constraint.y() + constraint.h() - 1, 0, 1);
        double right = maths::Map(isect.x() + isect.w() - 1, constraint.x(), constraint.x() + constraint.w() - 1, 0.0, 1.0);
        double bottom = maths::Map(isect.y() + isect.h() - 1, constraint.y(), constraint.y() + constraint.h() - 1, 0.0, 1.0);

        return  GeoRect < double>(left, top, right - left, bottom - top);
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

    // type conversion
    // need this to be explicit
    //operator PixelRect () const { return { (int)maths::Round(x()),(int)maths::Round(y()),(int)maths::Round(w()), (int)maths::Round(h()) }; }

};


// Simple description of an ellipse
template <typename T>
struct GeoEllipse
{
    T cx, cy;   // center of ellipse
    T rx;       // radius in x axis
    T ry;       // radius in y axis
};

/*
  PixelTriangle

  A triangle representation
*/
template <typename T>
struct GeoTriangle {
    vec<2,T> verts[3];   // A triangle has 3 vertices

    // Basic constructor take coordinates in any order, sorts them
    // so that subsequent drawing will be faster.
    GeoTriangle(const int x1, const int y1,
        const int x2, const int y2,
        const int x3, const int y3)
    {
        verts[0] = { x1, y1 };
        verts[1] = { x2, y2 };
        verts[2] = { x3, y3 };

        // sort the coordinates from topmost
        // to bottommost, so drawing is easier
        // This is essentially a 3 element bubble sort
        vec<2,T> tmp;
        if (verts[0].y() > verts[1].y()) {
            tmp = verts[0];
            verts[0] = verts[1];
            verts[1] = tmp;
        }

        if (verts[1].y() > verts[2].y()) {
            tmp = verts[1];
            verts[1] = verts[2];
            verts[2] = tmp;
        }

        // One more round to ensure the second isn't the smallest
        if (verts[0].y() > verts[1].y()) {
            tmp = verts[0];
            verts[0] = verts[1];
            verts[1] = tmp;
        }
    }
};

// Cubic Bezier, defined by 4 points
template <typename T>
struct GeoBezier
{
    GeoCoord<T> p1;
    GeoCoord<T> p2;
    GeoCoord<T> p3;
    GeoCoord<T> p4;

public:
    GeoBezier(const GeoCoord<T>& pp1, const GeoCoord<T>& pp2, const GeoCoord<T>& pp3, const GeoCoord<T>& pp4)
        :p1(pp1), p2(pp2), p3(pp3), p4(pp4)
    {}

    GeoBezier(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
        :p1({ x1, y1 })
        ,p2({ x2, y2 })
        ,p3({ x3, y3 })
        ,p4({ x4, y4 })

    {}

    // Value of curve at parametric position 'u'
// control points are P0, P1, P2, P3
// This function calculates a single component (x, y, or whatever)
// use another function to combine
// Cubic Bezier
    INLINE GeoCoord<T> eval(const double u)
    {
        double oneminusu = 1 - u;
        double BEZ03 = oneminusu * oneminusu * oneminusu;	// (1-u)^3
        double BEZ13 = 3 * u * (oneminusu * oneminusu);	    // 3u(1-u)^2
        double BEZ23 = 3 * (u * u) * oneminusu;				// 3u^2(1-u)
        double BEZ33 = u * u * u;							// u^3

        return ((p1 * BEZ03) + (p2*BEZ13) + (p3*BEZ23) + (p4*BEZ33));
    }

};

/*
template <typename T>
struct GeoPolygon
{
    // http://alienryderflex.com/polygon_fill/
    //  public-domain code by Darel Rex Finley, 2007

    GeoPolygon()
    {
        int  nodes, nodeX[MAX_POLY_CORNERS], pixelX, pixelY, i, j, swap;

        //  Loop through the rows of the image.
        for (pixelY = IMAGE_TOP; pixelY < IMAGE_BOT; pixelY++) 
        {

        //  Build a list of nodes.
        nodes = 0; j = polyCorners - 1;
        for (i = 0; i < polyCorners; i++) {
            if (polyY[i] < (double)pixelY && polyY[j] >= (double)pixelY
                || polyY[j] < (double)pixelY && polyY[i] >= (double)pixelY) {
                nodeX[nodes++] = (int)(polyX[i] + (pixelY - polyY[i]) / (polyY[j] - polyY[i])
                    * (polyX[j] - polyX[i]));
            }
            j = i;
        }

        //  Sort the nodes, via a simple “Bubble” sort.
        i = 0;
        while (i < nodes - 1) {
            if (nodeX[i] > nodeX[i + 1]) {
                swap = nodeX[i]; nodeX[i] = nodeX[i + 1]; nodeX[i + 1] = swap; if (i) i--;
            }
            else {
                i++;
            }
        }

        //  Fill the pixels between node pairs.
        for (i = 0; i < nodes; i += 2) {
            if (nodeX[i] >= IMAGE_RIGHT) break;
            if (nodeX[i + 1] > IMAGE_LEFT) {
                if (nodeX[i] < IMAGE_LEFT) nodeX[i] = IMAGE_LEFT;
                if (nodeX[i + 1] > IMAGE_RIGHT) nodeX[i + 1] = IMAGE_RIGHT;
                for (pixelX = nodeX[i]; pixelX < nodeX[i + 1]; pixelX++) fillPixel(pixelX, pixelY);
            }
        }
    }
};
*/
