#pragma once

#include "animator.h"
#include <memory>





struct WipeAnimation : public SamplerPositionAnimation
{
	WipeAnimation(SharedSamplerWrapper sampler, const RectD &beginPos, const RectD &endPos)
		:SamplerPositionAnimation(sampler, beginPos, endPos)
	{}

	void onUpdate(double u) override
	{
		// calculate frame and bounds
		double u1 = fEasing(u);
		auto newValue = maths::Lerp(u1, fBeginPos, fEndPos);

		// To do a wipe, change the frame and bounds at the same time
		// to be the same shape
		fSampler->setFrame(newValue);
		fSampler->setBounds(newValue);
	}

	static std::shared_ptr< WipeAnimation> create(SharedSamplerWrapper sampler, const RectD& beginPos, const RectD& endPos)
	{
		return std::make_shared< WipeAnimation>(sampler, beginPos, endPos);
	}
};

INLINE std::shared_ptr<AnimationWindow> createWiper(double duration,
	SourceSampler s1,
	SourceSampler s2,
	const RectD &beginPos, const RectD &endPos)
{
	// Form the backing window
	auto res = std::make_shared<AnimationWindow>(duration);
	auto backing = std::make_shared<SamplerWrapper>(s1, RectD(0, 0, 1, 1));
	res->addChild(backing);

	// Create the animator
	auto wiper = std::make_shared<SamplerWrapper>(s2, RectD(0,0,1,1));
	res->addChild(wiper);

	// Create the animator
	auto animator = WipeAnimation::create(wiper, beginPos, endPos);
	res->addMotion(animator);

	return res;
}