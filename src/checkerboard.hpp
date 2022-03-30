#pragma once

//#include "draw2d.hpp"
#include "sampler.hpp"
#include "maths.hpp"
#include "vdjPoint.hpp"

#include <memory>


//
// Commentary
// Using a checkerboard pattern is a common thing to do in 
// graphics development.  It can serve as a background, letting
// you know when drawing is not present (good for alpha blend testing)
// Or it can be used as a common texture to be wrapped around 
// 3D objects.
// Here we have the sampler and a graphic object.  They are 
// separate because the sampler typically responds to
// parametric u,v values (betwee 0 and 1), whereas the graphic
// object is intended to match actual pixel space, so it 
// has pixel dimensions.
//
// The two could be combined, but I think it's cleaner and
// more versatile to keep them separate.
//
// To get a checkerboard, we're essentially running two
// threshold patterns using a sine wave
// We use the sine function because it ranges from -1 to +1
// over the period of 0 to 2*pi
// from 0 - pi it is positive
// from pi - 2pi it is negative
// We can use this fact to select between two colors
//

namespace alib {
    class CheckerPattern
    {
        double fFactor;

        // simple step function
        // if the value 'u' is below the threshold, then the low
        // value is returned, otherwise the high value is returned
        // pretty simple ternary operation
        static double step(double u, double threshold, double low, double high)
        {
            return u < threshold ? low : high;
        }

    public:
        CheckerPattern(double freq)
        {
            setFrequency(freq);
        }

        void setFrequency(double freq)
        {
            // we don't actually need to retain the frequency
            // we just use it to make this calculation
            // we'll use the fFactor in the getValue() routine
            fFactor = freq * (2 * alib::Pi);
        }

        // u and v range from 0 to 1 inclusive
        // We want to turn that range into values
        // based on the frequency and colors given
        // at construction time
        virtual bool evalParam(double u, double v) const
        {
            double xrad = u * fFactor;
            double yrad = v * fFactor;

            // we need values that are either 1 or -1
            // we change sign at 0
            auto stepu = step(sin(xrad), 0, -1, 1);
            auto stepv = step(sin(yrad), 0, -1, 1);

            auto sines = stepu * stepv;

            return sines <= 0 ? false : true;
        }
    };

    class CheckerSampler : public ISample2D<PixelRGBA>
    {
        CheckerPattern fPattern;
        std::shared_ptr<ISample2D<PixelRGBA> > t1;   // First sampler
        std::shared_ptr<ISample2D<PixelRGBA> > t2;   // Second sampler

    public:
        CheckerSampler(int freq, const PixelRGBA& c1, const PixelRGBA& c2)
            :CheckerSampler(freq, std::make_shared< SolidColorSampler>(c1), std::make_shared< SolidColorSampler>(c2))
        {}

        CheckerSampler(double freq,
            std::shared_ptr<ISample2D<PixelRGBA> > s1,
            std::shared_ptr<ISample2D<PixelRGBA> > s2)
            : fPattern(freq),
            t1(s1), t2(s2)
        {}

        void setFrequency(double freq)
        {
            fPattern.setFrequency(freq);
        }

        PixelRGBA getValue(double u, double v) override
        {
            auto which = fPattern.evalParam(u, v);
            if (which)
                return t2->getValue(u, v);

            return t1->getValue(u, v);
        }
    };
}
