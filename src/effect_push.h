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

struct Push : public vdj::AnimationWindow
{
	// Quick and dirty left to right horizontal push
	//
	Push(double duration, 
		vdj::SourceSampler s1,
		vdj::SourceSampler s2)
		:AnimationWindow(duration)
	{
		auto pusher = std::make_shared<SamplerWrapper>(s2, vdj::RectD(0, 0, 1, 1));
		auto pushee = std::make_shared<SamplerWrapper>(s1, vdj::RectD(0, 0, 1, 1));


		addMotion(vdj::SamplerPositionAnimation::create(pusher, vdj::RectD(-1, 0, 1, 1), vdj::RectD(0, 0, 1, 1)));
		addMotion(vdj::SamplerPositionAnimation::create(pushee, vdj::RectD(0, 0, 1, 1), vdj::RectD(1, 0, 1, 1)));
		
		addChild(pusher);
		addChild(pushee);
	}

	//
	// It's a lot of parameters
	// but, you can implement any kind of push by specifying
	// all these parameters
	Push(double duration,
		vdj::SourceSampler s1, const vdj::RectD&s1bounds, const vdj::RectD&s1starting, const vdj::RectD&s1ending,
		vdj::SourceSampler s2, const vdj::RectD&s2bounds, const vdj::RectD&s2starting, const vdj::RectD&s2ending)
		:AnimationWindow(duration)
	{
		auto pusher = std::make_shared<vdj::SampledWindow>(s2, s2bounds);
		auto pushee = std::make_shared<vdj::SampledWindow>(s1, s1bounds);

		addMotion(vdj::SamplerPositionAnimation::create(pusher, s1starting, s1ending));
		addMotion(vdj::SamplerPositionAnimation::create(pushee, s2starting, s2ending));

		addChild(pusher);
		addChild(pushee);
	}

};