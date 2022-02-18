#pragma once

#include "vdjview.hpp"

#include <cstdint>


//
// As a general style and compile thing, we don't attach any behavior
// within the declaration of the data structure.  So, the structure
// can be pretty clean
//

namespace vdj {
	
	namespace Pixel
	{
		// Forward declarations of pixel structures
		struct Bgr24;
		struct Bgra32;
		struct Hsv24;
		struct Hsl24;
		struct Rgb24;
		struct Rgba32;

		// Bgr24 Declaration
		struct Bgr24
		{
			uint8_t blue;
			uint8_t green;
			uint8_t red;

			Bgr24(const uint8_t& gray = uint8_t(0));
			Bgr24(const uint8_t& b, const uint8_t& g, const uint8_t& r);
			Bgr24(const Bgra32& p);
			Bgr24(const Rgb24& p);
			Bgr24(const Rgba32& p);
			Bgr24(const Bgr24& p);

			//template <template<class> class A> static const Bgr24& At(const View<A>& view, ptrdiff_t col, ptrdiff_t row);
			//template <template<class> class A> static Bgr24& At(View<A>& view, ptrdiff_t col, ptrdiff_t row);

		};





		// Bgra32 declaration
		struct Bgra32
		{
			uint8_t blue;
			uint8_t green;
			uint8_t red;
			uint8_t alpha;

			Bgra32(const uint8_t& gray = uint8_t(0), const uint8_t& a = uint8_t(255));
			Bgra32(const uint8_t& b, const uint8_t& g, const uint8_t& r, const uint8_t& a = uint8_t(255));
			Bgra32(const Bgr24& p, const uint8_t& a = uint8_t(255));
			Bgra32(const Rgb24& p, const uint8_t& a = uint8_t(255));
			Bgra32(const Rgba32& p);
			Bgra32(const Bgra32& p);

			//template <template<class> class A> static const Bgra32& At(const View<A>& view, ptrdiff_t col, ptrdiff_t row);
			//template <template<class> class A> static Bgra32& At(View<A>& view, ptrdiff_t col, ptrdiff_t row);

		};



		// Hsv24 Declaration
		struct Hsv24
		{
			uint8_t hue;
			uint8_t saturation;
			uint8_t value;
		};

		// Hsl24 Declaration
		struct Hsl24
		{
			uint8_t hue;
			uint8_t saturation;
			uint8_t lightness;
		};

		struct Rgb24 
		{
			uint8_t red;
			uint8_t green;
			uint8_t blue;
		};
		
		struct Rgba32 
		{
			uint8_t red;
			uint8_t green;
			uint8_t blue;
			uint8_t alpha;

			Rgba32(const uint8_t& gray = uint8_t(0), const uint8_t& a = uint8_t(255));
			Rgba32(const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a = uint8_t(255));

			Rgba32(const Rgb24& p, const uint8_t& a = uint8_t(255));
			Rgba32(const Rgba32& p);

			Rgba32(const Bgra32& p);
			Rgba32(const Bgr24& p, const uint8_t& a = uint8_t(255));

			//template <template<class> class A> static const Rgba32& At(const View<A>& view, ptrdiff_t col, ptrdiff_t row);
			//template <template<class> class A> static Rgba32& At(View<A>& view, ptrdiff_t col, ptrdiff_t row);

		};

		//
		// ==== IMPLEMENTATIONS ====
		// These MUST be down here at the bottom so that all the types are
		// fully implemented.  Otherwise, you will have errors complaining
		// about references to fields in incomplete types
		// 
		
		// Bgr24 Implementation
		Bgr24::Bgr24(const uint8_t& gray)
			:blue(gray)
			, green(gray)
			, red(gray)
		{

		}

		Bgr24::Bgr24(const uint8_t& b, const uint8_t& g, const uint8_t& r)
			:blue(b)
			, green(g)
			, red(r)
		{
		}

		Bgr24::Bgr24(const Bgra32& p)
			:blue(p.blue)
			, green(p.green)
			, red(p.red)
		{
		}

		Bgr24::Bgr24(const Rgb24& p)
			:blue(p.blue)
			, green(p.green)
			, red(p.red)
		{
		}

		Bgr24::Bgr24(const Rgba32& p)
			:blue(p.blue)
			, green(p.green)
			, red(p.red)
		{
		}

		Bgr24::Bgr24(const Bgr24& p)
			:blue(p.blue)
			, green(p.green)
			, red(p.red)
		{
		}

		// Bgra32 Implementation
		//
		INLINE Bgra32::Bgra32(const uint8_t& gray, const uint8_t& a)
			:blue(gray)
			, green(gray)
			, red(gray)
			, alpha(a)
		{}

		INLINE Bgra32::Bgra32(const uint8_t& b, const uint8_t& g, const uint8_t& r, const uint8_t& a)
			: blue(b)
			, green(g)
			, red(r)
			, alpha(a)
		{}

		INLINE Bgra32::Bgra32(const Bgr24& p, const uint8_t& a)
			: blue(p.blue)
			, green(p.green)
			, red(p.red)
			, alpha(a)
		{}

		INLINE Bgra32::Bgra32(const Rgb24& p, const uint8_t& a)
			: blue(p.blue)
			, green(p.green)
			, red(p.red)
			, alpha(a)
		{}

		INLINE Bgra32::Bgra32(const Rgba32& p)
			: blue(p.blue)
			, green(p.green)
			, red(p.red)
			, alpha(p.alpha)
		{}

		INLINE Bgra32::Bgra32(const Bgra32& p)
			: blue(p.blue)
			, green(p.green)
			, red(p.red)
			, alpha(p.alpha)
		{}


		// Rgba32 Implementation
		INLINE Rgba32::Rgba32(const uint8_t& gray, const uint8_t& a)
			:red(gray)
			,green(gray)
			,blue(gray)
			,alpha(a)
		{

		}

		INLINE Rgba32::Rgba32(const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a)
			:red(r)
			, green(g)
			, blue(b)
			, alpha(a)
		{}

		INLINE Rgba32::Rgba32(const Bgra32& p)
			: red(p.red)
			, green(p.green)
			, blue(p.blue)
			, alpha(p.alpha)
		{
		}

		INLINE Rgba32::Rgba32(const Bgr24& p, const uint8_t& a)
			: red(p.red)
			, green(p.green)
			, blue(p.blue)
			, alpha(a)
		{
		}

		INLINE Rgba32::Rgba32(const Rgb24& p, const uint8_t& a)
			: red(p.red)
			, green(p.green)
			, blue(p.blue)
			, alpha(a)
		{
		}

		INLINE Rgba32::Rgba32(const Rgba32& p)
			: red(p.red)
			, green(p.green)
			, blue(p.blue)
			, alpha(p.alpha)
		{
		}

		/*
		template <template<class> class A> INLINE const Rgba32& Rgba32::At(const View<A>& view, ptrdiff_t col, ptrdiff_t row)
		{
			assert(view.format == View<A>::Rgba32);

			return Simd::At<A, Rgba32>(view, col, row);
		}

		template <template<class> class A> INLINE Rgba32& Rgba32::At(View<A>& view, ptrdiff_t col, ptrdiff_t row)
		{
			assert(view.format == View<A>::Rgba32);

			return Simd::At<A, Rgba32>(view, col, row);
		}
		*/
	}

}