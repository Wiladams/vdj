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

using namespace vdj;

CheckerSampler checkers(64, PixelRGBA(0xff1f1f1f), PixelRGBA(0xffffffff));

std::shared_ptr<ScreenSnapshot> screenCapture = nullptr;
SharedSamplerWrapper screenCap1 = nullptr;
SharedSamplerWrapper screenCap2 = nullptr;

void onFrame()
{
	// Do a background of some sort
	//background(PixelRGBA(0xffffffff));
	sampleRect(*gAppSurface, PixelRect(0, 0, canvasWidth, canvasHeight), RectD(0, 0, 1, 1), checkers);

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

void setup()
{
	setCanvasSize(1988, 1200);

	// Setup screen captures
	screenCapture = ScreenSnapshot::createForDisplay(0, 0, displayWidth, 1080);
	screenCap1 = std::make_shared<vdj::SamplerWrapper>(screenCapture, vdj::RectD(0, 0, 0.5, 1.0));
	screenCap2 = std::make_shared<vdj::SamplerWrapper>(screenCapture, vdj::RectD(0.50, 0, 0.5, 1.0));

}