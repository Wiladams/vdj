#pragma once

#include "apidefs.h"

namespace glib
{
	// Allocate an array of an object type
	template <typename T> struct pod_allocator
	{
		static T* allocate(size_t num) { return new T[num]; }
		static void deallocate(T* ptr, size_t) { delete[] ptr; }
	};

	// Allocate a single instance of an object
	template <typename T>struct obj_allocator
	{
		static T* allocate() { return new T; }
		static void deallocate(T* ptr) { delete ptr; }
	};

    INLINE int iround(double v)
    {
        return int((v < 0.0) ? v - 0.5 : v + 0.5);
    }
    INLINE int uround(double v)
    {
        return unsigned(v + 0.5);
    }
    INLINE int ifloor(double v)
    {
        int i = int(v);
        return i - (i > v);
    }
    INLINE unsigned ufloor(double v)
    {
        return unsigned(v);
    }
    INLINE int iceil(double v)
    {
        return int(std::ceil(v));
    }
    INLINE unsigned uceil(double v)
    {
        return unsigned(std::ceil(v));
    }

	typedef uint8_t cover_type;
	enum cover_scale_e
	{
		cover_shift = 8,
		cover_size = 1 << cover_shift,
		cover_mask = cover_size -1,
		cover_none = 0,
		cover_full = cover_mask
	};
}