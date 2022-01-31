#pragma once

#include "animator.h"
#include <memory>

INLINE std::shared_ptr<WindowAnimation> createRainBlocks(double duration,
	int maxRows, int maxColumns,
	std::shared_ptr<ISample2D<PixelRGBA, PixelCoord> > s1,
	std::shared_ptr<ISample2D<PixelRGBA, PixelCoord> > s2)
{
	auto res = std::make_shared<WindowAnimation>(duration);

	auto backing = std::make_shared<SampledWindow>(s1, TexelRect(0, 0, 1, 1));


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
			TexelRect endPos(uOffset, vOffset, uOffset + uSize, vOffset + vSize);
			TexelRect beginPos(uOffset, -vSize, uOffset + uSize, -0.01);

			auto block = std::make_shared<SampledWindow>(s2, endPos);
			block->setFrame(beginPos);

			auto motion = std::make_shared<TexelRectMotion>(block->fMovingFrame, beginPos, endPos);
			//double endTime = ((double)(maxRows - row)) * dropOffset;
			double endTime = maths::random_double(0.25, 1.0);
			double duration = 0.25;
			double startTime = endTime - duration;

			motion->setTiming(startTime, endTime);
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
