#pragma once

#include "geotypes.hpp"

#include <vector>

//
// SVGPathBuilder
// 
// This builder creates polygons using standard vector
// commands (moveTo, lineTo, cubicTo, etc)
// Working in conjuction with some context, you can build
// figures, with stroke and fill commands
//
namespace vdj
{
    //
     // A path should have commands and vertices
     //
    template <typename T>
    struct SVGPathBuilder
    {
    public:
        //
        // SVG Path Commands
        // M - moveTo       (M, m)
        // L - lineTo       (L, l, H, h, V, v)
        // C - cubicTo      (C, c, S, s)
        // Q - quadTo       (Q, q, T, t)
        // A - ellipticArc  (A, a)
        // Z - close        (Z, z)

        enum class ContourCommand
        {
            MoveTo = 0,

            LineTo,
            HLineTo,
            VLineTo,

            CubicBezierTo,
            QuadraticBezierTo,

            ArcCircleTo,
            ArcEllipseTo,

            Line,

            Close = 255,
        };

        std::vector<ContourCommand> fCommands;
        std::vector<T> fNumbers;  // vertices
        Point<T> fLastPosition;

    public:
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

        SVGPathBuilder()
            :fLastPosition(T(), T())
        {}

        // retrieve a number from the vector of numbers
        // and increment the index
        INLINE T getNumber(size_t& idx) const { idx++; return fNumbers[idx - 1]; }

        // Retrieve two numbers, and increment index
        INLINE Point<T> getPoint(size_t& idx) const
        {
            T x = getNumber(idx);
            T y = getNumber(idx);

            return Point<T>(x, y);
        }

        // Add a moveTo command, and two numbers
        void moveTo(const T& x, const T& y)
        {
            addCommand(ContourCommand::MoveTo);
            addNumber(x);
            addNumber(y);
            fLastPosition = Point<T>(x, y);
        }

        void moveTo(const Point<T>& pt)
        {
            moveTo(pt.x(), pt.y());
        }

        void moveBy(const T& dx, const T& dy)
        {
            moveTo(fLastPosition.x() + dx, fLastPosition.y() + dy);
        }


        // Add a line, using current point as first endpoint
        // SVG - H
        void lineTo(const T& x, const T& y)
        {
            addCommand(ContourCommand::LineTo);
            addNumber(x);
            addNumber(y);
            fLastPosition = Point<T>(x, y);
        }

        void lineTo(const Point<T>& pt)
        {
            lineTo(pt.x(), pt.y());
        }

        // Add a line using relative coordinates
        //SVG - l
        void lineBy(const T& dx, const T& dy)
        {
            lineTo(fLastPosition.x() + dx, fLastPosition.y() + dy);
        }

        // Add horizontal line, using only x coordinate added to existing point
        // SVG - H
        void hLineTo(const T& x)
        {
            addCommand(ContourCommand::HLineTo);
            addNumber(x);

            fLastPosition = Point<T>(x, fLastPosition.y);
        }

        // SVG - h
        void hLineBy(const T& dx)
        {
            hLineTo(fLastPosition.x() + dx, fLastPosition().y());
        }

        // SVG - V
        void vLineTo(const T& y)
        {
            addCommand(ContourCommand::VLineTo);
            addNumber(y);
            fLastPosition = Point<T>(fLastPosition.x, y);
        }

        // SVG - v
        void vLineBy(const T& dy)
        {
            vLineTo(fLastPosition.x(), fLastPosition().y() + dy);
        }

        // SVG - Q
        // Quadratic Bezier curve
        void quadTo(const T& c1x, const T& c1y,
            const T& p2x, const T& p2y)
        {
            addCommand(ContourCommand::QuadraticBezierTo);
            addNumber(c1x);
            addNumber(c1y);
            addNumber(p2x);
            addNumber(p2y);

            fLastPosition = Point<T>(p2x, p2y);
        }

        void quadTo(const Point<T>& c1, const Point<T>& p2)
        {
            quadTo(c1.x(), c1.y(), p2.x(), p2.y());
        }

