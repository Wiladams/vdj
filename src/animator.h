#pragma once

#include "apidefs.h"


#include "pixeltypes.h"
#include "easing.h"
#include "interpolator.h"
#include "stopwatch.h"
#include "normalizedwindow.h"

#include <functional>
#include <vector>
#include <memory>

namespace vdj
{
	// Used for setting the value of a field
	template <typename T>
	using PFieldSetter = std::function< void(const T& value)>;

	// Used for getting the value of a field
	template <typename T>
	using PFieldGetter = std::function< T()>;

	//
	// IAnimateField
	// Abstract base class for animators.  The sub-class
	// MUST implement onUpdate(double u);
	//
	struct IAnimateField
	{
	protected:
		PFunc1Double fEasing = easing::linear;
		double fStartProgress = 0.0;
		double fEndProgress = 1.0;

	public:
		IAnimateField() = default;

		IAnimateField(double startTime, double endTime)
			:fStartProgress(startTime),
			fEndProgress(endTime)
		{

		}

		virtual ~IAnimateField() {}

		void setProgressEnvelope(double startTime, double endTime)
		{
			fStartProgress = startTime;
			fEndProgress = endTime;
		}

		virtual void setProgress(double u)
		{
			onUpdate(u);
		}

		virtual void updateField(double u)
		{
			//if ((u < fStartTime) || (u>fEndTime))
			//	return;
			if (u < fStartProgress)
				return;


			double u1 = (u - fStartProgress) / (fEndProgress - fStartProgress);
			u1 = maths::Clamp(u1, 0, 1);

			onUpdate(u1);
		}

		virtual void onUpdate(double u) = 0;

		void setEasing(PFunc1Double ease)
		{
			fEasing = ease;
		}
	};

	//
	// SamplerPositionAnimation
	// Most general base class for position based animations
	// The default behavior is to change the frame of the panel
	// interpolating between a beginning and ending position
	struct SamplerPositionAnimation : public IAnimateField
	{
		SharedSamplerWrapper fSampler;
		RectD fBeginPos;
		RectD fEndPos;

		SamplerPositionAnimation(SharedSamplerWrapper wrapped,
			const RectD& beginPos,
			const RectD& endPos)
			: fSampler(wrapped)
			, fBeginPos(beginPos)
			, fEndPos(endPos)
		{}

		// By default, just adjust frame
		void onUpdate(double u) override
		{
			double u1 = fEasing(u);
			auto newValue = maths::Lerp(u1, fBeginPos, fEndPos);

			fSampler->setFrame(newValue);
		}

		static std::shared_ptr< SamplerPositionAnimation> create(SharedSamplerWrapper wrapped,
			const RectD& beginPos,
			const RectD& endPos)
		{
			return std::make_shared< SamplerPositionAnimation>(wrapped, beginPos, endPos);
		}
	};



	//
	// AnimationWindow
	//
	// Animation of a Window.  The actual animation is 
	// up to a sub-class.  Any attribute of the window
	// can be animated, and interpolators are required
	// to implement the intended animation.
	//
	struct AnimationWindow : public SampledWindow
	{
		bool fIsRunning = false;

		// Time associations
		StopWatch fTimer;
		double fDuration = 0;
		double fStartTime = 0;
		double fEndTime = 0;
		double fProgress = 0;	// Value from [0..1]
		int fDirection = 1;

		std::vector<std::shared_ptr<IAnimateField> > fAnimators;

		AnimationWindow(double duration)
			:fDuration(duration)
		{}

		void setDirection(int dir) { fDirection = dir; }

		virtual void setDuration(double dur) { fDuration = dur; }
		void setEasing(PFunc1Double ease)
		{
			for (auto& animator : fAnimators) {
				animator->setEasing(ease);
			}
		}

		void addMotion(std::shared_ptr<IAnimateField> animator)
		{
			fAnimators.push_back(animator);
		}

		void addMovingSample(std::shared_ptr<SampledWindow> win, std::shared_ptr<IAnimateField> animator)
		{
			addChild(win);
			addMotion(animator);
		}

		// This will reset the animation to 
		// its starting point.  It just resets
		// the start time and progress
		// BUGBUG
		// WAA - The problem here is that although 
		// we start, progress is governed by calling
		// udate(), or setProgress()
		// if neither of those occur, we don't actually
		// progress
		void start() noexcept
		{
			// reset all the animators
			for (auto& anim : fAnimators)
				anim->setProgress(0);

			// Reset the start time
			fStartTime = fTimer.seconds();
			fEndTime = fStartTime + fDuration;

			fProgress = 0;
			fIsRunning = true;
		}

		bool isRunning() { return fIsRunning; }

		// simply return the progress field
		INLINE double constexpr progress() noexcept { return fProgress; }

		// If not calling update, the animator can call setProgress
	// explicitly.  This is useful when you're 'scrubbing' through
	// a timeline and want to control exactly how far along the effect is
		void setProgress(double u)
		{
			fProgress = maths::Clamp(u, 0.0, 1.0);
			onProgress(fProgress);
		}

		// externally, an animator must call update for 
		// progress to be made.  You need to either call this
		// or set the progress manually.
		void update() noexcept
		{
			if (!isRunning())
				return;

			auto u = maths::Map(fTimer.seconds(), fStartTime, fEndTime, 0.0, 1.0);
			if (fDirection < 0)
				u = 1.0 - u;

			fProgress = maths::Clamp(u, 0.0, 1.0);

			onProgress(fProgress);

			if (fProgress >= 1.0)
				fIsRunning = false;
		}

		// A function to be implemented by sub-classes
		// by default, just tell all the children
		// of the progress
		virtual void onProgress(double u)
		{
			//onFrameChanged();

			for (auto& motion : fAnimators)
			{
				motion->updateField(u);
			}
		}

	};
}
// namespace vdj