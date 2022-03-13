#pragma once


#include "vec.h"
#include "pixeltypes.hpp"

#include <cmath>

namespace vdj {
    //using ColorRGBA = float4;
    //using ColorRGB = float3;

    class NTSCGray
    {
        // Based on old NTSC
        //static float redcoeff = 0.299f;
        //static float greencoeff = 0.587f;
        //static float bluecoeff = 0.114f;

        // New CRT and HDTV phosphors
        const float redcoeff = 0.2225f;
        const float greencoeff = 0.7154f;
        const float bluecoeff = 0.0721f;

        // use lookup tables to save ourselves from multiplications later
        std::array<uint8_t, 256> redfactor;
        std::array<uint8_t, 256> greenfactor;
        std::array<uint8_t, 256> bluefactor;

    public:
        NTSCGray() :NTSCGray(0.2225f, 0.7154f, 0.0721f) {}

        // Create an instance, initializing with the coefficients you desire
        NTSCGray(float rcoeff, float gcoeff, float bcoeff)
            :redcoeff(rcoeff), greencoeff(gcoeff), bluecoeff(bcoeff)
        {
            // construct lookup tables
            for (int counter = 0; counter < 256; counter++)
            {
                redfactor[counter] = (uint8_t)maths::Min(56, (int)maths::Floor((counter * redcoeff) + 0.5f));
                greenfactor[counter] = (uint8_t)maths::Min(181, (int)maths::Floor((counter * greencoeff) + 0.5f));
                bluefactor[counter] = (uint8_t)maths::Min(18, (int)maths::Floor((counter * bluecoeff) + 0.5f));
            }
        }

        INLINE constexpr uint32_t toLuminance(uint8_t r, uint8_t g, uint8_t b) const
        {
            return redfactor[r] + greenfactor[g] + bluefactor[b];
        }

        INLINE constexpr uint32_t toLuminance(const PixelRGBA& p) const
        {
            return redfactor[p.red()] + greenfactor[p.green()] + bluefactor[p.blue()];
        }
    };
} // namespace vdj
