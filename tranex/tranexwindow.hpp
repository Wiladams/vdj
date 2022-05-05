#pragma once

#include "gui.h"
//#include "draw2dcontext.hpp"
#include "normalizedwindow.hpp"
#include "checkerboard.hpp"
#include "svgpathbuilder.hpp"
//#include "sampledraw2d.hpp"

#include "animator.hpp"


//CheckerSampler checkers(64, PixelRGBA(0xff1f1f1f), PixelRGBA(0xffDDDDDD));
alib::CheckerSampler checkers(64, alib::PixelRGBA(0xAA, 0xFF), alib::PixelRGBA(0xDD, 0xff));

namespace pathdraw
{
	//===== Discreet Graphic Elements =====
	// Add a discrete line
	template <typename T>
	INLINE void line(alib::SVGPathBuilder<T>& spb, const T& x1, const T& y1, const T& x2, const T& y2)
	{
		spb.addCommand(alib::SVGPathBuilder<T>::ContourCommand::Line);
		spb.addPoint(Point<T>(x1, y1));
		spb.addPoint(Point<T>(x2, y2));
	}

	template <typename T>
	INLINE void line(alib::SVGPathBuilder<T>& spb, const alib::Point<T>& p1, const alib::Point<T>& p2)
	{
		spb.line(p1.x(), p1.y(), p2.x(), p2.y());
	}

	template <typename T>
	INLINE void rect(alib::SVGPathBuilder<T>& spb, const T x, const T y, const T w, const T h)
	{
		spb.moveTo(x, y);
		spb.lineTo(x + w - 1, y);
		spb.lineTo(x + w - 1, y + h - 1);
		spb.lineTo(x, y + h - 1);
		spb.close();
	}

	template <typename T>
	INLINE void ellipse(alib::SVGPathBuilder<T>& spb, const T cx, const T cy, const T radX, const T radY)
	{
		spb.moveTo(cx - radX, cy);
		spb.cubicTo(cx - radX, cy - radY, cx + radX, cy - radY, cx + radX, cy);
		spb.cubicTo(cx + radX, cy + radY, cx - radX, cy + radY, cx - radX, cy);
		spb.close();
	}

	template <typename T>
	INLINE void circle(alib::SVGPathBuilder<T>& spb, const T cx, const T cy, const T rad)
	{
		ellipse(spb, cx, cy, rad, rad);
	}
}


struct TranexWindow : public vdj::SampledWindow<vdj::SamplerWrapper>
{
    alib::PixelRect fWindowFrame;

	TranexWindow(ptrdiff_t w, ptrdiff_t h)
		:fWindowFrame(0, 0, w, h) {}

	void draw(alib::Draw2DContext& ctxt);
};

/*
TranexWindow::TranexWindow(ptrdiff_t w, ptrdiff_t h)
    :fWindowFrame(0,0,w,h)
{
	//this->addChild();
    
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

}
*/



void drawShapes(alib::Draw2DContext& ctxt)
{
	// Create some shapes to draw
	alib::SVGPathBuilder<alib::Real> sb;
	

	// Random concave polygon
	sb.moveTo(10,10);
	sb.lineTo(20, 10);
	sb.lineTo(20, 20);
	sb.lineTo(10, 20);
	sb.close();


//*
	// Some rectangles
	pathdraw::rect<alib::Real>(sb, 8, 8, 64, 1080);
	pathdraw::rect<alib::Real>(sb, 80, 8, 1920/2, 1080/2);
	pathdraw::rect<alib::Real>(sb, 64, 1092, 1920, 24);
	pathdraw::rect<alib::Real>(sb, 992 - 120, 1116 + 8, 240, 48);
	


	// Rectangle
	pathdraw::rect<alib::Real>(sb, 60, 100, 200, 200);

	// Ellipse
	pathdraw::ellipse<alib::Real>(sb, 400, 100, 200, 100);

	// Full circle
	pathdraw::circle<alib::Real>(sb, 300, 300, 100);

	// draw some lines
	pathdraw::line<alib::Real>(sb, 10, 408, 600, 408);
	pathdraw::line<alib::Real>(sb, 10, 416, 600, 416);
	pathdraw::line<alib::Real>(sb, 10, 424, 600, 424);
	

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
		//*/

	auto figs = sb.getPaths();

	for (auto& pth : figs)
	{
		// fill and stroke a polygon
		ctxt.fillPolygon(pth, alib::PixelRGBA(0xffffff00));
		//ctxt.strokePolygon(pth, alib::PixelRGBA(0xffff0000), 1);
	}
}

void TranexWindow::draw(alib::Draw2DContext & ctxt)
{
    // draw background
	ctxt.fillRectangle(alib::PixelRect(0,0,canvasWidth, canvasHeight), alib::RectD(0,0,1,1), checkers);

    // draw children
	//ctxt.fillRectangle(alib::PixelRect(0,0,canvasWidth, canvasHeight), alib::RectD(0,0,1,1), *this);

	drawShapes(ctxt);
}