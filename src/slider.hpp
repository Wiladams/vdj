#pragma once

#include "maths.hpp"

namespace vdj {

	//
	// Slider, a class for constraining movement
	// while reporting position and intermediate value
	template <typename T>
	struct Slider
	{
		// position [0..1] within the motion range
		// value [low..high] 
		T lowValue;
		T highValue;
		double fPosition;

		Slider(const T& low, const T& high, double initialpos);

		double getPosition()const ;
		T getValue() const;


	};

	template <typename T>
	Slider<T>::Slider(const T& low, const T& high, double initialpos)
		:lowValue(low)
		,highValue(high)
		,fPosition(initialpos)
	{}

	template <typename T>
	double Slider<T>::getPosition() const { return fPosition; }

	template <typename T>
	T Slider<T>::getValue() const
	{
		// do linear interpolation between low
		// and high values
		return maths::Lerp(fPosition, lowValue, highValue);
	}
}

