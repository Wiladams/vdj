#pragma once


#include "animator.h"

#include <memory>

//
// Horizontal fingers
// Do a cover zipper coming from left and right
// Specify how many fingers
//
INLINE std::shared_ptr<vdj::AnimationWindow> createHFingersIn(double duration, int howMany,
	vdj::SourceSampler s1,
	vdj::SourceSampler s2)
{
	auto res = std::make_shared<vdj::AnimationWindow>(duration);

	auto backing = vdj::SamplerWrapper::create(s1, vdj::RectD(0, 0, 1, 1));

	// Add the background that is to be covered
	res->addChild(backing);

	// Create the set of fingers which will do the covering
	bool fromTheLeft = true;
	double uSize = 1.0;
	double vSize = 1.0 / howMany;

	for (int i = 0; i < howMany; i++)
	{
		vdj::RectD beginPos;
		vdj::RectD endPos(0, i * vSize, 1, vSize);

		if (fromTheLeft) {
			beginPos = vdj::RectD(-1.0, i * vSize, 1.0, vSize);
		}
		else {
			beginPos = vdj::RectD(1.0, i * vSize, 1.0, vSize);
		}
		auto finger = vdj::SamplerWrapper::create(s2, endPos);
		auto motion = vdj::SamplerPositionAnimation::create(finger, beginPos, endPos);

		res->addChild(finger);
		res->addMotion(motion);

		fromTheLeft = !fromTheLeft;
	}

	return res;
}

//
// Vertical Fingers
// do a cover zipper from top and bottom
//
INLINE std::shared_ptr<vdj::AnimationWindow> createVFingersIn(double duration, int howMany,
	vdj::SourceSampler s1,
	vdj::SourceSampler s2)
{
	auto res = std::make_shared<vdj::AnimationWindow>(duration);

	auto backing = vdj::SamplerWrapper::create(s1, vdj::RectD(0, 0, 1, 1));

	// Add the background that is to be covered
	res->addChild(backing);

	// Create the set of fingers which will do the covering
	bool fromThisSide = true;
	double uSize = 1.0 / howMany;
	double vSize = 1.0;

	for (int i = 0; i < howMany; i++)
	{
		vdj::RectD beginPos;
		vdj::RectD endPos(i * uSize, 0, uSize, 1);

		if (fromThisSide) {
			beginPos = vdj::RectD(i * uSize, -1.0, uSize, 1.0);
		}
		else {
			beginPos = vdj::RectD(i * uSize, 1.0, uSize, 1.0);
		}
		auto finger = vdj::SamplerWrapper::create(s2, endPos);
		auto motion = vdj::SamplerPositionAnimation::create(finger, beginPos, endPos);

		res->addChild(finger);
		res->addMotion(motion);

		fromThisSide = !fromThisSide;
	}

	return res;
}
