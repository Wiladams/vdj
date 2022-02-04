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

#include "pixelmap.h"
#include "maths.h"
#include "sampler.h"

#include <memory>

using namespace maths;

// Set a single pixel
INLINE void point(PixelMap& pmap, const int x, const int y, const PixelRGBA src)
{
    pmap.set(x, y, src);
}


//
// Line Clipping in preparation for line drawing
//
typedef int OutCode;

static const uint32_t LN_INSIDE = 0; // 0000
static const uint32_t LN_LEFT = 1;   // 0001
static const uint32_t LN_RIGHT = 2;  // 0010
static const uint32_t LN_BOTTOM = 4; // 0100
static const uint32_t LN_TOP = 8;    // 1000

// Compute the bit code for a point (x, y) using the clip rectangle
// bounded diagonally by (xmin, ymin), and (xmax, ymax)
INLINE int computeOutCode(const PixelRect &rct, const int x, const int y)
{
    uint32_t code = LN_INSIDE;   // initialised as being inside of clip window
    int xmin = rct.x();
    int xmax = rct.x() + rct.w() - 1;
    int ymin = rct.y();
    int ymax = rct.y() + rct.h() - 1;
       

    if (x < xmin)           // to the left of clip window
        code |= LN_LEFT;
    else if (x > xmax)      // to the right of clip window
        code |= LN_RIGHT;
    
    if (y < ymin)           // above the clip window
        code |= LN_BOTTOM;
    else if (y > ymax)      // below the clip window
        code |= LN_TOP;

    return code;
}


// Using cohen Sutherland clipping algorithm
// Changes the input line coordinates to fit within
// the specified clipping rectangle.
// if the line is completely outside the clipping rectangle
// then false is returned, otherwise, true is returned
// and the input coordinates are changed to those that will
// fit within the clipping rectangle.
//
inline bool clipLine(const PixelRect &bounds, int& x0, int& y0, int& x1, int& y1)
{
    // compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
    int outcode0 = computeOutCode(bounds, x0, y0);
    int outcode1 = computeOutCode(bounds, x1, y1);
    bool accept = false;
    double xmin = bounds.x();
    double xmax = bounds.x() + bounds.w() - 1;
    double ymin = bounds.y();
    double ymax = bounds.y() + bounds.h() - 1;

    while (true) {
        if (!(outcode0 | outcode1)) { // Bitwise OR is 0. Trivially accept and get out of loop
            accept = true;
            break;
        }
        else if (outcode0 & outcode1) { // Bitwise AND is not 0. Trivially reject and get out of loop
            break;
        }
        else {
            // failed both tests, so calculate the line segment to clip
            // from an outside point to an intersection with clip edge
            double x = 0;
            double y = 0;

            // At least one endpoint is outside the clip rectangle; pick it.
            int outcodeOut = outcode0 ? outcode0 : outcode1;

            // Now find the intersection point;
            // use formulas y = y0 + slope * (x - x0), x = x0 + (1 / slope) * (y - y0)
            if (outcodeOut & LN_TOP) {           // point is above the clip rectangle
                x = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0);
                y = ymax;
            }
            else if (outcodeOut & LN_BOTTOM) { // point is below the clip rectangle
                x = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0);
                y = ymin;
            }
            else if (outcodeOut & LN_RIGHT) {  // point is to the right of clip rectangle
                y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
                x = xmax;
            }
            else if (outcodeOut & LN_LEFT) {   // point is to the left of clip rectangle
                y = y0 + (y1 - y0) * (xmin - x0) / (x1 - x0);
                x = xmin;
            }

            // Now we move outside point to intersection point to clip
            // and get ready for next pass.
            if (outcodeOut == outcode0) {
                x0 = (int)x;
                y0 = (int)y;
                outcode0 = computeOutCode(bounds, x0, y0);
            }
            else {
                x1 = (int)x;
                y1 = (int)y;
                outcode1 = computeOutCode(bounds, x1, y1);
            }
        }
    }

    return accept;
}

