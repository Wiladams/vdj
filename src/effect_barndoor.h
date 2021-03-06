#pragma once

#include "animator.hpp"

using namespace std::placeholders;

// Barn doors do an open reveal
class BarnDoors : public vdj::AnimationWindow
{
public:

	std::shared_ptr<SamplerWrapper> fDoor1;
	std::shared_ptr<SamplerWrapper> fDoor2;
	std::shared_ptr<SamplerWrapper> fDestination;
	
	BarnDoors(double duration, alib::SourceSampler s1, alib::SourceSampler s2)
		:BarnDoors(duration, s1, s2, 1)
	{}

	BarnDoors(double duration, alib::SourceSampler s1, alib::SourceSampler s2, int direction)
		: AnimationWindow(duration)
	{
		setDirection(direction);

		alib::RectD door1Bounds(0, 0, 0.5, 1);
		alib::RectD door2Bounds(0.5, 0, 0.5, 1);

		auto destinationBounds = alib::RectD(0, 0, 1, 1);

		fDoor1 = std::make_shared<SamplerWrapper>(s1, door1Bounds);
		fDoor2 = std::make_shared<SamplerWrapper>(s1, door2Bounds);
		fDestination = std::make_shared<SamplerWrapper>(s2, destinationBounds);

		addChild(fDestination);
		addChild(fDoor1);
		addChild(fDoor2);

		addMotion(vdj::SamplerPositionAnimation::create(fDoor1, door1Bounds, alib::RectD(-0.5, 0, 0.5, 1)));
		addMotion(vdj::SamplerPositionAnimation::create(fDoor2, door2Bounds, alib::RectD(1.0, 0, 0.5, 1)));
	}
};