        // SVG - C
        void cubicTo(const T & c1x, const T& c1y, 
            const T & c2x, const T &c2y, 
            const T & p2x, const T& p2y)
        {
            addCommand(ContourCommand::CubicBezierTo);
            addNumber(c1x);
            addNumber(c1y);
            addNumber(c2x);
            addNumber(c2y);
            addNumber(p2x);
            addNumber(p2y);

            fLastPosition = Point<T>(p2x, p2y);
        }


        void cubicTo(const Point<T>& c1, const Point<T>& c2, const Point<T>& p2)
        {
            cubicTo(c1.x(), c1.y(), c2.x(), c2.y(), p2.x(), p2.y());
        }

        //void cubicBy(const T& dc1x, const T& dc1y,
        //    const T& dc2x, const T& dc2y,
        //    const T& dp2x, const T& dp2y)
        //{}
        // SVG - S, smooth cubicTo
        // SVG - s, smooth cubicBy

        // SVG - Z,z    close path
        void close()
        {
            addCommand(ContourCommand::Close);
        }





        INLINE std::vector<GeoPolygon<T> > getFigures()
        {
            typedef Point<ptrdiff_t> Point;

            size_t ptidx = 0;
            Point lastPos;

            std::vector<GeoPolygon<T> > figures;    // figures returned
            GeoPolygon<T> fig;

            for (size_t i = 0; i < fCommands.size(); i++)
            {
                switch (fCommands[i])
                {
                case ContourCommand::MoveTo:
                {
                    // If current figure not empty
                    // add it to the list of figures
                    if (!fig.isEmpty())
                    {
                        fig.findTopmost();
                        figures.push_back(fig);
                    }

                    // start a new polygon
                    fig.clear();

                    lastPos = getPoint(ptidx);
                    fig.addPoint(lastPos);
                }
                break;

                case ContourCommand::Line:
                {
                    // finish previous figure if any
                    if (!fig.isEmpty()) {
                        fig.findTopmost();
                        figures.push_back(fig);
                        fig.clear();
                    }

                    auto p1 = getPoint(ptidx);
                    auto p2 = getPoint(ptidx);
                    fig.addPoint(p1);
                    fig.addPoint(p2);
                    
                    figures.push_back(fig);
                    fig.clear();
                }
                break;

                case ContourCommand::LineTo:
                {
                    lastPos = getPoint(ptidx);
                    fig.addPoint(lastPos);
                }
                break;

                case ContourCommand::HLineTo:
                {
                    auto x = getNumber(ptidx);
                    lastPos.fX = x;
                    fig.addPoint(lastPos);
                }
                break;

                case ContourCommand::VLineTo:
                {
                    auto y = getNumber(ptidx);
                    lastPos.fY = y;
                    fig.addPoint(lastPos);
                }
                break;

                case ContourCommand::QuadraticBezierTo:
                {
                    // lastPos
                    auto c1 = getPoint(ptidx);
                    auto p2 = getPoint(ptidx);

                    // 
                    // do the evaluation
                    GeoQuadraticBezier<T> bez(lastPos, c1, p2);
                    constexpr size_t maxSegments = 24;
                    fig.addPoint(lastPos);
                    for (size_t i = 1; i <= maxSegments; i++)
                    {
                        double u = maths::Map(i, 0, maxSegments, 0.0, 1.0);
                        auto pt = bez.eval(u);
                        fig.addPoint(pt);
                    }

                    lastPos = p2;
                }
                break;

                case ContourCommand::CubicBezierTo:
                {
                    // lastPos
                    auto c1 = getPoint(ptidx);
                    auto c2 = getPoint(ptidx);
                    auto p2 = getPoint(ptidx);

                    // 
                    // do the evaluation
                    GeoCubicBezier<T> bez(lastPos, c1, c2, p2);
                    constexpr size_t maxSegments = 24;
                    fig.addPoint(lastPos);
                    for (size_t i = 1; i <= maxSegments; i++)
                    {
                        double u = maths::Map(i, 0, maxSegments, 0.0 , 1.0);
                        auto pt = bez.eval(u);
                        fig.addPoint(pt);
                    }

                    lastPos = p2;
                }
                break;

                case ContourCommand::Close:
                    fig.setClose(true);

                    // add to list of figures
                    fig.findTopmost();
                    figures.push_back(fig);
                    fig.clear();

                    break;

                default:
                    // do nothing
                    break;
                }

            }

            return figures;
        }
    };

}
