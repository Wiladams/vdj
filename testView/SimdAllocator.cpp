#include "Simd/SimdDefs.h"
#include "Simd/SimdMemory.h"

namespace Simd {

    SIMD_API void* SimdAllocate(size_t size, size_t align)
    {
        return Allocate(size, align);
    }

    SIMD_API void SimdFree(void* ptr)
    {
        Free(ptr);
    }

    SIMD_API size_t SimdAlign(size_t size, size_t align)
    {
        return AlignHi(size, align);
    }
}