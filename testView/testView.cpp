
#include "gui.h"
#include "screensnapshot.hpp"
#include "normalizedwindow.hpp"

using namespace vdj;
using namespace alib;

constexpr size_t CAPTURE_WIDTH = 600;
constexpr size_t CAPTURE_HEIGHT = 600;

constexpr size_t CANVAS_WIDTH = 1920;
constexpr size_t CANVAS_HEIGHT = 1200;


// Source Samplers
std::shared_ptr<ScreenSnapshot> screenCapture = nullptr;
std::shared_ptr< SamplerWrapper> screenCap1 = nullptr;
std::shared_ptr< SamplerWrapper> screenCap2 = nullptr;

// generate a list of 't' values that will give 
// equal distance
template <typename T>
double findUForX(const GeoCubicBezier<T>& bez, ptrdiff_t x)
{
	double highEnd = 1.0;
	double lowEnd = 0.0;

	// Binary search to find the solution
	while (true)
	{
		double u = highEnd - ((highEnd - lowEnd) / 2.0);

		ptrdiff_t evalX = bez.eval(u).fX;

		if (evalX == x)
			return u;

		if (evalX > x)
		{
			highEnd = u;
		} else if (evalX < x)
		{
			lowEnd = u;
		}
	}
}

template <typename T>
void evalBezSpeed(const GeoCubicBezier<T>& bez, std::vector<T> &tvals)
{
	// Figure out lowest value for t==0
	// Figure out highest value for t == 1.0

	auto p1 = bez.eval(0);
	auto p2 = bez.eval(1.0);

	// do the loop searching for best fits
	for (size_t x = p1.x(); x <= p2.x(); x++)
	{
		auto u = findUForX(bez, x);

		tvals.push_back(u);
	}
}


// 
// Draw a bezier line using a single line sampler
template <typename T>
void sampledBezierLoft(PixelView& pmap, const GeoCubicBezier<T>& c1, const ptrdiff_t loft, ISample2D<PixelRGBA>& c)
{
	auto p1 = c1.eval(0);

	std::vector<T> tvals;
	evalBezSpeed(c1, tvals);

	for (size_t i = 0; i < tvals.size(); i++)
	{
		auto u = tvals[i];
		p1 = c1.eval(u);

		ptrdiff_t y2 = p1.y() + loft;

		// sample vertical line
		for (ptrdiff_t y = p1.y(); y <= y2; y++)
		{
			auto v = alib::Map(y, p1.y(), y2, 0.0, 1.0);
			pmap.getPixel(p1.x(), y) = c.getValue(u, v);
		}
	}


}

template <typename T>
void sampledBezierSurface(PixelView& pmap, const GeoCubicBezier<T>& c1, const GeoCubicBezier<T>& c2, ISample2D<PixelRGBA>& c)
{
	auto p1 = c1.eval(0);
	auto p2 = c2.eval(0);

	auto endp1 = c1.eval(1.0);
	auto endp2 = c2.eval(1.0);

	std::vector<T> tvals1;
	evalBezSpeed(c1, tvals1);

	std::vector<T> tvals2;
	evalBezSpeed(c1, tvals2);

	for (size_t i = 0; i < tvals1.size(); i++)
	{
		auto u1 = tvals1[i];
		p1 = c1.eval(u1);
		p2 = c2.eval(u1);

		// sample vertical line
		for (ptrdiff_t y = p1.y(); y <= p2.y(); y++)
		{
			auto v = alib::Map(y, p1.y(), p2.y(), 0.0, 1.0);
			pmap.getPixel(p1.x(), y) = c.getValue(u1, v);
		}
	}
}

void drawLines()
{
	for (size_t i = 0; i < canvasWidth; i+=10)
	{
		ptrdiff_t x1 = i;
		ptrdiff_t y1 = 0;
		ptrdiff_t x2 = canvasWidth;
		ptrdiff_t y2 = i;

		gCtxt.line(x1, y1, x2, y2, PixelRGBA(0xff000000), 1);
	}

}

void onFrame()
{
	drawLines();

	screenCapture->next();

	GeoCubicBezier<float> c1(1, 1, (canvasWidth*0.25), (canvasHeight/2.0), canvasWidth-(canvasWidth*0.25), (canvasHeight/2.0), canvasWidth-1, 1);
	GeoCubicBezier<float> c2(1, canvasHeight-1, (canvasWidth * 0.25), (canvasHeight / 2.0), canvasWidth - (canvasWidth * 0.25), (canvasHeight / 2.0), canvasWidth-1, canvasHeight-1);


	//PixelBezier c1(10, 300, 300, 0, 600, 250, 800, 150);
	//PixelBezier c2(10, 500, 300, 600, 600, 450, 800, 750);

	//PixelBezier c1(10, 100, 300,   0, 600,  50, 800, 150);
	//PixelBezier c2(10, 700, 300, 600, 600, 450, 800, 750);
	//sampledBezierSurface(*gAppSurface, c1, c2, *screenCapture);
	//sampledBezierLoft(*gAppSurface, c1, CAPTURE_HEIGHT, vs);

	gCtxt.strokeCubicBezier(c1, 60, PixelRGBA(0));
	gCtxt.strokeCubicBezier(c2, 60, PixelRGBA(0));

}

void setup()
{
	setCanvasSize(CANVAS_WIDTH, CANVAS_HEIGHT);

	// Setup screen captures
	screenCapture = ScreenSnapshot::createForDisplay(0, 0, displayWidth/2, displayHeight/2);
	//ViewSampler vs(*screenCapture);

	//screenCap1 = std::make_shared<vdj::SamplerWrapper>(screenCapture, alib::RectD(0, 0, 0.5, 1.0));
	//screenCap2 = std::make_shared<vdj::SamplerWrapper>(screenCapture, alib::RectD(0.50, 0, 0.5, 1.0));

	gCtxt.clear(PixelRGBA(0xffff00ff));
}