#pragma once

#include "apidefs.h"

#include <assert.h>
#include <memory>

#define SIMD_ALIGN 64

namespace vdj
{
    INLINE void* Allocate(size_t size, size_t align = SIMD_ALIGN)
    {
#ifdef SIMD_NO_MANS_LAND
        size += 2 * SIMD_NO_MANS_LAND;
#endif
        void* ptr = NULL;
#if defined(_MSC_VER) 
        ptr = _aligned_malloc(size, align);
#elif defined(__MINGW32__) || defined(__MINGW64__)
        ptr = __mingw_aligned_malloc(size, align);
#elif defined(__GNUC__)
        align = AlignHi(align, sizeof(void*));
        size = AlignHi(size, align);
        int result = ::posix_memalign(&ptr, align, size);
        if (result != 0)
            ptr = NULL;
#else
        ptr = malloc(size);
#endif
#ifdef SIMD_ALLOCATE_ERROR_MESSAGE
        if (ptr == NULL)
            std::cout << "The function posix_memalign can't allocate " << size << " bytes with align " << align << " !" << std::endl << std::flush;
#endif
#ifdef SIMD_ALLOCATE_ASSERT
        assert(ptr);
#endif
#ifdef SIMD_NO_MANS_LAND
        if (ptr)
            ptr = (char*)ptr + SIMD_NO_MANS_LAND;
#endif
        return ptr;
    }

    template<class T> T* Allocate(uint8_t*& buffer, size_t size, size_t align = SIMD_ALIGN)
    {
        T* ptr = (T*)buffer;
        buffer = buffer + AlignHi(size * sizeof(T), align);
        return ptr;
    }

    INLINE void Free(void* ptr)
    {
#ifdef SIMD_NO_MANS_LAND
        if (ptr)
            ptr = (char*)ptr - SIMD_NO_MANS_LAND;
#endif
#if defined(_MSC_VER) 
        _aligned_free(ptr);
#elif defined(__MINGW32__) || defined(__MINGW64__)
        return __mingw_aligned_free(ptr);
#else
        free(ptr);
#endif
    }

    //---------------------------------------------------------------------------------------------
    struct Deletable
    {
        virtual ~Deletable() {}
    };
}