// Replicate the single pixel value across a horizontal line
// This assumes coordinates have already been clipped
// so no range checking happens here
INLINE void copySpan(PixelMap& pmap, const int x, const int y, const int width, const PixelRGBA& src)
{
    __stosd((unsigned long*)pmap.getPixelPointer(x, y), src.value, width);
}

INLINE void blendSpan(PixelMap& pmap, const int x, const int y, const int width, const PixelRGBA& src)
{
    PixelRGBA* pixelPtr = (PixelRGBA*)pmap.getPixelPointer(x, y);
    for (int offset = 0; offset < width; offset++)
        pixelPtr[offset] = blend_pixel(pixelPtr[offset], src);
}

INLINE void setSpan(PixelMap& pmap, const int x, const int y, const int width, const PixelRGBA &src)
{
    if (src.isTransparent())
        return;     // fully transparent, nothing to do

    if (src.isOpaque())
        copySpan(pmap, x, y, width, src);
    else
        blendSpan(pmap, x, y, width, src);
}

INLINE void verticalLine(PixelMap& pmap, const int x, const int y, const int height, const PixelRGBA src)
{
    for (int row = y; row < y + height - 1; row++)
        pmap.set(x, row, src);
}

// 
// Vertical line drawing
// draw a vertical line from (x, y) to (x, y + height - 1)
//
INLINE void vline(PixelMap& pmap, const int x, const int y, const int height, const PixelRGBA src)
{
    int vheight = height;
    int x1 = x;
    int y1  = y;
    int x2 = x;
    int y2 = y+height-1;

    if (clipLine(pmap.frame(), x1, y1, x2, y2))
        verticalLine(pmap, x1, y1, y2-y1, src);
}

/*
line()

Stroke a line using the current stroking pixel.
Uses Bresenham line drawing.

clips line to frame of pixelmap
*/
INLINE void line(PixelMap& pmap, const int x1, const int y1, const int x2, const int y2, const PixelRGBA &c)
{
    int sdx, sdy, dxabs, dyabs;
    unsigned int x, y, px, py;

    // do clipping right here
    int x11 = x1;
    int y11 = y1;
    int x21 = x2;
    int y21 = y2;

    if (!clipLine({ 0,0,pmap.width(), pmap.height() }, x11, y11, x21, y21))
        return;

    int w = x21 - x11;

    int dx = x21 - x11;      // the horizontal distance of the line
    int dy = y21 - y11;      // the vertical distance of the line
    dxabs = maths::Abs(dx);
    dyabs = maths::Abs(dy);
    sdx = maths::Sign(dx);
    sdy = maths::Sign(dy);

    if (dyabs == 0) {
        // optimize for horizontal line
        x = x11 > x21 ? x21 : x11;
        setSpan(pmap, x, y11, dxabs, c);
        return;
    }

    x = dyabs >> 1;
    y = dxabs >> 1;
    px = x11;
    py = y11;

    pmap.set(x11, y11, c);  // set initial pixel

    if (dxabs >= dyabs) // the line is more horizontal than vertical
    {
            for (int i = 0; i<dxabs; i++)
            {
                y = y + dyabs;
                if (y >= (unsigned int)dxabs)
                {
                    y = y - dxabs;
                    py = py + sdy;
                }
                px = px + sdx;
                pmap.set(px, py, c);
            }
    } else // the line is more vertical than horizontal
    {
            for (int i = 0; i<dyabs; i++)
            {
                x += dxabs;
                if (x >= (unsigned int)dyabs)
                {
                    x -= dyabs;
                    px += sdx;
                }
                py += sdy;
                pmap.set(px, py, c);
            }
    }

}



/*
    Bresenham ellipse drawing algorithm
    Only for the frame, not for filling
*/
typedef void(*EllipseHandler)(PixelMap& pb, const int cx, const int cy, const int x, const int y, const PixelRGBA &color);

