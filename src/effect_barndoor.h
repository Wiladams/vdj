#pragma once

#include "animator.h"
using namespace std::placeholders;

// Barn doors do an open reveal
class BarnDoors : public AnimationWindow
{
public:

	std::shared_ptr<SamplerWrapper> fDoor1;
	std::shared_ptr<SamplerWrapper> fDoor2;
	std::shared_ptr<SamplerWrapper> fDestination;
	
	BarnDoors(double duration, SourceSampler s1, SourceSampler s2)
		:BarnDoors(duration, s1, s2, 1)
	{}

	BarnDoors(double duration, SourceSampler s1, SourceSampler s2, int direction)
		: AnimationWindow(duration)
	{
		setDirection(direction);

		RectD door1Bounds(0, 0, 0.5, 1);
		RectD door2Bounds(0.5, 0, 0.5, 1);

		auto destinationBounds = RectD(0, 0, 1, 1);

		fDoor1 = std::make_shared<SamplerWrapper>(s1, door1Bounds);
		fDoor2 = std::make_shared<SamplerWrapper>(s1, door2Bounds);
		fDestination = std::make_shared<SamplerWrapper>(s2, destinationBounds);

		addChild(fDestination);
		addChild(fDoor1);
		addChild(fDoor2);

		addMotion(SamplerPositionAnimation::create(fDoor1, door1Bounds, RectD(-0.5, 0, 0.5, 1)));
		addMotion(SamplerPositionAnimation::create(fDoor2, door2Bounds, RectD(1.0, 0, 0.5, 1)));
	}
};


