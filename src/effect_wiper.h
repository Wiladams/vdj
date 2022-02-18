#pragma once

#include "animator.hpp"

#include <memory>



//using namespace vdj;

struct WipeAnimation : public vdj::SamplerPositionAnimation
{
	WipeAnimation(SharedSamplerWrapper sampler, const vdj::RectD &beginPos, const vdj::RectD &endPos)
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

	static std::shared_ptr< WipeAnimation> create(SharedSamplerWrapper sampler, const vdj::RectD& beginPos, const vdj::RectD& endPos)
	{
		return std::make_shared< WipeAnimation>(sampler, beginPos, endPos);
	}
};

INLINE std::shared_ptr<vdj::AnimationWindow> createWiper(double duration,
	vdj::SourceSampler s1,
	vdj::SourceSampler s2,
	const vdj::RectD &beginPos, const vdj::RectD &endPos)
{
	// Form the backing window
	auto res = std::make_shared<vdj::AnimationWindow>(duration);
	auto backing = std::make_shared<vdj::SamplerWrapper>(s1, vdj::RectD(0, 0, 1, 1));
	res->addChild(backing);

	// Create the animator
	auto wiper = std::make_shared<vdj::SamplerWrapper>(s2, vdj::RectD(0,0,1,1));
	res->addChild(wiper);

	// Create the animator
	auto animator = WipeAnimation::create(wiper, beginPos, endPos);
	res->addMotion(animator);

	return res;
}