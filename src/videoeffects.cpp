//
// Exercise and develop various kinds of visual effects
//
#include "gui.h"
#include "screensnapshot.hpp"
#include "recorder.h"
#include "stopwatch.h"
#include "sampledraw2d.hpp"
#include "windowcaptures.h"
#include "xmlparse.hpp"

#include "effect_barndoor.h"
#include "effect_crossfade.h"
#include "effect_corners.h"
#include "effect_fingers.h"
#include "effect_push.h"
#include "effect_rainblocks.h"
#include "effect_spin.h"

#include "effect_wiper.h"

using namespace maths;
using namespace vdj;

StopWatch appClock;


// Source Samplers
std::shared_ptr<ScreenSnapshot> screenCapture = nullptr;
std::shared_ptr< SamplerWrapper> screenCap1 = nullptr;
std::shared_ptr< SamplerWrapper> screenCap2 = nullptr;


// Wrapping Samplers
//std::shared_ptr<CheckerSampler> checkSamp = nullptr;
std::shared_ptr<vdj::LumaWrapper> graySamp = nullptr;
std::shared_ptr<vdj::AnimationWindow> spinner = nullptr;

//std::shared_ptr<EffectCheckers> checkersEffect = nullptr;

// Pixel Effects
std::shared_ptr<vdj::AnimationWindow> blankEffect = nullptr;
std::shared_ptr<CrossFadeEffect> fadeFromBlack = nullptr;
std::shared_ptr<CrossFadeEffect> fadeToBlack = nullptr;
std::shared_ptr<CrossFadeEffect> fadeScreen1ToScreen2 = nullptr;
std::shared_ptr<CrossFadeEffect> fadeScreen2ToScreen1 = nullptr;

// Multi-Block effects
std::shared_ptr<vdj::AnimationWindow> horizontalFingersIn = nullptr;
std::shared_ptr<vdj::AnimationWindow> verticalFingersIn = nullptr;
std::shared_ptr<vdj::AnimationWindow> rainBlocks = nullptr;

// Push and Slide Effects
std::shared_ptr<BarnDoors> barnDoorOpen = nullptr;
std::shared_ptr<BarnDoors> barnDoorClose = nullptr;

std::shared_ptr<CornersFly> cornersFlyOut = nullptr;
std::shared_ptr<CornersFly> cornersFlyIn = nullptr;

std::shared_ptr<Push> pushLeftToRight = nullptr;
std::shared_ptr<Push> pushFromUpperLeft = nullptr;
std::shared_ptr<Push> pushFromTop = nullptr;
std::shared_ptr<vdj::AnimationWindow> wiper = nullptr;
std::shared_ptr<vdj::AnimationWindow> wiper2 = nullptr;
std::shared_ptr<vdj::AnimationWindow> wiper3 = nullptr;

std::shared_ptr<vdj::AnimationWindow> currentEffect = nullptr;

std::shared_ptr<vdj::Recorder> reco = nullptr;


constexpr int FRAMERATE = 30;
double progress = 0;

int gDirection = 1;



