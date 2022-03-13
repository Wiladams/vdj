//
// Transition explorer
// // Raytrace reference: https://sergeyreznik.github.io/rtcode/index.html
//

#include "gui.h"
#include "screensnapshot.hpp"
#include "recorder.h"
#include "stopwatch.h"
#include "checkerboard.hpp"

#include "animator.hpp"
#include "tranexwindow.hpp"


using namespace vdj;

std::shared_ptr<ScreenSnapshot> screenCapture = nullptr;
SharedSamplerWrapper screenCap1 = nullptr;
SharedSamplerWrapper screenCap2 = nullptr;

//constexpr ptrdiff_t WINDOWWIDTH = 1024;
//constexpr ptrdiff_t WINDOWHEIGHT = 768;

constexpr ptrdiff_t WINDOWWIDTH = 1988;
constexpr ptrdiff_t WINDOWHEIGHT = 1200;


TranexWindow mainWindow(WINDOWWIDTH, WINDOWHEIGHT);

void onFrame()
{
	background(PixelRGBA(0xff00ff00));

	mainWindow.draw(*gAppSurface);
}

void setup()
{
	setCanvasSize(WINDOWWIDTH, WINDOWHEIGHT);

	// Setup screen captures
	screenCapture = ScreenSnapshot::createForDisplay(0, 0, displayWidth, 1080);
	screenCap1 = std::make_shared<vdj::SamplerWrapper>(screenCapture, vdj::RectD(0, 0, 0.5, 1.0));
	screenCap2 = std::make_shared<vdj::SamplerWrapper>(screenCapture, vdj::RectD(0.50, 0, 0.5, 1.0));


}