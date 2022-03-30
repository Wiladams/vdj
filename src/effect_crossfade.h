#pragma once

#include "animator.hpp"

//#define lerp255(bg, fg, a) ((uint32_t)div255((fg*a+bg*(255-a))))

//
// Cross fade, or dissolve, is a per pixel operation
// essentially go from the source1 image to the source2
// image with a mix of the two.
class CrossFadeEffect : public vdj::AnimationWindow
{
	alib::SourceSampler fSource1;
	alib::SourceSampler fSource2;

	INLINE static alib::PixelRGBA lerp_pixel(double u, const alib::PixelRGBA& bg, const alib::PixelRGBA& fg)
	{
		uint32_t a = (uint32_t)(u * 255.0);
		return alib::PixelRGBA(
			alib::lerp255(bg.r(), fg.r(), a),
			alib::lerp255(bg.g(), fg.g(), a),
			alib::lerp255(bg.b(), fg.b(), a), 255);
	}

public:
	CrossFadeEffect(
		double duration,
		alib::SourceSampler s1,
		alib::SourceSampler s2)
		:AnimationWindow(duration)
		,fSource1(s1)
		,fSource2(s2)
	{
	}

	INLINE alib::SourceSampler  source1() { return fSource1; }
	INLINE alib::SourceSampler  source2() { return fSource2; }

	alib::PixelRGBA getValue(double u, double v) override
	{
		auto c1 = source1()->getValue(u, v);
		auto c2 = source2()->getValue(u, v);

		return lerp_pixel(progress(), c1, c2);
	}
};