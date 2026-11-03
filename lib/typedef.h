#ifndef typedef_h
#define typedef_h

#include <stdint.h>

#define Assert(Asertion)                                                       \
  if (!(Asertion)) {                                                           \
    *(s32 *)0 = 0;                                                             \
  }
// Base types
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef s32 b32; // 32-bit boolean value

typedef float f32;
typedef double f64;

#define ArrayCount(Array) (sizeof(Array) / sizeof(*Array))

s64 CharCount(char *Str) {
  s64 Result = 0;
  while (*Str++ != 0) {
    Result++;
  }
  return Result;
}

#define VORTEX_GUI_PORT 2100
#endif
