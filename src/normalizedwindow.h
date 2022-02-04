#pragma once

#include "pixeltypes.h"


#include <vector>
#include <memory>

//
// SamplerFrame
// Implement a sampler base class that has a frame
// The coordinates are meant to be expressed in normalized
// coordinate space, so they should range [0..1], but they 
// don't have to be in that range at all.  The only
// criteria is doubles are used, and getValue() typically takes
// parameters in the [0..1] range
//
struct SamplerFrame : public ISample2D<PixelRGBA>
{
protected:

public:
	RectD fMovingFrame;

	SamplerFrame()
		:fMovingFrame(0, 0, 1, 1) {}

	SamplerFrame(const RectD& frame)
		: fMovingFrame(frame)
	{
		onFrameChanged();
	}

	virtual ~SamplerFrame() {}

	RectD frame() const { return fMovingFrame; }
	void setFrame(const RectD& frame)
	{
		fMovingFrame = frame;
		onFrameChanged();
	}

	virtual void onFrameChanged() {}

	virtual bool contains(double u, double v)
	{
		return fMovingFrame.contains(u, v);
	}

	virtual PixelRGBA getValue(double u, double v) { return PixelRGBA(0x0); }
};


// 
// Trying to get the math right on Sub-samples
// We want a SubSample to be able to grab a portion of
// a wrapped sampler, and display it using the range
// [0..1]
//
// A SamplerWrapper is just a sampler that wraps
// another sampler.  Since it is a sampler itself
// it maps the range of [0..1] to span the wrapped
// range.  This is good for sprite sheets for example.
//
struct SamplerWrapper : public SamplerFrame
{
protected:
	double uFactor = 1;
	double vFactor = 1;

public:
	std::shared_ptr< ISample2D<PixelRGBA> > fBackground;	// The thing we're sub-sampling from
	RectD fStickyBounds;

	SamplerWrapper()
		: SamplerFrame()
		, fBackground(nullptr)
		, fStickyBounds(RectD(0,0,1,1))
	{
	}

	SamplerWrapper(std::shared_ptr< ISample2D<PixelRGBA> > wrapped, const RectD &bounds)
		:SamplerFrame(RectD(0,0,1,1))
		,fBackground(wrapped)
		, fStickyBounds(bounds)
	{
		onFrameChanged();
	}

	SamplerWrapper(std::shared_ptr< ISample2D<PixelRGBA> > wrapped,
		const RectD&bounds, const RectD&frame)
		:SamplerFrame(frame)
		,fBackground(wrapped)
		,fStickyBounds(bounds)
	{
		onFrameChanged();
	}

	virtual ~SamplerWrapper() = default;

	RectD bounds() const { return fStickyBounds; }
	void setBounds(const RectD& bounds)
	{
		fStickyBounds = bounds;
		onFrameChanged();
	}

	void onFrameChanged() override
	{
		uFactor = ((fStickyBounds.w()) / (fMovingFrame.w()));
		vFactor = ((fStickyBounds.h()) / (fMovingFrame.h()));
	}

	PixelRGBA getValue(double u, double v) override
	{
		if (nullptr != fBackground)
		{
			double u1 = fStickyBounds.left() + (u - fMovingFrame.left()) * uFactor;
			double v1 = fStickyBounds.top() + (v - fMovingFrame.top()) * vFactor;

			return fBackground->getValue(u1, v1);
		}

		// return transparent if we're not wrapping anything
		return PixelRGBA(0x0);
	}

	static std::shared_ptr< SamplerWrapper> create(std::shared_ptr< ISample2D<PixelRGBA> > wrapped, const RectD& bounds)
	{
		return std::make_shared<SamplerWrapper>(wrapped, bounds, RectD(0, 0, 1, 1));
	}
};


// SampledWindow
//
// A window is already a SamplerWrapper, so it has a background
// and it can act as a sample source.
// What a window brings to the party is the ability to have child
// regions.  The child regions themselves are at least SamplerWrapper
// objects, so they too can return a pixel value at a given location.
// Having the sub-areas makes it relatively easy to compose scenes
// by just adding children and moving them around.
//
struct SampledWindow : public SamplerWrapper
{
	std::vector<std::shared_ptr<SamplerWrapper > > fChildren;

	
	SampledWindow()
	{ 
		onFrameChanged();
	}
	
	SampledWindow(std::shared_ptr<ISample2D<PixelRGBA> > background)
		:SamplerWrapper(background, RectD(0,0,1,1))
	{
		onFrameChanged();
	}

	SampledWindow(std::shared_ptr<ISample2D<PixelRGBA> > background, const RectD &bounds)
		:SamplerWrapper(background, bounds)
	{
		onFrameChanged();
	}

	virtual ~SampledWindow() {}

	void onFrameChanged() override
	{ 
		uFactor = ((fStickyBounds.w()) / (fMovingFrame.w()));
		vFactor = ((fStickyBounds.h()) / (fMovingFrame.h()));

		// Tell all the children the frame changed
		for (auto& child : fChildren)
		{
			child->onFrameChanged();
		}
	}

	// Dealing with Window grouping stuff
	void addChild(std::shared_ptr<SamplerWrapper > aChild)
	{
		fChildren.push_back(aChild);
	}

	void clearChildren()
	{
		fChildren.clear();
	}



	// Reference: http://www.guyrutenberg.com/2007/11/19/c-goes-to-operator/
	// useful for while loops
	PixelRGBA getValue(double parentu, double parentv) override
	{
		// find which most visible child the uv coordinates hit
		// ask that child for a value.
		int count = fChildren.size();
		double myu = (parentu - fMovingFrame.fX)/fMovingFrame.fWidth;
		double myv = (parentv - fMovingFrame.fY)/fMovingFrame.fHeight;

		while (count-- > 0)
		{
			if (fChildren[count]->contains(myu, myv))
				return fChildren[count]->getValue(myu, myv);
		}

		if (nullptr != fBackground)
			return fBackground->getValue(myu, myv);

		return PixelRGBA(0x0);
	}

};


using SharedSamplerWrapper = std::shared_ptr<SamplerWrapper>;
