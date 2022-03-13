#pragma once

#include "pixeltypes.hpp"
#include "coloring.h"

#include <array>

namespace vdj {
    // The ISample interface is meant to support a generic interface
// for generating color values based on parameters.
// Doing this makes it easy to create color values for different
// rendering situations, without limiting ourselves to bitmaps.
//
// You can create a sample to return a certain type using the 
// teamplate class Sample<T>.
//
// struct SolidColorSample2D : ISample2D<PixelRGBA>
//

// A 1 dimensional sampler
    template <typename T>
    struct ISample1D
    {
        virtual T getValue(double u) = 0;
        T operator()(double u)
        {
            return getValue(u);
        }
    };

    // A 2 dimentional sampler
    template <typename T>
    struct ISample2D
    {
        virtual T getValue(double u, double v) = 0;
        T operator()(double u, double v)
        {
            return getValue(u, v);
        }
    };

    // A 3 dimensional sampler
    template <typename T>
    struct ISample3D
    {
        virtual T getValue(double u, double v, double w) = 0;
        T operator()(double u, double v, double w)
        {
            return getValue(u, v, w);
        }
    };

    template <typename T>
    struct ISampleRGBA :
        public ISample1D<T>,
        public ISample2D<T>,
        public ISample3D<T>
    {

    };

    using SourceSampler = std::shared_ptr<ISample2D<PixelRGBA> >;

    //
    // Some simple samplers
    // Return solid color
    class SolidColorSampler : public ISampleRGBA<PixelRGBA>
    {
        PixelRGBA fColor;

    public:
        SolidColorSampler(uint32_t c) :fColor(c) {}
        SolidColorSampler(const PixelRGBA& c) :fColor(c) {}

        virtual PixelRGBA getValue(double u) override { return fColor; }
        virtual PixelRGBA getValue(double u, double v) override { return fColor; }
        virtual PixelRGBA getValue(double u, double v, double w) override { return fColor; }
    };



    //
    // LumaSampler
    //
    // Converts rgb to a single luminance value.  This goes
    // pretty quick as it uses pre-computed lookup tables
    // so nothing but lookups and additions.  Assuming lookups
    // are faster than the calculations they replace.
    class LumaWrapper : public ISample2D<PixelRGBA>
    {
        NTSCGray fLuminance;
        SourceSampler fWrapped = nullptr;

    public:
        LumaWrapper(SourceSampler wrapped)
            :fWrapped(wrapped)
        {}

        virtual ~LumaWrapper() {}

        PixelRGBA getValue(double u, double v) override
        {
            // get value from our wrapped sampler
            PixelRGBA c = fWrapped->getValue(u, v);

            // convert to grayscale, preserving alpha
            uint8_t g = fLuminance.toLuminance(c);

            // return it
            return PixelRGBA(g, g, g, c.alpha());
        }

        static std::shared_ptr< LumaWrapper> create(SourceSampler wrapped)
        {
            return std::make_shared<LumaWrapper>(wrapped);
        }
    };


    // Some routines in here to deal with colors in various
    // ways.
    // 
    // Utility function.  Turn a visible wavelength into 
    // an RGB color value
    // 380 <= wl <= 780 nanometers
    // gamma typically 1.0
    static inline vdj::PixelRGBA  ColorRGBAFromWavelength(double wl, double gamma = 1.0)
    {

        double red = 0;
        double green = 0;
        double blue = 0;

        if (wl >= 380.0 && wl <= 440.0) {
            red = -1.0f * ((float)wl - 440.0f) / (440.0f - 380.0f);
            blue = 1.0;
        }
        else if (wl >= 440.0 && wl <= 490.0) {
            green = ((float)wl - 440.0f) / (490.0f - 440.0f);
            blue = 1.0;
        }
        else if (wl >= 490.0 && wl <= 510.0) {
            green = 1.0;
            blue = -1.0f * ((float)wl - 510.0f) / (510.0f - 490.0f);
        }
        else if ((wl >= 510.0) && (wl <= 580.0)) {
            red = (float)((wl - 510.0) / (580.0 - 510.0));
            green = 1.0;
        }
        else if (wl >= 580.0 && wl <= 645.0) {
            red = 1.0;
            green = (float)(-1.0 * (wl - 645.0) / (645.0 - 580.0));
        }
        else if (wl >= 645.0 && wl <= 780.0) {
            red = 1.0;
        }

        auto s = 1.0;
        if (wl > 700.0) {
            s = 0.3 + 0.7 * (780.0 - wl) / (780.0 - 700.0);
        }
        else if (wl < 420.0) {
            s = 0.3 + 0.7 * (wl - 380.0) / (420.0 - 380.0);
        }

        if (gamma != 1.0) {
            red = (float)pow(red * s, gamma);
            green = (float)pow(green * s, gamma);
            blue = (float)pow(blue * s, gamma);
        }

        return PixelRGBA((uint32_t)(red * 255), (uint32_t)(green * 255), (uint32_t)(blue * 255), 255);
    }

    class RainbowSampler : public ISample1D<PixelRGBA>
    {
        double fGamma;

    public:
        RainbowSampler()
            :fGamma(1.0)
        {}

        RainbowSampler(double gamma)
            :fGamma(gamma)
        {}

        PixelRGBA getValue(double u) override
        {
            //double wl = maths::Map(u, 0, 1, 380, 780);
            double wl = 380 + (u * 400);

            auto c = ColorRGBAFromWavelength(wl, fGamma);
            return c;
        }
    };
}

