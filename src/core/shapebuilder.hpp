#pragma once

#include "geotypes.hpp"

//
 // A path should have commands and vertices
 //
template <typename T>
struct ShapeBuilder
{
public:
    enum class ShapeCommand
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

    std::vector<ShapeCommand> fCommands;
    std::vector<T> fNumbers;  // vertices
    Point<T> fLastPosition;

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


    void addCommand(const ShapeCommand cmd)
    {
        fCommands.push_back(cmd);
    }

public:

    ShapeBuilder()
        :fLastPosition(T(), T())
    {}

    // clear out existing commands and numbers
    void clear()
    {
        fCommands.clear();
        fNumbers.clear();
        fLastPosition = Point<T>();
    }

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
    void moveTo(const Point<T>& pt)
    {
        addCommand(ShapeCommand::MoveTo);
        addPoint(pt);

        fLastPosition = pt;
    }

    // Add a line, using current point as first endpoint
    void lineTo(const Point<T>& pt)
    {
        addCommand(ShapeCommand::LineTo);
        addPoint(pt);
        fLastPosition = pt;
    }

    // Add horizontal line, using only x coordinate added to 
    // existing point
    void hLineTo(const T& x)
    {
        addCommand(ShapeCommand::HLineTo);
        addNumber(x);

        fLastPosition = Point<T>(x,fLastPosition.y);
    }

    void vLineTo(const T& y)
    {
        addCommand(ShapeCommand::VLineTo);
        addNumber(y);
        fLastPosition = Point<T>(fLastPosition.x, y);
    }

    void cubicTo(const Point<T>& c1, const Point<T>& c2, const Point<T>& p2)
    {
        addCommand(ShapeCommand::CubicBezierTo);
        addPoint(c1);
        addPoint(c2);
        addPoint(p2);

        fLastPosition = p2;
    }

    void quadraticTo(const Point<T>& c1, const Point<T>& p2)
    {
        addCommand(ShapeCommand::QuadraticBezierTo);
        addPoint(c1);
        addPoint(p2);

        fLastPosition = p2;
    }
    
    void close()
    {
        addCommand(ShapeCommand::Close);
    }

    //===== Adding Discrete Geometries =====
    // BUGBUG - Maybe these don't belong here
    // They do not alter the fLastPosition

    // add a discrete line with two points
    void line(const Point<T>& p1, const Point<T>& p2)
    {
        addCommand(ShapeCommand::Line);
        addPoint(p1);
        addPoint(p2);

        fLastPosition = p2;
    }

    void rect(const T x, const T y, const T w, const T h)
    {
        addCommand(ShapeCommand::Rect);
        addPoint(Point<T>(x, y));
        addPoint(Point<T>(w, h));
    }

    void ellipse(const T cx, const T cy, const T xRadius, const T yRadius)
    {
        addCommand(ShapeCommand::Ellipse);
        addPoint(Point<T>(cx, cy));
        addPoint(Point<T>(xRadius, yRadius));
    }

    void circle(const T cx, const T cy, const T radius)
    {
        addCommand(ShapeCommand::Circle);
        addPoint(Point<T>(cx, cy));
        addPoint(Point<T>(radius, radius));
    }

    /*
    CubicBezierTo,      // pt1, control1, control2, pt2
    QuadraticBezierTo,  // pt1, control, pt2
    ArcCircleTo,
    ArcEllipseTo,
    Rect,
    Circle,
    Ellipse,
    Close = 255,
*/
    INLINE GeoPolygon<T> getPolyline()
    {
        typedef Point<ptrdiff_t> Point;

        size_t ptidx = 0;
        Point lastPos;
        GeoPolygon<T> fig;

        for (size_t i = 0; i < fCommands.size(); i++)
        {
            switch (fCommands[i])
            {
            case ShapeCommand::MoveTo:
            {
                // start a new polygon
                fig.clear();

                lastPos = getPoint(ptidx);
                fig.addPoint(lastPos);
            }
            break;

            case ShapeCommand::LineTo:
            {
                    lastPos = path.getPoint(ptidx);
                    fig.addPoint(lastPos);
            }

            case ShapeCommand::HLineTo:
            {
                // get x
                auto x = path.getNumber(ptidx);
                lastPos.fX = x;
                fig.addPoint(lastPos);
            }
            break;

            case ShapeCommand::VLineTo:
            {
                auto y = path.getNumber(ptidx);
                lastPos.fY = y;
                fig.addPoint(lastPos);
            }
            break;

            case ShapeCommand::Close:
                fig.setClose(true);
                break;

            default:
                // do nothing
                break;
            }

            /*
                        case ShapeCommand::Line:
                        case ShapeCommand::HLine:
                        case ShapeCommand::VLine:
                        {
                            // independent lines
                            Point pt1 = path.getPoint(ptidx);
                            Point pt2 = path.getPoint(ptidx);
                            line(pb, pt1, pt2, c, width);
                        }
                        break;
                        */
        }
    }
};
