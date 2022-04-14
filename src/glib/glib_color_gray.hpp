#pragma once

#include "glib_basics.hpp"
#include "glib_color_rgba.hpp"

namespace glib
{
	template <class Colorspace>
	struct gray8T
	{
		typedef uint8_t value_type;
		typedef uint32_t calc_type;
		typedef int32_t		long_type;

		enum base_scale_e
		{
			base_shift = 8,
			base_scale = 1 << base_shift,
			base_mask = base_scale -1,
			base_MSB = 1 << (base_shift -1)
		};
		typedef gray8T self_type;

		value_type v;	// The value
		value_type a;	// The alpha

		// Constructors
		gray8T() = default;
		explicit gray8T(unsigned v_, unsigned a_ = base_mask)
			:v(uint8_t(v_)), a(uint8_t(a_)) {}
		gray8T(const self_type& c, unsigned a_)
			:v(c.v), a(value_type(a_)) {}
		gray8T(const rgba& c)
			:v(luminance(c)),
			a(value_type(uround(c.a*base_mask))){}
		template <typename T>
		gray8T(const gray8T<T>& c)
		{
			convert(*this, c);
		}

		template<typename T>
		gray8T(const rgba8T<T>& c)
		{
			convert(*this, c);
		}
		// Create luminance according to ITU-R BT.709
		static value_type luminance(const rgba& c)
		{
			return value_type(uround((0.2126 * c.r + 0.7152 * c.g + 0.0722 * c.b) * base_mask));
		}

		static value_type luminance(const rgba8& c)
		{
			return value_type((55u * c.r + 184u * c.g + 18u * c.b) >> 8);
		}

		static void convert(gray8T<linear>& dst, const gray8T<sRGB>& src)
		{
			dst.v = sRGB_conv<value_type>::rgb_from_sRGB(src.v);
			dst.a = src.a;
		}

		static void convert(gray8T<sRGB>& dst, const gray8T<linear>& src)
		{
			dst.v = sRGB_conv<value_type>::rgb_to_sRGB(src.v);
			dst.a = src.a;
		}

		static void convert(gray8T<linear>& dst, const rgba8& src)
		{
			dst.v = luminance(src);
			dst.a = src.a;
		}

		static void convert(gray8T<linear>& dst, const srgba8& src)
		{
			convert(dst, rgba8(src));
		}

		static void convert(gray8T<sRGB>& dst, const rgba8& src)
		{
			dst.v = sRGB_conv<value_type>::rgb_tosRGB(luminance(src));
			dst.a = src.a;
		}

		static void convert(gray8T<sRGB>& dst, const srgba8& src)
		{
			convert(dst, rgba8(src));
		}


	};
}