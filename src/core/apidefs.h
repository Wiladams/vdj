#pragma once

#include <cstdint>



#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4201)  // nonstandard extension used : nameless struct/union
#endif


#ifdef _MSC_VER
#define INLINE __forceinline
#else
#define INLINE inline
#endif

// EXPORT allows us to declare a function as exported
// this will make it possible to then lookup that 
// function in the .dll using GetProcAddress
// So, if that's needed, just put EXPORT at the front
// of a declaration.
#define APP_EXPORT		__declspec(dllexport)
//#define APP_EXPORT

#define APP_EXTERN  extern


// Reference: http://www.guyrutenberg.com/2007/11/19/c-goes-to-operator/
// useful for while loops