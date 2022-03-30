#pragma once

#include "apidefs.h"
#include "maths.hpp"

namespace alib {

    template <typename T>
    struct Point
    {
        typedef T Type;
        T fX;
        T fY;

        Point();
        Point(const T &a, const T &b);

        template <typename TX, typename TY>
        Point(TX tx, TY ty);
        ~Point();

        // Copy construct from any point type
        //template <class TP, template<class> class TPoint> Point(const TPoint<TP>& p);


        T x() const;
        T y() const;

        // Assignment
        template <typename TP> Point& operator = (const Point<TP>& p);

        // Negation
        //Point<T> operator - (const Point<T>& p);
        
        // Some arithmetic operators
        // By some constant
        // multiply self by arbitrary type
        template <typename TA> Point& operator *= (const TA& a);
        Point& operator /= (double a);

        // By another point type
        template <typename TP> Point& operator += (const Point<TP>& p);
        template <typename TP> Point& operator -= (const Point<TP>& p);

    };


    // Field accessors
    template <typename T> T Point<T>::x() const { return fX; }
    template <typename T> T Point<T>::y() const { return fY; }

    // Declaration of operators
    template <typename T> Point<T> operator * (const Point<T>& p1, const Point<T>& p2);
    template <typename TP, typename TA> Point<TP> operator * (const Point<TP>& p, const TA& a);
    template <typename TP, typename TA> Point<TP> operator * (const TA& a, const Point<TP>& p);
    template <typename T> Point<T> operator - (const Point<T>& p);

    // Struct Point Implementation
    // Point is one of the most used data structures.  We want
    // want it to be efficient, and comlplete for conversions
    template <class TD, class TS>
    INLINE TD Convert(TS src)
    {
        return (TD)src;
    }

    template<>
    INLINE ptrdiff_t Convert<ptrdiff_t, double>(double src)
    {
        return (ptrdiff_t)alib::Round(src);
    }

    template<>
    INLINE ptrdiff_t Convert<ptrdiff_t, float>(float src)
    {
        return (ptrdiff_t)alib::Round(src);
    }

    template <typename T>
    Point<T>::Point() : fX(T()), fY(T()) {}

    template <typename T>
    Point<T>::Point(const T &x, const T& y)
    : fX(x), fY(y) {}

    // This constructor allows you to create
    // a point with a mix of types from double,
    // float, and int typically
    template <typename T>
    template <typename TX, typename TY>
    INLINE Point<T>::Point(TX tx, TY ty)
        :fX(Convert<T, TX>(tx))
        , fY(Convert<T, TY>(ty))
    {}

    template <typename T>
    Point<T>::~Point() {}

    template <typename T> template <typename TP>
    INLINE Point<T>& Point<T>::operator = (const Point<TP>& p)
    {
        fX = Convert<T, TP>(p.fX);
        fY = Convert<T, TP>(p.fY);
        return *this;
    }

    template <typename T> template <typename TA>
    INLINE Point<T>& Point<T>::operator *= (const TA& a)
    {
        fX = Convert<T, TA>(fX * a);
        fY = Convert<T, TA>(fY * a);
        return *this;
    }

    template <typename T>
    INLINE Point<T>& Point<T>::operator /= (double a)
    {
        fX = Convert<T, double>(fX / a);
        fY = Convert<T, double>(fY / a);
        return *this;
    }

    template <typename T> template <typename TP>
    INLINE Point<T>& Point<T>::operator += (const Point<TP>& p)
    {
        fX += Convert<T, TP>(p.fX);
        fY += Convert<T, TP>(p.fY);
        return *this;
    }

    template <typename T> template <typename TP>
    INLINE Point<T>& Point<T>::operator -= (const Point<TP>& p)
    {
        fX -= Convert<T, TP>(p.fX);
        fY -= Convert<T, TP>(p.fY);
        return *this;
    }

    template <typename T>
    INLINE Point<T> operator + (const Point<T>& p1, const Point<T>& p2)
    {
        return Point<T>(p1.fX + p2.fX, p1.fY + p2.fY);
    }

    // Negation
    template <typename T>
    INLINE Point<T> operator - (const Point<T>& p)
    {
        return Point<T>(-p.FX, -p.fY);
    }

    // Subtraction
    template <typename T>
    INLINE Point<T> operator - (const Point<T>& p1, const Point<T>& p2)
    {
        return Point<T>(p1.fX - p2.fX, p1.fY - p2.fY);
    }

    // Multiplication
    template <typename T>
    INLINE Point<T> operator * (const Point<T>& p1, const Point<T>& p2)
    {
        return Point<T>(p1.fX * p2.fX, p1.fY * p2.fY);
    }

    template <typename TP, typename TA>
    INLINE Point<TP> operator * (const Point<TP>& p, const TA& a)
    {
        return Point<TP>(p.fX * a, p.fY * a);
    }

    template <typename TP, typename TA>
    INLINE Point<TP> operator * (const TA& a, const Point<TP>& p)
    {
        return Point<TP>(p.fX * a, p.fY * a);
    }


}
// namespace vdj