INLINE void Plot4EllipsePoints(PixelMap& pb, const int cx, const int cy, const int x, const int y, const PixelRGBA &color)
{
    int lowx = cx - x;
    int maxx = cx + x;
    int lowy = cy - y;
    int maxy = cy + y;

    pb.set(cx + x, cy - y, color);
    pb.set(cx - x, cy - y, color);
    pb.set(cx - x, cy + y, color);
    pb.set(cx + x, cy + y, color);
}

INLINE void fill2EllipseLines(PixelMap& pb, const int cx, const int cy, const int x, const int y, const PixelRGBA &color)
{
    int x1 = cx - x;
    int y1 = cy + y;
    int x2 = cx + x;
    int y2 = cy + y;

    if (clipLine({0,0,pb.width(), pb.height()}, x1, y1, x2, y2)) {
        setSpan(pb, x1, y1, x2 - x1, color);
    }

    y1 = cy - y;
    y2 = cy - y;
    if (clipLine({0,0,pb.width(), pb.height()}, x1, y1, x2, y2)) {
        setSpan(pb, x1, y1, x2 - x1, color);
    }
}

INLINE void raster_ellipse(PixelMap& pb, const int cx, const int cy, const size_t xradius, size_t yradius, const PixelRGBA &color, EllipseHandler handler)
{
    int x = xradius;
    int y = 0;
    int xchange = yradius * yradius * (1 - 2 * xradius);
    int ychange = xradius * xradius;
    int ellipseerror = 0;
    int twoasquare = 2 * xradius * xradius;
    int twobsquare = 2 * yradius * yradius;
    int stoppingx = twobsquare * xradius;
    int stoppingy = 0;

    // first set of points, sides
    while (stoppingx >= stoppingy)
    {
        handler(pb, cx, cy, x, y, color);
        y++;
        stoppingy += twoasquare;
        ellipseerror += ychange;
        ychange += twoasquare;
        if ((2 * ellipseerror + xchange) > 0) {
            x--;
            stoppingx -= twobsquare;
            ellipseerror += xchange;
            xchange += twobsquare;
        }
    }
    
    // second set of points, top and bottom
    x = 0;
    y = yradius;
    xchange = yradius * yradius;
    ychange = xradius * xradius * (1 - 2 * yradius);
    ellipseerror = 0;
    stoppingx = 0;
    stoppingy = twoasquare * yradius;

    while (stoppingx <= stoppingy) {
        handler(pb, cx, cy, x, y, color);
        x++;
        stoppingx += twobsquare;
        ellipseerror += xchange;
        xchange += twobsquare;
        if ((2 * ellipseerror + ychange) > 0) {
            y--;
            stoppingy -= twoasquare;
            ellipseerror += ychange;
            ychange += twoasquare;
        }
    }
    
}

INLINE void strokeEllipse(PixelMap& pb, const int cx, const int cy, const size_t xradius, size_t yradius, const PixelRGBA &color)
{
    raster_ellipse(pb, cx, cy, xradius, yradius, color, Plot4EllipsePoints);
}


//
// Polygons, and triangles
// Triangles
//

INLINE static int findTopmostVertex(PixelCoord* verts, const int numVerts)
{
    int ymin= 65535;
    int vmin = 0;

    for (int i = 0; i < numVerts; i++) {
        if (verts[i].y() < ymin)
        {
            ymin = verts[i].y();
            vmin = i;
        }
    }

    return vmin;
}

struct APolyDda {
    short vertIndex;
    short vertNext;
    float x;
    float dx;
    short ybeg;
    short yend;


    void setupPolyDda(const PixelCoord* pVerts, const int numVerts, short ivert, int dir)
    {
        vertIndex = ivert;
        vertNext = ivert + dir;
        if (vertNext < 0) {
            vertNext = numVerts - 1;
        }
        else if (vertNext == numVerts) {
            vertNext = 0;
        }

        // set starting/ending ypos and current xpos
        ybeg = yend;
        //yend = round(pVerts[vertNext].y);
        yend = pVerts[vertNext].y();
        x = pVerts[vertIndex].x();

        // Calculate fractional number of pixels to step in x (dx)
        float xdelta = pVerts[vertNext].x() - pVerts[vertIndex].x();
        int ydelta = yend - ybeg;
        if (ydelta > 0) {
            dx = xdelta / ydelta;
        }
        else {
            dx = 0;
        }
    }
};

