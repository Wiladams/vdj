//
// Transition explorer
// // Raytrace reference: https://sergeyreznik.github.io/rtcode/index.html
//

#include "gui.h"
#include "draw2dcontext.hpp"
#include "screensnapshot.hpp"
#include "recorder.h"
#include "stopwatch.h"
#include "checkerboard.hpp"

#include "animator.hpp"
#include "tranexwindow.hpp"


using namespace vdj;
using namespace alib;


std::shared_ptr<ScreenSnapshot> screenCapture = nullptr;
SharedSamplerWrapper screenCap1 = nullptr;
SharedSamplerWrapper screenCap2 = nullptr;

//constexpr ptrdiff_t WINDOWWIDTH = 800;
//constexpr ptrdiff_t WINDOWHEIGHT = 600;

//constexpr ptrdiff_t WINDOWWIDTH = 1024;
//constexpr ptrdiff_t WINDOWHEIGHT = 768;

constexpr ptrdiff_t WINDOWWIDTH = 1024;
constexpr ptrdiff_t WINDOWHEIGHT = 768;


TranexWindow mainWindow(WINDOWWIDTH, WINDOWHEIGHT);

static PixelRect randomRect(size_t dw, size_t dh, const PixelRect& bounds)
{
	size_t w = alib::random_int(2, dw);
	size_t h = alib::random_int(2, dh);
	ptrdiff_t x = alib::random_int(bounds.fX, bounds.fX+bounds.fWidth - 1);
	ptrdiff_t y = alib::random_int(bounds.fY, bounds.fY + bounds.fHeight - 1);

	return PixelRect(x, y, w, h);
}

static PixelRGBA randomPixel()
{
	return PixelRGBA(alib::random_int(0, 255), alib::random_int(0, 255), alib::random_int(0, 255), 255);
}

void drawRects(size_t w, size_t h, const PixelRect &bounds, size_t count=5000)
{
	screenCapture->next();

	for (size_t i = 1; i <= count; i++) {
		PixelRect r = randomRect(w, h, bounds);
		auto c = r.center();
		double u = (double)c.fX / (canvasWidth-1);
		double v = (double)c.fY / (canvasHeight-1);

		auto c1 = randomPixel();
		//auto c2 = randomPixel();
		
		PixelRGBA sc = screenCap1->getValue(u, v);

		gCtxt.fillRectangle(r.left(), r.top(), r.fWidth, r.fHeight, sc);

		//strokeRectangle(*gAppSurface, r, sc);
	}
}

void drawRectAffect()
{
	drawRects(64, 64, PixelRect(0, 0, canvasWidth, canvasHeight), 1000);
	gCtxt.fillRectangle(200, 200, canvasWidth - 400, canvasHeight - 400, PixelRGBA(0));
	drawRects(4, 4, PixelRect(200, 200, canvasWidth - 400, canvasHeight - 400), 30000);
}

void onFrame()
{
	gCtxt.clear(PixelRGBA(0xffdddddd));

	mainWindow.draw(gCtxt);
	//drawRectAffect();
}

void setup()
{
	setFrameRate(15);
	setCanvasSize(1920, 1080);
	//layered();

	// Setup screen captures
	screenCapture = ScreenSnapshot::createForDisplay(0, 0, displayWidth, 1080);
	screenCap1 = std::make_shared<vdj::SamplerWrapper>(screenCapture, alib::RectD(0, 0, 0.5, 1.0));
	screenCap2 = std::make_shared<vdj::SamplerWrapper>(screenCapture, alib::RectD(0.50, 0, 0.5, 1.0));
}