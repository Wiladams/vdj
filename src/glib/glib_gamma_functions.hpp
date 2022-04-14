#pragma once

#include "apidefs.h"
#include <cmath>

namespace glib 
{
	// These structures are functors
	// You set them up with some state, then
	// you can call them like a function, because
	// of the operator() overload
	//
	struct gamma_none 
	{
		double operator()(double x) const { return x; }
	};

	struct gamma_power
	{
		gamma_power() :fGamma(1.0) {}
		gamma_power(double g) : fGamma(g) {}

		void gamma(double g) { fGamma = g; }
		double gamma() const { return fGamma; }

		double operator() (double x) const
		{
			return std::pow(x, fGamma);
		}
	private:
		double fGamma;
	};

	struct gamma_threshold 
	{
		gamma_threshold() :fThreshold(0.5) {}
		gamma_threshold(double t) : fThreshold(t) {}

		void threshold(double t) { fThreshold = t; }
		double threshold() const { return fThreshold; }

		double operator() (double x) const
		{
			return (x < fThreshold) ? 0.0 : 1.0;
		}

	private:
		double fThreshold;
	};

	struct gamma_linear 
	{
		gamma_linear() :fStart(0.0), fEnd(1.0) {}
		gamma_linear(double s, double e) :fStart(s), fEnd(e) {}

		void set(double s, double e) { fStart = s; fEnd = e; }
		
		void start(double s) { fStart = s; }
		double start() const { return fStart; }

		void end(double e) { fEnd = e; }
		double end() const { return fEnd; }

		double operator() (double x) const
		{
			if (x < fStart) return 0.0;
			if (x > fEnd) return 1.0;
			return (x - fStart) / (fEnd - fStart);
		}

	private:
		double fStart;
		double fEnd;
	};

	struct gamma_multiply 
	{
		gamma_multiply() :fMul(1.0) {}
		gamma_multiply(double v) :fMul(v) {}

		void value(double v) { fMul = v; }
		double value() const { return fMul; }

		double operator()(double x) const
		{
			double y = x * fMul;
			if (y > 1.0) y = 1.0;
			return y;
		}
	private:
		double fMul;
	};

	INLINE double sRGB_to_linear(double x) 
	{
		return (x <= 0.04045) ? (x / 12.92) : pow((x + 0.055) / (1.055), 2.4);
	}

	INLINE double linear_to_sRGB(double x) 
	{
		return (x <= 0.0031308) ? (x * 12.92) : (1.055 * pow(x, 1 / 2.4) - 0.055);
	}
}