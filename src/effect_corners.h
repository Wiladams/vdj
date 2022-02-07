#pragma once

#include "animator.h"

// Note:  To generalize flyouts, need to calculate vectors to the 
// circumference of a circle
// Reference:
// https://www.varsitytutors.com/hotmath/hotmath_help/topics/shortest-distance-between-a-point-and-a-circle
//

class CornersFly : public AnimationWindow
{
protected:

public:
	CornersFly(double duration,
		SourceSampler s1,
		SourceSampler s2)
		: AnimationWindow(duration)
	{
		auto destinationBounds = RectD(0, 0, 1, 1);
		auto destination = SamplerWrapper::create(s2, destinationBounds);
		addChild(destination);

		// Create the 4 corner panels
		RectD door1Bounds(0.0, 0.0, 0.5, 0.5);
		RectD door2Bounds(0.5, 0.0, 0.5, 0.5);
		RectD door3Bounds(0.0, 0.5, 0.5, 0.5);
		RectD door4Bounds(0.5, 0.5, 0.5, 0.5);

		auto door1 = SamplerWrapper::create(s1, door1Bounds);
		auto door2 = SamplerWrapper::create(s1, door2Bounds);
		auto door3 = SamplerWrapper::create(s1, door3Bounds);
		auto door4 = SamplerWrapper::create(s1, door4Bounds);

		addMotion(SamplerPositionAnimation::create(door1, door1Bounds, RectD(-0.5, -0.5, 0.5, 0.5)));
		addMotion(SamplerPositionAnimation::create(door2, door2Bounds, RectD(1.0, -0.5, 0.5, 0.5)));
		addMotion(SamplerPositionAnimation::create(door3, door3Bounds, RectD(-0.5, 1.0, 0.5, 0.5)));
		addMotion(SamplerPositionAnimation::create(door4, door4Bounds, RectD(1.0, 1.0, 0.5, 0.5)));

		addChild(door1);
		addChild(door2);
		addChild(door3);
		addChild(door4);
	}
};


