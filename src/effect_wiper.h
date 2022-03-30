#pragma once

#include "animator.hpp"

#include <memory>


struct WipeAnimation : public vdj::SamplerPositionAnimation
{
	WipeAnimation(SharedSamplerWrapper sampler, const alib::RectD &beginPos, const alib::RectD &endPos)
		:SamplerPositionAnimation(sampler, beginPos, endPos)
	{}

	void onUpdate(double u) override
	{
		// calculate frame and bounds
		double u1 = fEasing(u);
		auto newValue = alib::Lerp(u1, fBeginPos, fEndPos);

		// To do a wipe, change the frame and bounds at the same time
		// to be the same shape
		fSampler->setFrame(newValue);
		fSampler->setBounds(newValue);
	}

	static std::shared_ptr< WipeAnimation> create(SharedSamplerWrapper sampler, const alib::RectD& beginPos, const alib::RectD& endPos)
	{
		return std::make_shared< WipeAnimation>(sampler, beginPos, endPos);
	}
};

INLINE std::shared_ptr<vdj::AnimationWindow> createWiper(double duration,
	alib::SourceSampler s1,
	alib::SourceSampler s2,
	const alib::RectD &beginPos, const alib::RectD &endPos)
{
	// Form the backing window
	auto res = std::make_shared<vdj::AnimationWindow>(duration);
	auto backing = std::make_shared<vdj::SamplerWrapper>(s1, alib::RectD(0, 0, 1, 1));
	res->addChild(backing);

	// Create the animator
	auto wiper = std::make_shared<vdj::SamplerWrapper>(s2, alib::RectD(0,0,1,1));
	res->addChild(wiper);

	// Create the animator
	auto animator = WipeAnimation::create(wiper, beginPos, endPos);
	res->addMotion(animator);

	return res;
}