#pragma once

//
// A push is typically a displacement, the incoming image
// displaces the outgoing one.  There's nothing really special
// about this class other than it pre-calculates one particular
// motion, that of the push from the left side.
// The second constructor takes all the parameters necessary to 
// achieve a push from anywhere, and since the source and
// destinations can be anything, it doesn't even have to be a push

#include "animator.hpp"

struct Push : public vdj::AnimationWindow
{
	// Quick and dirty left to right horizontal push
	//
	Push(double duration, 
		alib::SourceSampler s1,
		alib::SourceSampler s2)
		:AnimationWindow(duration)
	{
		auto pusher = std::make_shared<SamplerWrapper>(s2, alib::RectD(0, 0, 1, 1));
		auto pushee = std::make_shared<SamplerWrapper>(s1, alib::RectD(0, 0, 1, 1));


		addMotion(vdj::SamplerPositionAnimation::create(pusher, alib::RectD(-1, 0, 1, 1), alib::RectD(0, 0, 1, 1)));
		addMotion(vdj::SamplerPositionAnimation::create(pushee, alib::RectD(0, 0, 1, 1), alib::RectD(1, 0, 1, 1)));
		
		addChild(pusher);
		addChild(pushee);
	}

	//
	// It's a lot of parameters
	// but, you can implement any kind of push by specifying
	// all these parameters
	Push(double duration,
		alib::SourceSampler s1, const alib::RectD&s1bounds, const alib::RectD&s1starting, const alib::RectD&s1ending,
		alib::SourceSampler s2, const alib::RectD&s2bounds, const alib::RectD&s2starting, const alib::RectD&s2ending)
		:AnimationWindow(duration)
	{
		auto pusher = std::make_shared<SamplerWrapperWindow>(s2, s2bounds);
		auto pushee = std::make_shared<SamplerWrapperWindow>(s1, s1bounds);

		addMotion(vdj::SamplerPositionAnimation::create(pusher, s1starting, s1ending));
		addMotion(vdj::SamplerPositionAnimation::create(pushee, s2starting, s2ending));

		addChild(pusher);
		addChild(pushee);
	}

};