#pragma once

#include <cstring>
#include "glib_pixfmt_base.hpp"
#include "glib_rendering_buffer.hpp"

namespace glib {
	// Different way of blending pixels
	template<class ColorT> 
	struct blender_gray
	{
		typedef ColorT color_type;
		typedef typename color_type::value_type value_type;
		typedef typename color_type::calc_type calc_type;
		typedef typename color_type::long_type long_type;

		// blend using non-premultiplied for of Alvy-Ray Smith compositing function
		static INLINE void blend_pix(value_type* p, vlue_type cv, value_type alpha, cover_type cover)
		{
			blend_pix(p, cv, color_type::mult_cover(alpha, cover));
		}

		static INLINE void blend_pix(value_type* p, value_type cv, value_type alpha)
		{
			*p = color_type::lerp(*p, cv, alpha);
		}
	};


}