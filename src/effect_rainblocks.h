#pragma once

#include "animator.h"
#include <memory>

INLINE std::shared_ptr<AnimationWindow> createRainBlocks(double duration,
	int maxRows, int maxColumns,
	SourceSampler s1,
	SourceSampler s2)
{
	auto res = std::make_shared<AnimationWindow>(duration);

	auto backing = std::make_shared<SamplerWrapper>(s1, RectD(0, 0, 1, 1));


	// Add the background that is to be covered
	res->addChild(backing);

	// create individual blocks
	double uSize = 1.0/maxColumns;
	double vSize = 1.0 / maxRows;
	double dropDuration = 1.0/(maxRows*4);
	double dropOffset = 1.0 / (maxRows);

	double uOffset = 0.0;

	for (int col = 0; col<maxColumns; col++) 
	{

		int row = maxRows;
		while  (row-->0) 
		{
			double vOffset = row * vSize;
			RectD endPos(uOffset, vOffset, uSize, vSize);
			RectD beginPos(uOffset, -vSize, uSize, vSize-0.01);

			auto block = std::make_shared<SamplerWrapper>(s2, endPos);
			block->setFrame(beginPos);

			auto motion = SamplerPositionAnimation::create(block, beginPos, endPos);

			double endTime = maths::random_double(0.25, 1.0);
			double duration = 0.25;
			double startTime = endTime - duration;

			motion->setProgressEnvelope(startTime, endTime);

			// we want blocks hitting the last row to bounce
			// off the bottom
			if (row == maxRows - 1)
			{
				motion->setEasing(easing::bounceOut);
			}

			res->addChild(block);
			res->addMotion(motion);

			vOffset += vSize;
		}
		uOffset += uSize;
	}
	return res;
}
