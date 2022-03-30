#pragma once

#include "pixeltypes.hpp"
#include "sampler.hpp"

#include <vector>
#include <memory>

namespace vdj
{
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
	struct SamplerWrapper : public alib::ISample2D<alib::PixelRGBA>
	{
	protected:
		double uFactor = 1;
		double vFactor = 1;

	public:
		std::shared_ptr< alib::ISample2D<alib::PixelRGBA> > fBackground;	// The thing we're sub-sampling from
		alib::RectD fMovingFrame;
		alib::RectD fStickyBounds;

	public:
		SamplerWrapper()
			: fBackground(nullptr)
			, fMovingFrame(alib::RectD(0, 0, 1, 1))
			, fStickyBounds(alib::RectD(0, 0, 1, 1))
		{
			onFrameChanged();
		}

		SamplerWrapper(std::shared_ptr< alib::ISample2D<alib::PixelRGBA> > wrapped, const alib::RectD& bounds)
			:fBackground(wrapped)
			, fMovingFrame(alib::RectD(0, 0, 1, 1))
			, fStickyBounds(bounds)
		{
			onFrameChanged();
		}

		SamplerWrapper(std::shared_ptr< alib::ISample2D<alib::PixelRGBA> > wrapped,
			const alib::RectD& bounds, const alib::RectD& frame)
			:fBackground(wrapped)
			, fMovingFrame(frame)
			, fStickyBounds(bounds)
		{
			onFrameChanged();
		}

		virtual ~SamplerWrapper() = default;

		alib::RectD bounds() const { return fStickyBounds; }
		void setBounds(const alib::RectD& bounds)
		{
			fStickyBounds = bounds;
			onFrameChanged();
		}

		alib::RectD frame() const { return fMovingFrame; }
		void setFrame(const alib::RectD& frame)
		{
			fMovingFrame = frame;
			onFrameChanged();
		}

		virtual void onFrameChanged()
		{
			uFactor = ((fStickyBounds.w()) / (fMovingFrame.w()));
			vFactor = ((fStickyBounds.h()) / (fMovingFrame.h()));
		}

		virtual bool contains(double u, double v)
		{
			return fMovingFrame.contains(u, v);
		}

		alib::PixelRGBA getValue(double u, double v) override
		{
			if (nullptr != fBackground)
			{
				double u1 = fStickyBounds.left() + (u - fMovingFrame.left()) * uFactor;
				double v1 = fStickyBounds.top() + (v - fMovingFrame.top()) * vFactor;

				return fBackground->getValue(u1, v1);
			}

			// return transparent if we're not wrapping anything
			return alib::PixelRGBA(0x0);
		}

		static std::shared_ptr< SamplerWrapper> create(std::shared_ptr< ISample2D<alib::PixelRGBA> > wrapped)
		{
			return std::make_shared<SamplerWrapper>(wrapped, alib::RectD(0, 0, 1, 1), alib::RectD(0, 0, 1, 1));
		}

		static std::shared_ptr< SamplerWrapper> create(std::shared_ptr< ISample2D<alib::PixelRGBA> > wrapped, const alib::RectD& bounds)
		{
			return std::make_shared<SamplerWrapper>(wrapped, bounds, alib::RectD(0, 0, 1, 1));
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
	
	template <typename CT=SamplerWrapper>
	struct SampledWindow : public SamplerWrapper
	{
		std::vector<std::shared_ptr<CT > > fChildren;	// Typically SamplerWrapper


		SampledWindow()
		{
			onFrameChanged();
		}

		SampledWindow(std::shared_ptr<ISample2D<alib::PixelRGBA> > background)
			:SamplerWrapper(background, alib::RectD(0, 0, 1, 1))
		{
			onFrameChanged();
		}

		SampledWindow(std::shared_ptr<ISample2D<alib::PixelRGBA> > background, const alib::RectD& bounds)
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
		void addChild(std::shared_ptr<CT> aChild)
		{
			fChildren.push_back(aChild);
		}

		void clearChildren()
		{
			fChildren.clear();
		}



		// Reference: http://www.guyrutenberg.com/2007/11/19/c-goes-to-operator/
		// useful for while loops
		alib::PixelRGBA getValue(double parentu, double parentv) override
		{
			// find which most visible child the uv coordinates hit
			// ask that child for a value.
			int count = fChildren.size();
			double myu = (parentu - fMovingFrame.fX) / fMovingFrame.fWidth;
			double myv = (parentv - fMovingFrame.fY) / fMovingFrame.fHeight;

			while (count-- > 0)
			{
				if (fChildren[count]->contains(myu, myv))
					return fChildren[count]->getValue(myu, myv);
			}

			if (nullptr != fBackground)
				return fBackground->getValue(myu, myv);

			return alib::PixelRGBA(0x0);
		}

	};
}
// namespace vdj

using SharedSamplerWrapper = std::shared_ptr<vdj::SamplerWrapper>;
using SamplerWrapperWindow = vdj::SampledWindow<vdj::SamplerWrapper>;
