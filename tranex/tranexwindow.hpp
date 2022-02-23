#pragma once

#include "gui.h"
#include "normalizedwindow.hpp"
#include "checkerboard.hpp"
#include "svgpathbuilder.hpp"

#include "animator.hpp"

using namespace vdj;

//CheckerSampler checkers(64, PixelRGBA(0xff1f1f1f), PixelRGBA(0xffDDDDDD));
CheckerSampler checkers(64, PixelRGBA(0xffDDDDDD), PixelRGBA(0xffDDDDDD));

//===== Discreet Graphic Elements =====
// Add a discrete line
template <typename T>
void line(SVGPathBuilder<T> &spb, const T& x1, const T& y1, const T& x2, const T& y2)
{
	spb.addCommand(SVGPathBuilder<T>::ContourCommand::Line);
	spb.addPoint(Point<T>(x1, y1));
	spb.addPoint(Point<T>(x2, y2));
}

template <typename T>
void line(SVGPathBuilder<T>& spb, const Point<T>& p1, const Point<T>& p2)
{
	spb.line(p1.x(), p1.y(), p2.x(), p2.y());
}

template <typename T>
void rect(SVGPathBuilder<T>& spb, const T x, const T y, const T w, const T h)
{
	spb.moveTo(x, y);
	spb.lineTo(x + w - 1, y);
	spb.lineTo(x + w - 1, y + h - 1);
	spb.lineTo(x, y + h - 1);
	spb.close();
}

template <typename T>
void ellipse(SVGPathBuilder<T>& spb, const T cx, const T cy, const T radX, const T radY)
{
	spb.moveTo(cx - radX, cy);
	spb.cubicTo(cx - radX, cy - radY, cx + radX, cy - radY, cx + radX, cy);
	spb.cubicTo(cx + radX, cy + radY, cx - radX, cy + radY, cx - radX, cy);
	spb.close();
}

template <typename T>
void circle(SVGPathBuilder<T>& spb, const T cx, const T cy, const T rad)
{
	ellipse(spb, cx, cy, rad, rad);
}



struct TranexWindow : public SampledWindow<SamplerWrapper>
{
    PixelRect fWindowFrame;

	TranexWindow(ptrdiff_t w, ptrdiff_t h);

	void draw(PixelView &ctx);
};

TranexWindow::TranexWindow(ptrdiff_t w, ptrdiff_t h)
    :SampledWindow()
    ,fWindowFrame(0,0,w,h)
{
    /*
    // Area for selecting effects
	PixelRect tranSelector(8,8,64,1080);
	fillRectangle(*gAppSurface, tranSelector, PixelRGBA(0xFF101010));

	// Area showing transition
	PixelRect displayArena(80, 8, 1920/2, 1080/2);
	//fillRectangle(*gAppSurface, displayArena, PixelRGBA(0xFF1f1f00));
	sampleRect(*gAppSurface, displayArena, RectD(0, 0, 1, 1), *screenCap1);

	// Transition slider
	PixelRect tranSlide(64, 1092, 1920, 24);
	fillRectangle(*gAppSurface, tranSlide, PixelRGBA(0xFF1fCf00));

	PixelRect recordo(992-120, 1116 + 8, 240, 48);
	fillRectangle(*gAppSurface, recordo, PixelRGBA(0xFF000fcf));
*/
}

void drawShapes()
{
	// Create some shapes to draw
	SVGPathBuilder<ptrdiff_t> sb;
	
	// Some rectangles
	rect<ptrdiff_t>(sb, 8, 8, 64, 1080);
	rect<ptrdiff_t>(sb, 80, 8, 1920/2, 1080/2);
	rect<ptrdiff_t>(sb, 64, 1092, 1920, 24);
	rect<ptrdiff_t>(sb, 992 - 120, 1116 + 8, 240, 48);
	
	/*
	// Random concave polygon
	sb.moveTo(10,10);
	sb.lineTo(600, 30);
	sb.lineTo(400, 300);
	sb.lineTo(600, 400);
	sb.lineTo(10, 400);
	sb.close();
	*/

	// Rectangle
	rect<ptrdiff_t>(sb, 60, 100, 200, 200);

	// Ellipse
	ellipse<ptrdiff_t>(sb, 400, 100, 200, 100);

	// Full circle
	circle<ptrdiff_t>(sb, 300, 300, 100);

	// draw some lines
	line<ptrdiff_t>(sb, 10, 408, 600, 408);
	line<ptrdiff_t>(sb, 10, 416, 600, 416);
	line<ptrdiff_t>(sb, 10, 424, 600, 424);
	

	// Create a tabbed panel using quadratic bezier
	// and lineTo
	sb.moveTo(10, 440);
	sb.lineTo(100, 440);

	sb.quadTo(100, 420, 108, 420);
	//sb.lineTo(100, 420);
	//sb.lineTo(120, 420);

	sb.lineTo(200, 420);

	sb.quadTo(220, 420, 220, 440);
	//sb.lineTo(220, 420);
	//sb.lineTo(220, 440);

	sb.lineTo(300, 440);
	sb.lineTo(300, 480);
	sb.lineTo(10, 480);
	sb.close();

	auto figs = sb.getFigures();

	for (auto& fig : figs)
	{
		// fill and stroke a polygon
		fillPolygon(*gAppSurface, fig, vdj::PixelRGBA(0xffffff00));
		//strokePolygon(*gAppSurface, fig, vdj::PixelRGBA(0xff00ff00), 1);
		strokePolygon(*gAppSurface, fig, vdj::PixelRGBA(0xffff0000), 1);
	}



}

void TranexWindow::draw(PixelView &ctx)
{
    // draw background
	sampleRect(ctx, PixelRect(0,0,canvasWidth, canvasHeight), RectD(0,0,1,1), checkers);

    // draw children
	sampleRect(ctx, PixelRect(0,0,canvasWidth, canvasHeight), RectD(0,0,1,1), *this);

	drawShapes();
}