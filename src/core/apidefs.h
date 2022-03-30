#pragma once

#include <cstdint>
#include <cstddef>



#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4201)  // nonstandard extension used : nameless struct/union
#endif


#if defined(_MSC_VER)
#define INLINE __forceinline
//#include <intrin.h>
#elif defined(__GNUC__)
#define INLINE inline __attribute__ ((always_inline))
#else
#error This compiler is unsupported!
#endif

// EXPORT allows us to declare a function as exported
// this will make it possible to then lookup that 
// function in the .dll using GetProcAddress
// So, if that's needed, just put EXPORT at the front
// of a declaration.
#define APP_EXPORT		__declspec(dllexport)
//#define APP_EXPORT

#define APP_EXTERN  extern


#ifdef __BIG_ENDIAN__
#define APP_BIG_ENDIAN	1
#else
#define APP_LITTLE_ENDIAN 1
#endif


// Random programming reference
// 
// Reference: http://www.guyrutenberg.com/2007/11/19/c-goes-to-operator/
// useful for while loops
