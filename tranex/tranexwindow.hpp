#pragma once

#include "gui.h"
#include "normalizedwindow.hpp"
#include "checkerboard.hpp"

#include "animator.hpp"

using namespace vdj;

CheckerSampler checkers(64, PixelRGBA(0xff1f1f1f), PixelRGBA(0xffffffff));

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

void TranexWindow::draw(PixelView &ctx)
{
    // draw background
	sampleRect(ctx, PixelRect(0,0,canvasWidth, canvasHeight), RectD(0,0,1,1), checkers);

    // draw children
	sampleRect(ctx, PixelRect(0,0,canvasWidth, canvasHeight), RectD(0,0,1,1), *this);

}