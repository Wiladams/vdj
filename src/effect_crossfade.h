#pragma once

#include "animator.h"

#define lerp255(bg, fg, a) ((uint32_t)div255((fg*a+bg*(255-a))))

class CrossFadeEffect : public AnimationWindow
{
	SourceSampler fSource1;
	SourceSampler fSource2;

	INLINE static PixelRGBA lerp_pixel(double u, const PixelRGBA& bg, const PixelRGBA& fg)
	{
		uint32_t a = u * 255;
		return PixelRGBA(
			lerp255(bg.r(), fg.r(), a),
			lerp255(bg.g(), fg.g(), a),
			lerp255(bg.b(), fg.b(), a), 255);
	}

public:
	CrossFadeEffect(
		double duration,
		SourceSampler s1,
		SourceSampler s2)
		:AnimationWindow(duration)
		,fSource1(s1)
		,fSource2(s2)
	{
	}

	INLINE SourceSampler  source1() { return fSource1; }
	INLINE SourceSampler  source2() { return fSource2; }

	PixelRGBA getValue(double u, double v) override
	{
		auto c1 = source1()->getValue(u, v);
		auto c2 = source2()->getValue(u, v);

		return lerp_pixel(progress(), c1, c2);
	}
};