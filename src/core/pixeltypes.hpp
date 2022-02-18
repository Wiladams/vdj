#pragma once

#include "apidefs.h"
#include "coloring.h"
#include "maths.hpp"
#include "geotypes.hpp"

#include <cassert>
#include <array>
#include <cstdint>
#include <memory>

namespace vdj {
    using RectD = GeoRect<double>;

    using PixelBezier = GeoCubicBezier<ptrdiff_t>;
    using PixelEllipse = GeoEllipse<ptrdiff_t>;
    using PixelRect = GeoRect<ptrdiff_t>;
    using PixelSpan = GeoSpan<ptrdiff_t>;
    using PixelTriangle = GeoTriangle<ptrdiff_t>;
    using PixelPath = GeoPath<ptrdiff_t > ;

    /*

    In this case, PixelRGBA has 4 byte sized(8 - bit) values to represent
    each of the R, G, B, A values.Some modern day monitorsand graphics
    systems in general, are capable of using 10 - bit or even 16 - bit values.
    As this is not universally common, we'll stick to 8-bit components
    for now.

    If you were to set a PixelRGBA by its integer value, it would layout
    on a little endian system as follows :

    0xAARRGGBB
    */
    struct PixelRGBA
    {
        uint32_t value;

        // default constructor, transparent
        INLINE PixelRGBA() noexcept = default;
        INLINE constexpr PixelRGBA(const PixelRGBA& rhs) noexcept = default;
        INLINE constexpr explicit PixelRGBA(uint32_t val) noexcept : value(val) {}
        INLINE PixelRGBA(uint32_t r, uint32_t g, uint32_t b) noexcept : value((r << 16) | (g << 8) | b | (0xffu << 24)) {}
        INLINE PixelRGBA(uint32_t r, uint32_t g, uint32_t b, uint32_t a) noexcept : value((r << 16) | (g << 8) | b | (a << 24)) {}

        // bool operator for fast comparison to fully transparent
        INLINE explicit operator bool() const noexcept { return value != 0; }
        INLINE PixelRGBA& operator=(const PixelRGBA& other) noexcept = default;
        INLINE constexpr bool operator==(const PixelRGBA& other) const noexcept { return equals(other); }

        //INLINE operator ColorRGBA() { return ColorRGBA((float)r()/255.0f,g()/255.0f,b()/255.0f,a()/255.0f); }

        INLINE constexpr bool equals(const PixelRGBA& other) const noexcept { return value == other.value; }

        INLINE constexpr uint32_t r() const noexcept { return (value >> 16) & 0xffu; }
        INLINE constexpr uint32_t g() const noexcept { return (value >> 8) & 0xffu; }
        INLINE constexpr uint32_t b() const noexcept { return (value >> 0) & 0xffu; }
        INLINE constexpr uint32_t a() const noexcept { return (value >> 24) & 0xffu; }

        INLINE void setR(uint32_t r) noexcept { value = (value & 0xff00ffffu) | (r << 16); }
        INLINE void setG(uint32_t g) noexcept { value = (value & 0xffff00ffu) | (g << 8); }
        INLINE void setB(uint32_t b) noexcept { value = (value & 0xffffff00u) | (b << 0); }
        INLINE void setA(uint32_t a) noexcept { value = (value & 0x00ffffffu) | (a << 24); }

        INLINE constexpr bool isOpaque() const noexcept { return value >= 0xff000000u; }
        INLINE constexpr bool isTransparent() const noexcept { return value <= 0x00ffffff; }
    };



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

        INLINE constexpr uint32_t toLuminance(const PixelRGBA &p) const
        {
            return redfactor[p.r()] + greenfactor[p.g()] + bluefactor[p.b()];
        }
    };
}
// namespace vdj
