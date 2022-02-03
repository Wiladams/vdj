#pragma once

#include "animator.h"
#include <memory>

INLINE std::shared_ptr<AnimationWindow> createWiper(double duration,
	SourceSampler s1,
	SourceSampler s2,
	const vec2f &dir)
{
	const int maxRows = 4;
	const int maxColumns = 4;

	auto res = std::make_shared<AnimationWindow>(duration);

	auto backing = std::make_shared<SamplerWrapper>(s1, RectD(0, 0, 1, 1));
	
	// Add the background that is to be covered
	res->addChild(backing);

	RectD beginPos(0, 0, 0, 1);
	RectD endPos(0, 0, 1, 1);

	auto wiper = std::make_shared<SamplerWrapper>(s2, RectD(0,0,1,1));
	auto frameMotion = std::make_shared<TexelRectMotion>(wiper->fMovingFrame, beginPos, endPos);
	auto boundsMotion = std::make_shared<TexelRectMotion>(wiper->fStickyBounds, beginPos, endPos);

	res->addChild(wiper);
	res->addMotion(frameMotion);
	res->addMotion(boundsMotion);

	return res;
}