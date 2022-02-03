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

// Used for setting the value of a field
template <typename T>
using PFieldSetter = std::function< void(const T& value)>;

// Used for getting the value of a field
template <typename T>
using PFieldGetter = std::function< T ()>;


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


		double u1 = (u - fStartProgress)/(fEndProgress - fStartProgress);
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
// Animotion
// This defines the motion associated with an animation
// it is templatized, because an animation can decide to 
// attach motion to any number of attributes within an animation


template <typename T>
struct FieldAnimator : public IAnimateField
{
private:
	T fStartValue;
	T fEndValue;
	T& fField;

	PFieldGetter<T> fGetter;
	PFieldSetter<T> fSetter;

protected:


public:
	///*
	FieldAnimator(T& field, const T& startValue, const T& endValue)
		: IAnimateField(0.0,1.0)
		,fField(field)
		,fStartValue(startValue)
		,fEndValue(endValue)
	{}

	FieldAnimator(T& field, const T& startValue, const T& endValue, double startTime, double endTime)
		: IAnimateField(startTime, endTime)
		, fField(field)
		, fStartValue(startValue)
		, fEndValue(endValue)
	{}
	//*/
	/*
	FieldAnimator(PFieldGetter<T> getter, PFieldSetter<T> setter, const T& startValue, const T& endValue)
		: IAnimateField(0.0, 1.0)
		, fGetter(getter)
		, fSetter(setter)
		, fStartValue(startValue)
		, fEndValue(endValue)
	{}

	FieldAnimator(PFieldGetter<T> getter, PFieldSetter<T> setter, const T& startValue, const T& endValue, double startTime, double endTime)
		: IAnimateField(startTime, endTime)
		, fGetter(getter)
		, fSetter(setter)
		, fStartValue(startValue)
		, fEndValue(endValue)
	{}
	*/
	// When progress is made, alter the field
	// 
	void onUpdate(double u) override
	{
		//if (fSetter != nullptr)
		//	fSetter(this->operator()(u));
		fField = this->operator()(u);
	}

	T operator() (double u)
	{
		double u1 = fEasing(u);
		T newValue = maths::Lerp(u1, fStartValue, fEndValue);
		return newValue;
	}
};

using TexelRectMotion = FieldAnimator<RectD>;
using RectMotion = FieldAnimator<RectD>;



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
	std::vector<std::shared_ptr<IAnimateField> > fAnimators;

	AnimationWindow(double duration)
		:fDuration(duration)
	{}

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
		fProgress = maths::Clamp(u,0.0,1.0);
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
		onFrameChanged();

		for (auto& motion : fAnimators)
		{
			motion->updateField(u);
		}
	}

};


using SourceSampler = std::shared_ptr<ISample2D<PixelRGBA> >;
