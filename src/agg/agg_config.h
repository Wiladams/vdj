#ifndef AGG_CONFIG_INCLUDED
#define AGG_CONFIG_INCLUDED

// This file can be used to redefine certain data types.

//---------------------------------------
// 1. Default basic types such as:
// 
// AGG_INT8
// AGG_INT8U
// AGG_INT16
// AGG_INT16U
// AGG_INT32
// AGG_INT32U
// AGG_INT64
// AGG_INT64U
//
// Just replace this file with new defines if necessary. 
// For example, if your compiler doesn't have a 64 bit integer type
// you can still use AGG if you define the follows:
//
// #define AGG_INT64  int
// #define AGG_INT64U unsigned
//
// It will result in overflow in 16 bit-per-component image/pattern resampling
// but it won't result any crash and the rest of the library will remain 
// fully functional.

#define AGG_INT8   int8_t         //----int8
#define AGG_INT8U  uint8_t        //----int8u
#define AGG_INT16  int16_t        //----int16
#define AGG_INT16U uint16_t       //----int16u
#define AGG_INT32  int32_t        //----int32
#define AGG_INT32U uint32_t       //----int32u
#define AGG_INT64  int64_t        //----int64
#define AGG_INT64U uint64_t       //----int64u

//---------------------------------------
// 2. Default rendering_buffer type. Can be:
//
// Provides faster access for massive pixel operations, 
// such as blur, image filtering:
// #define AGG_RENDERING_BUFFER row_ptr_cache<int8u>
// 
// Provides cheaper creation and destruction (no mem allocs):
// #define AGG_RENDERING_BUFFER row_accessor<int8u>
//
// You can still use both of them simultaneously in your applications 
// This #define is used only for default rendering_buffer type,
// in short hand typedefs like pixfmt_rgba32.

#endif
