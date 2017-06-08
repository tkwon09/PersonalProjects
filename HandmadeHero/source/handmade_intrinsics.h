#if !defined(HANDMADE_INTRINSICS_H)
/* ------------------------------------------------
   $File: handmade_intrinsics.h
   $Date: 8/28/2015
   $Creator: Casey Muratori
   ------------------------------------------------ */

/*
  STUDY: The purpose of the intrinsics file is for performance.
  This is the place where we make a compromise on the strictly segregated platform and game layers.
  Only place we are allowed to do platform-specific stuff that can be directly included into platform non-specific stuff.
  ONLY REASON IS FOR PERFORMANCE.
 */

//
// TODO: Convert all of these to platform-efficient versions and remove math.h
//

#include "math.h"

inline int32
SignOf(int32 Value)
{
    int32 Result = (Value >= 0)? 1 : -1;

    return Result;
}

inline real32
SquareRoot(real32 Real32)
{
    real32 Result = sqrtf(Real32);

    return Result;
}

inline real32
AbsoluteValue(real32 Real32)
{
    real32 Result = fabs(Real32);
    return Result;
}

inline uint32
RotateLeft(uint32 Value, int32 Amount)
{
#if COMPILER_MSVC
    uint32 Result = _rotl(Value, Amount);
#else
    // TODO : Actually port this to other compiler platforms!
    Amount &= 31;
    uint32 Result = ((Value << Amount) | (Value >> (32 - Amount)));
#endif
    
    return Result;
}

inline uint32
RotateRight(uint32 Value, int32 Amount)
{
#if COMPILER_MSVC
    uint32 Result = _rotr(Value, Amount);
#else
    // TODO : Actually port this to other compiler platforms!
    uint32 Result = ((Value >> Amount) | (Value << (32 - Amount)));
#endif
    
    return Result;
}

inline uint32
RoundReal32ToInt32(real32 Real32)
{
     int32 Result = (int32) roundf(Real32);
     return Result;
}

inline uint32
RoundReal32ToUInt32(real32 Real32)
{
     uint32 Result = (uint32) roundf(Real32);
     return Result;
}

inline int32
FloorReal32ToInt32(real32 Real32)
{
     int32 Result = (int32) floorf(Real32);
     return Result;
}

inline int32
CeilReal32ToInt32(real32 Real32)
{
     int32 Result = (int32) ceilf(Real32);
     return Result;
}

inline int32
TruncateReal32ToInt32(real32 Real32)
{
     int32 Result = (int32) Real32;
     return Result;
}

inline real32
Sin(real32 Angle)
{
     real32 Result = sinf(Angle);
     return Result;
}

inline real32
Cos(real32 Angle)
{
     real32 Result = cosf(Angle);
     return Result;
}

inline real32
ATan2(real32 Y, real32 X)
{
     real32 Result = atan2f(Y, X);
     return Result;
}

struct bit_scan_result
{
     bool32 Found;
     uint32 Index;
};
inline bit_scan_result
FindLeastSignificantSetBit(uint32 Value)
{
     bit_scan_result Result = {};

     // Check to see if our compiler supports a fast scan
#if COMPILER_MSVC
     // _BitScanForward searches the mask data from LSB to MSB for a set
     // https://msdn.microsoft.com/en-us/library/wfd9z0bb.aspx
     Result.Found = _BitScanForward((unsigned long *) &Result.Index, Value);
     // the thingy converts to bsf eax, eax
     // 1 instruction :D
#else
     // otherwise do this ugly crap...
     for (uint32 Test = 0; Test < 32; ++Test)
     {
      if (Value & (1 << Test))
      {
           Result.Index = Test;
           Result.Found = true;
           break;
      }
     }
#endif
     return Result;
}

#define HANDMADE_INTRINSICS_H
#endif
