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

constexpr ptrdiff_t WINDOWWIDTH = 1988;
constexpr ptrdiff_t WINDOWHEIGHT = 1200;


TranexWindow mainWindow(WINDOWWIDTH, WINDOWHEIGHT);

static PixelRect randomRect(size_t dw, size_t dh)
{
	size_t w = alib::random_int(4, dw);
	size_t h = alib::random_int(4, dh);
	ptrdiff_t x = alib::random_int(0, canvasWidth - w - 1);
	ptrdiff_t y = alib::random_int(0, canvasHeight - h - 1);

	return PixelRect(x, y, w, h);
}

static PixelRGBA randomPixel()
{
	return PixelRGBA(alib::random_int(0, 255), alib::random_int(0, 255), alib::random_int(0, 255), 255);
}

void drawRects()
{


	for (size_t i = 1; i <= 1000; i++) {
		auto r = randomRect(60, 60);
		auto c1 = randomPixel();
		auto c2 = randomPixel();

		fillRectangle(*gAppSurface, r, c1);
		//strokeRectangle(*gAppSurface, r, c2);
	}
}

void onFrame()
{
	background(PixelRGBA(0xff00ff00));
	gCtxt.clear(PixelRGBA(0xff00ff00));

	mainWindow.draw(gCtxt);

	drawRects();
}

void setup()
{
	setFrameRate(30);
	setCanvasSize(WINDOWWIDTH, WINDOWHEIGHT);
	//layered();
	gCtxt.setView(gAppSurface);

	// Setup screen captures
	screenCapture = ScreenSnapshot::createForDisplay(0, 0, displayWidth, 1080);
	screenCap1 = std::make_shared<vdj::SamplerWrapper>(screenCapture, alib::RectD(0, 0, 0.5, 1.0));
	screenCap2 = std::make_shared<vdj::SamplerWrapper>(screenCapture, alib::RectD(0.50, 0, 0.5, 1.0));
}