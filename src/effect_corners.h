#pragma once

#include "animator.hpp"

// Note:  To generalize flyouts, need to calculate vectors to the 
// circumference of a circle
// Reference:
// https://www.varsitytutors.com/hotmath/hotmath_help/topics/shortest-distance-between-a-point-and-a-circle
//

class CornersFly : public vdj::AnimationWindow
{
protected:

public:
	CornersFly(double duration,
		alib::SourceSampler s1,
		alib::SourceSampler s2)
		: AnimationWindow(duration)
	{
		auto destinationBounds = alib::RectD(0, 0, 1, 1);
		auto destination = SamplerWrapper::create(s2, destinationBounds);
		addChild(destination);

		// Create the 4 corner panels
		alib::RectD door1Bounds(0.0, 0.0, 0.5, 0.5);
		alib::RectD door2Bounds(0.5, 0.0, 0.5, 0.5);
		alib::RectD door3Bounds(0.0, 0.5, 0.5, 0.5);
		alib::RectD door4Bounds(0.5, 0.5, 0.5, 0.5);

		auto door1 = vdj::SamplerWrapper::create(s1, door1Bounds);
		auto door2 = vdj::SamplerWrapper::create(s1, door2Bounds);
		auto door3 = vdj::SamplerWrapper::create(s1, door3Bounds);
		auto door4 = vdj::SamplerWrapper::create(s1, door4Bounds);

		addMotion(vdj::SamplerPositionAnimation::create(door1, door1Bounds, alib::RectD(-0.5, -0.5, 0.5, 0.5)));
		addMotion(vdj::SamplerPositionAnimation::create(door2, door2Bounds, alib::RectD(1.0, -0.5, 0.5, 0.5)));
		addMotion(vdj::SamplerPositionAnimation::create(door3, door3Bounds, alib::RectD(-0.5, 1.0, 0.5, 0.5)));
		addMotion(vdj::SamplerPositionAnimation::create(door4, door4Bounds, alib::RectD(1.0, 1.0, 0.5, 0.5)));

		addChild(door1);
		addChild(door2);
		addChild(door3);
		addChild(door4);
	}
};