inline void setConvexPolygon(PixelMap& pb, PixelCoord* verts, const int nverts, int vmin, const PixelRGBA &color, const PixelRect& clipRect)
{
    // set starting line
    APolyDda ldda, rdda;
    int y = verts[vmin].y();
    ldda.yend = rdda.yend = y;

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
            int w = abs(rx - lx)+1;
            int x1 = lx < rx ? lx : rx;
            int x2 = x1 + w-1;

            if (clipLine({ 0,0,pb.width(), pb.height() }, x1, y1, x2, y2)) {
                w = x2 - x1;
                setSpan(pb, x1, y1, w, color);
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

inline void fillConvexPolygon(PixelMap& pb, PixelCoord* verts, const int nverts, int vmin, const PixelRGBA &color, const PixelRect& clipRect)
{
    setConvexPolygon(pb, verts, nverts, vmin, color, clipRect);
}

inline void fillTriangle(PixelMap& pb, const int x1, const int y1,
    const int x2, const int y2,
    const int x3, const int y3, const PixelRGBA &color, const PixelRect& clipRect)
{
    // Create a triangle object
    PixelTriangle tri(x1, y1, x2, y2, x3, y3);

    // find topmost vertex of the polygon
    int nverts = 3;
    int vmin = 0;

    setConvexPolygon(pb, tri.verts, nverts, vmin, color, clipRect);
}

// Draw the outline of a rectangle
INLINE void strokeRectangle(PixelMap& pmap, const int x, const int y, const int w, const int h, const PixelRGBA &c)
{
    // draw top and bottom
    line(pmap, x, y, x + w - 1, y, c);
    line(pmap, x, y + h - 1, x + w - 1, y + h - 1, c);

    // draw sides
    line(pmap, x, y, x, y + h - 1, c);
    line(pmap, x + w - 1, y, x + w - 1, y + h - 1, c);
}

// fill the inside of a rectangle
INLINE void fillRectangle(PixelMap& pmap, const int x, const int y, const int w, const int h, PixelRGBA c)
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
        setSpan(pmap, dstRect.x(), row, dstRect.w(), c);
    }
}

// A rather simple ellipse drawing routine
// not the most performant, but uses the 
// already available polygon filling routine
INLINE void fillEllipse(PixelMap& pmap, int centerx, int centery, int xRadius, int yRadius, PixelRGBA c)
{
    static const int nverts = 72;
    int steps = nverts;
    PixelCoord verts[nverts];

    int awidth = xRadius * 2;
    int aheight = yRadius * 2;

    for (int i = 0; i < steps; i++) {
        auto u = (double)i / steps;
        auto angle = u * (2 * maths::Pi);

        int x = (int)Floor((awidth / 2.0) * cos(angle));
        int y = (int)Floor((aheight / 2.0) * sin(angle));
        verts[i] = PixelCoord({ x + centerx, y + centery });
    }
    
    int vmin = findTopmostVertex(verts, nverts);
    setConvexPolygon(pmap, verts, nverts, vmin, c, { 0,0,canvasWidth,canvasHeight });
}

// filling a circle with a fixed color
//
INLINE void fillCircle(PixelMap& pmap, int centerX, int centerY, int radius, const PixelRGBA &fillStyle) 
{
    auto x1 = centerX - radius, y1 = centerY - radius;
    auto  x2 = centerX + radius, y2 = centerY + radius;
    for (int y = y1; y < y2; y++) {
        for (int x = x1; x < x2; x++) {
            auto distX = (x - centerX + 0.5), distY = (y - centerY + 0.5);
            auto distance = sqrt(distX * distX + distY * distY);
            if (distance <= radius) {
                //auto rgb = fillStyle(x, y);
                pmap.set(x, y, fillStyle);
            }
        }
    }
}







