#pragma once

//
// A push is typically a displacement, the incoming image
// displaces the outgoing one.  There's nothing really special
// about this class other than it pre-calculates one particular
// motion, that of the push from the left side.
// The second constructor takes all the parameters necessary to 
// achieve a push from anywhere, and since the source and
// destinations can be anything, it doesn't even have to be a push

#include "animator.h"

struct Push : public AnimationWindow
{
	// Quick and dirty left to right horizontal push
	//
	Push(double duration, 
		SourceSampler s1,
		SourceSampler s2)
		:AnimationWindow(duration)
	{
		auto pusher = std::make_shared<SamplerWrapper>(s2, RectD(0, 0, 1, 1));
		auto pushee = std::make_shared<SamplerWrapper>(s1, RectD(0, 0, 1, 1));


		addMotion(SamplerPositionAnimation::create(pusher, RectD(-1, 0, 1, 1), RectD(0, 0, 1, 1)));
		addMotion(SamplerPositionAnimation::create(pushee, RectD(0, 0, 1, 1), RectD(1, 0, 1, 1)));
		
		addChild(pusher);
		addChild(pushee);
	}

	//
	// It's a lot of parameters
	// but, you can implement any kind of push by specifying
	// all these parameters
	Push(double duration,
		SourceSampler s1, const RectD&s1bounds, const RectD&s1starting, const RectD&s1ending,
		SourceSampler s2, const RectD&s2bounds, const RectD&s2starting, const RectD&s2ending)
		:AnimationWindow(duration)
	{
		auto pusher = std::make_shared<SampledWindow>(s2, s2bounds);
		auto pushee = std::make_shared<SampledWindow>(s1, s1bounds);

		addMotion(SamplerPositionAnimation::create(pusher, s1starting, s1ending));
		addMotion(SamplerPositionAnimation::create(pushee, s2starting, s2ending));

		addChild(pusher);
		addChild(pushee);
	}

};