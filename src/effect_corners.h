#pragma once

#include "animator.h"

// Note:  To generalize flyouts, need to calculate vectors to the 
// circumference of a circle
// Reference:
// https://www.varsitytutors.com/hotmath/hotmath_help/topics/shortest-distance-between-a-point-and-a-circle
//

class CornersFlyOut : public AnimationWindow
{
protected:

public:
	CornersFlyOut(double duration,
		SourceSampler s1,
		SourceSampler s2)
		: AnimationWindow(duration)
	{
		auto destinationBounds = RectD(0, 0, 1, 1);
		RectD door1Bounds(0.0, 0.0, 0.5, 0.5);
		RectD door2Bounds(0.5, 0.0, 0.5, 0.5);
		RectD door3Bounds(0.0, 0.5, 0.5, 0.5);
		RectD door4Bounds(0.5, 0.5, 0.5, 0.5);

		auto fDoor1 = std::make_shared<SamplerWrapper>(s1, door1Bounds);
		auto fDoor2 = std::make_shared<SamplerWrapper>(s1, door2Bounds);
		auto fDoor3 = std::make_shared<SamplerWrapper>(s1, door3Bounds);
		auto fDoor4 = std::make_shared<SamplerWrapper>(s1, door4Bounds);

		addMotion(std::make_shared<TexelRectMotion>(fDoor1->fMovingFrame, RectD(0.0, 0.0, 0.5, 0.5), RectD(-0.5, -0.5, 0.5, 0.5)));
		addMotion(std::make_shared<TexelRectMotion>(fDoor2->fMovingFrame, RectD(0.5, 0.0, 0.5, 0.5), RectD(1.0, -0.5, 0.5, 0.5)));
		addMotion(std::make_shared<TexelRectMotion>(fDoor3->fMovingFrame, RectD(0.0, 0.5, 0.5, 0.5), RectD(-0.5, 1.0, 0.5, 0.5)));
		addMotion(std::make_shared<TexelRectMotion>(fDoor4->fMovingFrame, RectD(0.5, 0.5, 0.5, 0.5), RectD(1.0, 1.0, 0.5, 0.5)));


		auto fDestination = std::make_shared<SamplerWrapper>(s2, destinationBounds);

		addChild(fDestination);
		addChild(fDoor1);
		addChild(fDoor2);
		addChild(fDoor3);
		addChild(fDoor4);
	}


};


