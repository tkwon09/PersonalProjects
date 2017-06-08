#if !defined(HANDMADE_PLATFORM_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */
/*
  NOTE: Compiling this in C. These are all the structures and definitions the platform needs from the game.
  Prevents name mangling that C++ does, among other benefits of not using C++. Refer to GAME_GET_SOUND_SAMPLES
  in handmade.cpp for another example that prevents name mangling.
*/
/*
  STUDY: Basic set-up for the code
*/
/*
  NOTE:
  HANDMADE_INTERNAL:
      0 - Build for public
      1 - Build for developer
      
  HANDMADE_SLOW:
      0 - No slow code allowed
      1 - Slow code allowed
*/

#ifdef __cplusplus
extern "C" {
#endif
     
//
// NOTE: Compilers
//

#ifndef COMPILER_MSVC
#define COMPILER_MSVC 0
#endif

#ifndef COMPILER_LLVM
#define COMPILER_LLVM 0
#endif

#if !COMPILER_MSVC && !COMPILER_LLVM
#if _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else
// TODO: Moar compilerz!!!
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif
#endif

#if COMPILER_MSVC
#include <intrin.h>
#pragma intrinsic(_BitScanForward)
#endif
     
//
// NOTE: Types
//
     
#include <stdint.h>
#include <stddef.h>

/*
  STUDY: The below are explicit definitions for our integer and
  floating point values. Will need #if blocks for compiling on
  different platforms.
*/
    typedef int8_t int8;
    typedef int16_t int16;
    typedef int32_t int32;
    typedef int64_t int64;

    typedef int32 bool32;

    typedef uint8_t uint8;
    typedef uint16_t uint16;
    typedef uint32_t uint32;
    typedef uint64_t uint64;

    typedef size_t memory_index;

    typedef float real32;
    typedef double real64;

#define internal static // functions accessible only in same source
#define local_persist static // locally scoped variables <- bad
#define global_variable static // actual static variables
    
#define Pi32 3.14159265359f

    
#if HANDMADE_SLOW // lets compiler know to build with assertions. Not for performance builds.
// TODO: Complete assertion macro
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;} // Writes to the null pointer on failure
#else
#define Assert(Expression)
#endif

#define InvalidCodePath Assert(!"InvalidCodePath");
    
#define Kilobytes(Value) ((Value) * 1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)
#define Terabytes(Value) (Gigabytes(Value) * 1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
// TODO: swap, min, max ... macros???

    inline uint32
    SafeTruncateUInt64(uint64 Value)
    {
        // TODO: Defines for maximum values
        Assert(Value <= 0xFFFFFFFF);
        uint32 Result = (uint32) Value;
        return(Result);
    }

    
/*
  STUDY: It's important in multi-threaded code to know which thread is
  which. Some platforms don't do it for you.  So the game needs to
  keep track of it.
*/
    typedef struct thread_context
    {
        int Placeholder;
    } thread_context;         

/*
  NOTE: Services that the platform layer provides to the game
*/
#if HANDMADE_INTERNAL
/*
  IMPORTANT:
  
  These are NOT for doing anything in the shipping game - they are
  blocking and the write doesn't protect against lost data! For
  example, if a write fails then you could produce a corrupted save
  file.
*/
    typedef struct debug_read_file_result
    {
        uint32 ContentsSize;
        void *Contents;
    } debug_read_file_result;

// NOTE: Converted functions into pointer to functions. Remember, need to make them a type to make them a pointer.
#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(thread_context *Thread, void *Memory)
    typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(thread_context *Thread, char *Filename)
    typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) bool32 name(thread_context *Thread, char *Filename, uint32 MemorySize, void *Memory)
    typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);

#endif

/*
  NOTE: Services that the game provides to the platform layer
  (this may expand in the future - sound on separate thread, etc.)
*/
    
// FOUR THINGS - timing, controller/keyboard input, bitmap buffer to use, sound buffer to use

// TODO: In the future, rendering _specifically_ will become a three-tiered abstraction. Software/Hardware abstractions.
    typedef struct game_offscreen_buffer
    {
        // NOTE: Pixels are always 32-bits wide, Memory order: BB GG RR XX
        void *Memory;
        int Width;
        int Height;
        int Pitch;
        int BytesPerPixel;
    } game_offscreen_buffer;

    typedef struct game_sound_output_buffer
    {
        int SamplesPerSecond;
        int SampleCount;
        int16 *Samples;
    } game_sound_output_buffer;

    typedef struct game_button_state
    {
        int HalfTransitionCount;
        bool32 EndedDown;
    } game_button_state;

    typedef struct game_controller_input
    {
        bool32 IsConnected;
        bool32 IsAnalog;
        real32 StickAverageX;
        real32 StickAverageY;

        /*
          STUDY: Union lets us access each game_button_state by name or by index
          Also lets us to use the same memory for different variables.
         */
        union
        {
            game_button_state Buttons[12];
            struct
            {
                game_button_state MoveUp;
                game_button_state MoveDown;
                game_button_state MoveLeft;
                game_button_state MoveRight;
           
                game_button_state ActionUp;
                game_button_state ActionDown;
                game_button_state ActionLeft;
                game_button_state ActionRight;
           
                game_button_state LeftShoulder;
                game_button_state RightShoulder;

                game_button_state Start;
                game_button_state Back;

                // NOTE: All buttons must be added above this one
                game_button_state Terminator;
            };
        };
    } game_controller_input;

    typedef struct game_input
    {
        game_button_state MouseButtons[5];
        int32 MouseX, MouseY, MouseZ;
     
        real32 dtForFrame;
     
        game_controller_input Controllers[5];
    } game_input;

    // IMPORTANT: Casey doesn't keep this here. Where is it?????
    inline game_controller_input*
    GetController(game_input *Input, int ControllerIndex)
    {
        Assert(ControllerIndex < ArrayCount(Input->Controllers));
     
        game_controller_input *Result = &Input->Controllers[ControllerIndex];
        return Result;
    }
/*
  STUDY: Many modern games allocate and free memory as much as it
  wants. Each allocation introduces a failure point. Another approach
  is to define a fixed block of memory for our game. This is more
  reliable and requires less complex code to have it fail gracefully.

  Also, the allocate/free structure requires the platform to go to the
  game and then the game to go back to the platform and back to the
  game. The goal is to maintain one direction across the API as much
  as possible.
*/
    typedef struct game_memory
    {
        bool32 IsInitialized;
        uint64 PermanentStorageSize;
        void *PermanentStorage; // NOTE: REQUIRED to be cleared to zero at startup. VirtualAlloc does this for us.
        uint64 TransientStorageSize;
        void *TransientStorage; // NOTE: REQUIRED to be cleared to zero at startup. VirtualAlloc does this for us.

        debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory;
        debug_platform_read_entire_file *DEBUGPlatformReadEntireFile;
        debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;
    } game_memory;

// NOTE: Handle for platform code to call into handmade.h
#define GAME_UPDATE_AND_RENDER(name) void name(thread_context *Thread, game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer)
    typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

// NOTE: At the moment this has to be a very fast function. Can't be more than 1 ms or so.
// TODO: Reduce the pressure on this functions performance, measure or ask about it
// NOTE: Handle for platform code to call into handmade.h
#define GAME_GET_SOUND_SAMPLES(name) void name(thread_context *Thread, game_memory *Memory, game_sound_output_buffer *SoundBuffer)
    typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);

#ifdef __cplusplus
}
#endif
     
#define HANDMADE_PLATFORM_H
#endif