void keyReleased(const KeyboardEvent& e)
{
	switch (e.keyCode)
	{
	case VK_ESCAPE:
		halt();
		break;

	case VK_SPACE:
		//reco->toggleRecording();
		break;

	case VK_OEM_PLUS:
	case VK_ADD:
		gDirection = 1;
		currentEffect->setDirection(gDirection);
		break;

	case VK_OEM_MINUS:
	case VK_SUBTRACT:
		gDirection = -1;
		currentEffect->setDirection(gDirection);
		break;

		// Select from our known effects
	case VK_F1:
		currentEffect = fadeFromBlack;
		currentEffect->start();
		break;

	case VK_F2:
		// flip between screens
		currentEffect = fadeScreen1ToScreen2;
		currentEffect->start();
		break;

	case VK_F3:
		// flip between screens
		currentEffect = fadeScreen2ToScreen1;
		currentEffect->start();
		break;

	case VK_F4:
		currentEffect = barnDoorOpen;
		currentEffect->start();
		break;

	case VK_F5:
		currentEffect = barnDoorClose;
		currentEffect->start();
		break;

	case VK_F6:
		currentEffect = cornersFlyOut;
		currentEffect->start();
		break;
	
	case '6':
		currentEffect = cornersFlyIn;
		currentEffect->start();
		break;

	case VK_F7:
		currentEffect = pushLeftToRight;
		currentEffect->start();
		break;

	case VK_F8:
		currentEffect = pushFromUpperLeft;
		currentEffect->start();
		break;

	case VK_F9:
		currentEffect = pushFromTop;
		currentEffect->start();
		break;


	case VK_F10:
		currentEffect = horizontalFingersIn;
		currentEffect->start();
		break;

	case VK_F11:
		currentEffect = verticalFingersIn;
		currentEffect->start();
		break;

	case 'B':
		currentEffect = fadeToBlack;
		currentEffect->start();
		break;

	case 'Q':
		currentEffect = wiper3;
		currentEffect->start();
	break;

	case 'W':
		currentEffect = wiper;
		currentEffect->start();
		break;
	
	case 'E':
		currentEffect = wiper2;
		currentEffect->start();
		break;

	case 'R':
		currentEffect = rainBlocks;
		currentEffect->start();
		break;

	case 'S':
		currentEffect = spinner;
		currentEffect->start();
		break;

	}


}

void keyPressed(const KeyboardEvent& e)
{
	switch (e.keyCode) {
	case VK_UP:
		progress += 0.001;
		progress = maths::Clamp(progress, 0.0, 1.0);
		currentEffect->setProgress(progress);
		break;
	case VK_DOWN:
		progress -= 0.01;
		progress = maths::Clamp(progress, 0.0, 1.0);
		currentEffect->setProgress(progress);
		break;


	}
}

// Just to time frames
double thisTime;
double lastTime;


void drawShapes()
{
	//vdj::GeoTriangle<ptrdiff_t> tri(400, 10, 600, 400, 200, 400);
	GeoPolygon<ptrdiff_t> poly;
	poly.addPoint(vdj::Point<ptrdiff_t>(10, 10));
	poly.addPoint(vdj::Point<ptrdiff_t>(600, 30));
	poly.addPoint(vdj::Point<ptrdiff_t>(400, 300));
	poly.addPoint(vdj::Point<ptrdiff_t>(600, 400));
	poly.addPoint(vdj::Point<ptrdiff_t>(10, 400));
	poly.findTopmost();

	// fill and stroke a polygon
	fillPolygon(*gAppSurface, poly, vdj::PixelRGBA(0xffffff00));
	strokePolygon(*gAppSurface, poly, vdj::PixelRGBA(0xff00ff00), 3);

	// fill an ellipse
	fillEllipse(*gAppSurface, 300, 300, 200, 100, vdj::PixelRGBA(0x7fff0000));

	// draw some lines
	line(*gAppSurface, 10, 400, 600, 400, vdj::PixelRGBA(0xff0000ff), 4);
}

void onFrame()
{
	// If there's no effect, don't do anything
	// keep whatever was last on the screen;
	if (nullptr == currentEffect)
		return;
	
	//thisTime = appClock.millis();
	//auto diffTime = thisTime - lastTime;
	//printf("Interval: %3.2f  FPS: %3.2f\n", diffTime, (double)frameCount / appClock.seconds());
	//lastTime = thisTime;

	screenCapture->next();


	// start with blank slate
	background(vdj::PixelRGBA(0));

	// Either call update(), or setProgress()
	// update will be based on the clock within the effect
	//// setProgress() allows you to control the progress externally
	currentEffect->update();

	
	//sampleRectangle(*gAppSurface, PixelRect(0, 0, canvasWidth, canvasHeight), *currentEffect);
	sampleRect(*gAppSurface, vdj::PixelRect(0, 0, canvasWidth, canvasHeight), vdj::RectD(0, 0, 1, 1), *currentEffect);

	drawShapes();

	reco->saveFrame();
}

std::vector<HWND> gWinHandles;

void parseXml()
{
	Xml::Document<char> doc;

	// string must be writable
	char * aString = new char[] {"<?xml version='1.0'?><doc>content</doc>"};

	doc.Parse<0>(aString);

	auto child = doc.FirstNode();

}