INLINE void bezier(PixelMap& pmap, GeoBezier<int> bez, int segments, PixelRGBA c)
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
INLINE void blit(PixelMap & pb, const int x, const int y, PixelMap & src)
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
    uint32_t* dstPtr = (uint32_t *)pb.getPixelPointer(dstX,dstY);

    int rowCount = 0;
    for (int srcrow = srcY; srcrow < srcY+ dstisect.h(); srcrow++)
    {
        uint32_t* srcPtr = (uint32_t*)src.getPixelPointer(srcX, srcrow);
        memcpy(dstPtr, srcPtr, dstisect.w() * 4);
        rowCount++;
        dstPtr = (uint32_t*)pb.getPixelPointer(dstisect.x(), dstisect.y()+rowCount);
    }
}


/*
// C99, public domain licensed 
// https://gist.github.com/ideasman42/983738130f754ef58ffa66bcdbbab892

#include <stdbool.h>


// utilities 

#define SWAP(type, a, b) do {  \
	type sw_ap;                \
	sw_ap = (a);               \
	(a) = (b);                 \
	(b) = sw_ap;               \
} while (0)

static INLINE int min_ii(int a, int b)
{
    return (a < b) ? a : b;
}

static INLINE int max_ii(int a, int b)
{
    return (b < a) ? a : b;
}

// sort edge-segments on y, then x axis 
static int fill_poly_v2i_n__span_y_sort(const void* a_p, const void* b_p, void* verts_p)
{
    const int(*verts)[2] = verts_p;
    const int* a = a_p;
    const int* b = b_p;
    const int* co_a = verts[a[0]];
    const int* co_b = verts[b[0]];

    if (co_a[1] < co_b[1]) {
        return -1;
    }
    else if (co_a[1] > co_b[1]) {
        return 1;
    }
    else if (co_a[0] < co_b[0]) {
        return -1;
    }
    else if (co_a[0] > co_b[0]) {
        return 1;
    }
    else {
        // co_a & co_b are identical, use the line closest to the x-min 
        const int* co = co_a;
        co_a = verts[a[1]];
        co_b = verts[b[1]];
        int ord = (((co_b[0] - co[0]) * (co_a[1] - co[1])) -
            ((co_a[0] - co[0]) * (co_b[1] - co[1])));
        if (ord > 0) {
            return -1;
        }
        if (ord < 0) {
            return 1;
        }
    }
    return 0;
}

//
 // \param callback: Takes the x, y coords and x-span (\a x_end is not inclusive),
 // note that \a x_end will always be greater than \a x, so we can use:
 //
 // \code{.c}
 // do {
 //     func(x, y);
 // } while (++x != x_end);
 // \endcode
 //
void fill_poly_v2i_n(const PixelMap &pmap,
    const int verts[][2], const int nr,
    void (*callback)(int x, int x_end, int y, void*), void* user_data)
{
    int xmin = pmap.x();
    int ymin = pmap.y();
    int xmax = pmap.right();
    int ymax = pmap.bottom();

    // Originally by Darel Rex Finley, 2007.
    // Optimized by Campbell Barton, 2016 to keep sorted intersections. 

    int(*span_y)[2] = malloc(sizeof(*span_y) * (size_t)nr);
    int span_y_len = 0;

    for (int i_curr = 0, i_prev = nr - 1; i_curr < nr; i_prev = i_curr++) {
        const int* co_prev = verts[i_prev];
        const int* co_curr = verts[i_curr];

        if (co_prev[1] != co_curr[1]) {
            // Any segments entirely above or below the area of interest can be skipped. 
            if ((min_ii(co_prev[1], co_curr[1]) >= ymax) ||
                (max_ii(co_prev[1], co_curr[1]) < ymin))
            {
                continue;
            }

            int* s = span_y[span_y_len++];
            if (co_prev[1] < co_curr[1]) {
                s[0] = i_prev;
                s[1] = i_curr;
            }
            else {
                s[0] = i_curr;
                s[1] = i_prev;
            }
        }
    }

    qsort_r(span_y, (size_t)span_y_len, sizeof(*span_y), fill_poly_v2i_n__span_y_sort, (void*)verts);

    struct NodeX {
        int span_y_index;
        int x;
    } *node_x = malloc(sizeof(*node_x) * (size_t)nr, __func__);
    int node_x_len = 0;

    int span_y_index = 0;
    if (span_y_len != 0 && verts[span_y[0][0]][1] < ymin) {
        while ((span_y_index < span_y_len) &&
            (verts[span_y[span_y_index][0]][1] < ymin))
        {
            assert(verts[span_y[span_y_index][0]][1] <
                verts[span_y[span_y_index][1]][1]);
            if (verts[span_y[span_y_index][1]][1] >= ymin) {
                struct NodeX* n = &node_x[node_x_len++];
                n->span_y_index = span_y_index;
            }
            span_y_index += 1;
        }
    }

    // Loop through the rows of the image. 
    for (int pixel_y = ymin; pixel_y < ymax; pixel_y++) {
        bool is_sorted = true;
        bool do_remove = false;

        for (int i = 0, x_ix_prev = INT_MIN; i < node_x_len; i++) {
            struct NodeX* n = &node_x[i];
            const int* s = span_y[n->span_y_index];
            const int* co_prev = verts[s[0]];
            const int* co_curr = verts[s[1]];

            assert(co_prev[1] < pixel_y && co_curr[1] >= pixel_y);

            const double x = (co_prev[0] - co_curr[0]);
            const double y = (co_prev[1] - co_curr[1]);
            const double y_px = (pixel_y - co_curr[1]);
            const int    x_ix = (int)round((double)co_curr[0] + ((y_px / y) * x));
            n->x = x_ix;

            if (is_sorted && (x_ix_prev > x_ix)) {
                is_sorted = false;
            }
            if (do_remove == false && co_curr[1] == pixel_y) {
                do_remove = true;
            }
            x_ix_prev = x_ix;
        }

        // Sort the nodes, via a simple "Bubble" sort. 
        if (is_sorted == false) {
            int i = 0;
            const int current_end = node_x_len - 1;
            while (i < current_end) {
                if (node_x[i].x > node_x[i + 1].x) {
                    SWAP(struct NodeX, node_x[i], node_x[i + 1]);
                    if (i != 0) {
                        i -= 1;
                    }
                }
                else {
                    i += 1;
                }
            }
        }

        // Fill the pixels between node pairs. 
        for (int i = 0; i < node_x_len; i += 2) {
            int x_src = node_x[i].x;
            int x_dst = node_x[i + 1].x;

            if (x_src >= xmax) {
                break;
            }

            if (x_dst > xmin) {
                if (x_src < xmin) {
                    x_src = xmin;
                }
                if (x_dst > xmax) {
                    x_dst = xmax;
                }
                // for single call per x-span 
                if (x_src < x_dst) {
                    callback(x_src - xmin, x_dst - xmin, pixel_y - ymin, user_data);
                }
            }
        }

        // Clear finalized nodes in one pass, only when needed
         // (avoids excessive array-resizing). 
        if (do_remove == true) {
            int i_dst = 0;
            for (int i_src = 0; i_src < node_x_len; i_src += 1) {
                const int* s = span_y[node_x[i_src].span_y_index];
                const int* co = verts[s[1]];
                if (co[1] != pixel_y) {
                    if (i_dst != i_src) {
                        // x is initialized for the next pixel_y (no need to adjust here) 
                        node_x[i_dst].span_y_index = node_x[i_src].span_y_index;
                    }
                    i_dst += 1;
                }
            }
            node_x_len = i_dst;
        }

        // scan for new x-nodes 
        while ((span_y_index < span_y_len) &&
            (verts[span_y[span_y_index][0]][1] == pixel_y))
        {
            // note, node_x these are just added at the end,
            // not ideal but sorting once will resolve. 

             // x is initialized for the next pixel_y 
            struct NodeX* n = &node_x[node_x_len++];
            n->span_y_index = span_y_index;
            span_y_index += 1;
        }
    }

    free(span_y);
    free(node_x);
}
*/