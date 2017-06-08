#if !defined(HANDMADE_MATH_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

/*
  STUDY: The union form allows us to access elements using index +
  brackets. But it normally doesn't allow us to initialize using
  braces. The anonymous struct inside the union allows us to this,
  some other compilers may complain about this.
 */
union v2
{
    struct
    {
        real32 X, Y;
    };
    real32 E[2];
};

// NOTE : Considered v2 A = v2{5, 3}; Requires C++11
// Decided not to because of backwards compatibility and it looks weird
inline v2
V2(real32 X, real32 Y)
{
    v2 Result;

    Result.X = X;
    Result.Y = Y;

    return(Result);
}

inline v2
operator*(real32 A, v2 B)
{
    v2 Result;

    Result.X = A * B.X;
    Result.Y = A * B.Y;

    return(Result);
}

inline v2
operator*(v2 B, real32 A)
{
    v2 Result = A * B;

    return(Result);
}

inline v2 &
operator*=(v2 &B, real32 A)
{
    B = A * B;

    return(B);
}

inline v2
operator-(v2 A)
{
    v2 Result;

    Result.X = -A.X;
    Result.Y = -A.Y;

    return(Result);
}

inline v2
operator+(v2 A, v2 B)
{
    v2 Result;

    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;

    return(Result);
}

inline v2 &
operator+=(v2 &A, v2 B)
{
    A = A + B;

    return(A);
}

inline v2
operator-(v2 A, v2 B)
{
    v2 Result;

    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;

    return(Result);
}

inline real32
Square(real32 A)
{
    real32 Result = A * A;

    return(Result);
}

/* STUDY: Can use dot product of a vector v with 2 orthogonal unit
   vectors to get v' which is v in terms of the new coordinate system
   defined by the 2 orthogonal unit vectors.

   vector dot unit axes = coordinates of vector along new axes
 */
inline real32
Inner(v2 A, v2 B)
{
    real32 Result = A.X * B.X + A.Y * B.Y;

    return(Result);
}

inline real32
LengthSq(v2 A)
{
    real32 Result = Inner(A, A);

    return(Result);
}

struct rectangle2
{
    v2 Min;
    v2 Max;
};

inline v2
GetMinCorner(rectangle2 Rect)
{
    v2 Result = Rect.Min;
    return Result;
}

inline v2
GetMaxCorner(rectangle2 Rect)
{
    v2 Result = Rect.Max;
    return Result;
}

inline v2
GetCenter(rectangle2 Rect)
{
    v2 Result = 0.5f * (Rect.Min + Rect.Max);
    return Result;
}

inline rectangle2
RectMinMax(v2 Min, v2 Max)
{
    rectangle2 Result;

    Result.Min = Min;
    Result.Max = Max;

    return Result;
}

inline rectangle2
RectCenterHalfDim(v2 Center, v2 HalfDim)
{
    rectangle2 Result;

    Result.Min = Center - HalfDim;
    Result.Max = Center + HalfDim;

    return Result;
}

inline rectangle2
RectCenterDim(v2 Center, v2 Dim)
{
    rectangle2 Result = RectCenterHalfDim(Center, 0.5f * Dim);

    return Result;
}

inline bool32
IsInRectangle(rectangle2 Rectangle, v2 Test)
{
    bool32 Result = (Test.X >= Rectangle.Min.X) &&
                    (Test.Y >= Rectangle.Min.Y) &&
                    (Test.X < Rectangle.Max.X) &&
                    (Test.Y < Rectangle.Max.Y);

    return Result;
}

#define HANDMADE_MATH_H
#endif