void setup()
{
	//parseXml();

	setCanvasSize(displayWidth/2, displayHeight/2);
	setFrameRate(FRAMERATE);

	// Enumerate windows
	WindowCaptures::getVisibleWindowHandles(gWinHandles);

	// Setup screen captures
	screenCapture = ScreenSnapshot::createForDisplay(0, 0, displayWidth, displayHeight / 2);
	screenCap1 = std::make_shared<vdj::SamplerWrapper>(screenCapture, vdj::RectD(0, 0, 0.5, 1.0));
	screenCap2 = std::make_shared<vdj::SamplerWrapper>(screenCapture, vdj::RectD(0.50, 0, 0.5, 1.0));



	blankEffect = std::make_shared<vdj::AnimationWindow>(1);
	
	// dissolve
	fadeFromBlack = std::make_shared<CrossFadeEffect>(2, blankEffect, screenCap1);
	fadeToBlack = std::make_shared<CrossFadeEffect>(2, screenCap2, blankEffect);
	fadeScreen1ToScreen2 = std::make_shared<CrossFadeEffect>(1, screenCap1, screenCap2);
	fadeScreen2ToScreen1 = std::make_shared<CrossFadeEffect>(1, screenCap2, screenCap1);
	
	// Covering Fingers
	horizontalFingersIn = createHFingersIn(1, 32, screenCap1, screenCap2);
	verticalFingersIn = createVFingersIn(1, 64, screenCap1, screenCap2);

	// Barn Doors
	barnDoorOpen = std::make_shared<BarnDoors>(1, screenCap1, screenCap2);
	barnDoorClose = std::make_shared<BarnDoors>(1, screenCap1, screenCap2,-1);

	// Flying pieces
	cornersFlyOut = std::make_shared<CornersFly>(1, screenCap1, screenCap2);
	cornersFlyOut->setEasing(easing::circIn);
	cornersFlyIn = std::make_shared<CornersFly>(1, screenCap1, screenCap2);
	cornersFlyIn->setDirection(-1);

	// Miscellaneous
	//                        seconds, rows, columns, source 1,   source 2
	rainBlocks = createRainBlocks(1.5,     8,     16,    screenCap1, screenCap2);
	//rainBlocks->setDirection(-1);

	// Wipers
	// Left to right
	// do a gray tint while wiping
	auto lumaCap1 = vdj::SamplerWrapper::create(vdj::LumaWrapper::create(screenCap1));
	wiper = createWiper(1.5, screenCap1, lumaCap1, vdj::RectD(0, 0, 0, 1), vdj::RectD(0, 0, 1, 1));
	// wipe up from bottom left corner to top right
	wiper2 = createWiper(1, screenCap1, screenCap2, vdj::RectD(0, 1, 0, 0), vdj::RectD(0, 0, 1, 1));
	// Center outward
	wiper3 = createWiper(1, screenCap1, screenCap2, vdj::RectD(0.5, 0.5, 0, 0), vdj::RectD(0, 0, 1, 1));


	// Pushes
	pushLeftToRight = std::make_shared<Push>(1, screenCap2, screenCap1);
	pushLeftToRight->setEasing(easing::backIn);

	pushFromUpperLeft = std::make_shared<Push>(1,
		screenCap2, vdj::RectD(0,0,1,1), vdj::RectD(0,0,1,1), vdj::RectD(1,1,1,1),
		screenCap1, vdj::RectD(0,0,1,1), vdj::RectD(-1,-1,1,1), vdj::RectD(0,0,1,1));
	
	pushFromTop = std::make_shared<Push>(1.5,
		screenCap2, vdj::RectD(0, 0, 1, 1), vdj::RectD(0, 0, 1, 1), vdj::RectD(0, 1, 1,1),
		screenCap1, vdj::RectD(0, 0, 1, 1), vdj::RectD(0, -1, 1, 1), vdj::RectD(0, 0, 1, 1));
	pushFromTop->setEasing(easing::bounceOut);

	// Geometry transformations
	spinner = createSpinner(3, screenCap2, screenCap1, 0, maths::Radians(360.0));

	currentEffect = fadeFromBlack;

	// setup the recorder
	reco = std::make_shared<vdj::Recorder>(&(*gAppSurface), "frame-", 0);

}