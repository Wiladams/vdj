
#include "gui.h"
#include "screensnapshot.hpp"
#include "normalizedwindow.hpp"
#include "maths.hpp"

using namespace vdj;
using namespace alib;

constexpr size_t CANVAS_WIDTH = 1280;
constexpr size_t CANVAS_HEIGHT = 1080;

constexpr size_t CAPTURE_WIDTH = 1280;
constexpr size_t CAPTURE_HEIGHT = 1080;

// Source Samplers
std::shared_ptr<ScreenSnapshot> screenCapture = nullptr;
std::shared_ptr< SamplerWrapper> screenCap1 = nullptr;
std::shared_ptr< SamplerWrapper> screenCap2 = nullptr;

//GeoCubicBezier<float> c1(1, 1, (CANVAS_WIDTH * 0.25), (CANVAS_HEIGHT / 2.0), CANVAS_WIDTH - (CANVAS_WIDTH * 0.25), (CANVAS_HEIGHT / 2.0), CANVAS_WIDTH - 1, 1);
//GeoCubicBezier<float> c2(1, CANVAS_HEIGHT - 1, (CANVAS_WIDTH * 0.25), (CANVAS_HEIGHT / 2.0), CANVAS_WIDTH - (CANVAS_WIDTH * 0.25), (CANVAS_HEIGHT / 2.0), CANVAS_WIDTH - 1, CANVAS_HEIGHT - 1);

//GeoCubicBezier<Real> c1(0, CANVAS_HEIGHT*0.25, (CANVAS_WIDTH * 0.25), 0, CANVAS_WIDTH - (CANVAS_WIDTH * 0.25), 0, CANVAS_WIDTH - 1, CANVAS_HEIGHT * 0.25);

//pinched in the middle
GeoCubicBezier<Real> c1(1, 1, (CANVAS_WIDTH * 0.25), (CANVAS_HEIGHT *0.25), CANVAS_WIDTH - (CANVAS_WIDTH * 0.25), (CANVAS_HEIGHT * 0.25), CANVAS_WIDTH - 1, 1);
GeoCubicBezier<Real> c2(1, CANVAS_HEIGHT - 1, (CANVAS_WIDTH * 0.25), (CANVAS_HEIGHT * 0.75), CANVAS_WIDTH - (CANVAS_WIDTH * 0.25), (CANVAS_HEIGHT * 0.75), CANVAS_WIDTH - 1, CANVAS_HEIGHT - 1);


// 
// Draw a bezier line using a single line sampler
//template <typename T>
void sampledBezierLoft(agg::rendering_buffer & pmap, const GeoCubicBezier<Real>& c1, const ptrdiff_t loft, ISample2D<PixelRGBA>& c)
{
	auto p1 = c1.eval(0);

	for (size_t i = 0; i < c1.tvals.size(); i++)
	{
		auto u = c1.tvals[i];
		p1 = c1.eval(u);

		ptrdiff_t y2 = p1.y() + loft;

		// sample vertical line
		for (ptrdiff_t y = p1.y(); y <= y2; y++)
		{
			auto v = (y-p1.y()) / (double)loft;
			((PixelRGBA*)pmap.row_ptr(y))[(ptrdiff_t)p1.x()] = c.getValue(u, v);
		}
	}
}

// draw a texture between two curves
// The curves should be linear in the x direction
// c1 should be on top and c2 on bottom
// but this is not strictly required
// This is a very specialized drawing routine
// so it should not be a part of the drawingcontext
//template <typename T>
void sampledBezierSurface(agg::row_accessor<uint8_t>& pmap, const GeoCubicBezier<Real>& c1, const GeoCubicBezier<Real>& c2, ISample2D<PixelRGBA>& c)
{
	auto p1 = c1.eval(0);
	auto p2 = c2.eval(0);

	auto endp1 = c1.eval(1.0);
	auto endp2 = c2.eval(1.0);

	for (size_t i = 0; i < c1.tvals.size(); i++)
	{
		auto u1 = c1.tvals[i];
		p1 = c1.eval(u1);
		p2 = c2.eval(u1);

		// sample vertical line
		for (ptrdiff_t y = p1.y(); y <= p2.y(); y++)
		{
			auto v = alib::Map(y, p1.y(), p2.y(), 0.0, 1.0);
			//auto v = (y-p1.y()) / (p2.y()-p1.y());
			//((PixelRGBA *)pmap.row_ptr(y))[(ptrdiff_t)p1.x()] = c.getValue(u1, v);
			auto cvalue = c.getValue(u1,v);

			// Data is in BGRA format
			auto data = pmap.row_ptr(y) + (ptrdiff_t)p1.x() * 4;
			data[0] = cvalue.b;
			data[1] = cvalue.g;
			data[2] = cvalue.r;
			data[3] = cvalue.a;
		}
	}
}

// Draw some random lines
void drawLines()
{
	static double u = 0.0;
	static double v = 0.0;

	double dx = 1.0 / (frameRate);

	for (size_t ctr = 1; ctr <= 10; ctr++)
	{
		static size_t i = 0;
		ptrdiff_t x1 = alib::Map(u, 0, 1, 0, canvasWidth);
		ptrdiff_t y1 = 0;
		ptrdiff_t x2 = alib::Map(u, 0, 1, canvasWidth, 0);
		ptrdiff_t y2 = canvasHeight;

		gCtxt.line(x1, y1, x2, y2, PixelRGBA(0xff,0x00,0x00), 1);

		u += dx;
		if (u > 1.0)
			u = 0.0;
	}
}

void drawShapes()
{
	drawLines();
}

void onFrame()
{
	// Capture the current screen
	screenCapture->next();

	// clear our canvas
	background(PixelRGBA(0, 0, 0));

	//drawShapes();

	sampledBezierSurface(*gAppSurface, c1, c2, *screenCapture);
	//sampledBezierLoft(*gAppSurface, c1, CAPTURE_HEIGHT/2, *screenCapture);

	//gCtxt.strokeCubicBezier(c1, 60, PixelRGBA(0,0,0));
	//gCtxt.strokeCubicBezier(c2, 60, PixelRGBA(0,0,0));

	//gCtxt.sampleRect(PixelRect(0, 0, canvasWidth, canvasHeight), RectD(0, 0, 1.0, 1.0), *screenCapture);
}

void setup()
{
	c1.calcSpeeds();
	c2.calcSpeeds();

	setCanvasSize(CANVAS_WIDTH, CANVAS_HEIGHT);
	setFrameRate(15);

	// Setup screen captures
	screenCapture = ScreenSnapshot::createForDisplay(0, 0, CAPTURE_WIDTH, CAPTURE_HEIGHT);

	//screenCap1 = std::make_shared<vdj::SamplerWrapper>(screenCapture, alib::RectD(0, 0, 0.5, 1.0));
	//screenCap2 = std::make_shared<vdj::SamplerWrapper>(screenCapture, alib::RectD(0.50, 0, 0.5, 1.0));

	background(PixelRGBA(0, 0, 0));

	//windowOpacity(0.5f);
	//layered();
}