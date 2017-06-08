/* ------------------------------------------------
   $File: win32_handmade.cpp
   $Date: 8/10/2015
   $Creator: Casey Muratori
   ------------------------------------------------ */
/*
  TODO:   THIS IS NOT A FINAL PLATFORM LAYER
  The purpose of this file is to give us a bootstrap for the Windows platform
  graphics buffer, sound buffer, simple input, timing
  THIS IS NOT A PLATFORM LAYER THAT WILL SHIP
  Our goals for this file is to encapsulate the things we need to do for Windows

  - Fullscreen support
  
  - Make the right calls so WIndows doesn't think we're still loading for a bit after we actuall start
  - Saved game location
  - Getting a handle to our own executable file
  - Asset loading path
  - Threading (launch a thread)
  - Raw Input (support for multiple keyboards)
  - ClipCursor() (for multimonitor support)
  - QueryCancelAutoplay
  - WM_ACTIVATEAPP (for when we are not the active window)
  - Blit speed improvements (BitBlt)
  - Hardware acceleration (OpenGL or Direct3D or BOTH?)
  - GetKeyboardLayout (for French keyboards, international WASD support)
  - ChangeDisplaySettings option if we detect slow fullscreen blit?

  x Sleep/timeBeginPeriod: don't use every single CPU cycle

  Just a partial list of stuff
 */
/*
  STUDY:
  Procrastination - Do not ever let the amount of
  programming you do drop to zero. Then you could procrastinate
  forever. Force yourself to program for 2 hours every day, even if
  what you write is useless and sucks. You will randomly hit on
  something that will let you move forward.
*/
#include "handmade_platform.h"

#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <xinput.h>
#include <dsound.h>

#include "win32_handmade.h"

/*
  STUDY
  Program in a way that prevents obviously bad things. Don't concentrate on
  optimizing on a low level unless you're actually doing an optimization pass.
  A - Compiler may be generating the same code anyway
  B - You don't know if one way is faster until you are actually testing it
*/

// TODO: Global for now
global_variable bool GlobalRunning;
global_variable bool GlobalPause;
global_variable win32_offscreen_buffer GlobalBackBuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;
global_variable int64 GlobalPerfCountFrequency;
global_variable bool32 DEBUGGlobalShowCursor;
global_variable WINDOWPLACEMENT GlobalWindowPosition = {sizeof(GlobalWindowPosition)};

/*
  STUDY
  Xinput libraries have stringent OS requirements. To avoid locking the game if they
  don't have these non-essential .dll files, we will load the functions at runtime.
*/
// NOTE: Using function pointers that point to default stub functions. Dynamically
// replaced with XInput functions if they are available.
/*
  STUDY
  For functions with potentially the same signature (avoid C++ name mangling)
  
  1) create a #define macro for the function signature. i.e. the name, the parameters
  2) typedef using the macro on a specific function name to let us create a pointer to the function
  3) use the macro to define a stub version of the function
  4) using the typedef and stub, create a pointer to the function

  for convenience, another macro lets us use the original function name without running the risk of causing a linker error.
  
  IMPORTANT: Purpose of metaprogramming is to remove the need for templates and avoid the associated baggage.
*/
// NOTE: XInputGetState
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE (XInputGetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// NOTE: XInputSetState
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE (XInputSetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal void
CatStrings(size_t SourceACount, char *SourceA, size_t SourceBCount, char *SourceB, size_t DestCount, char *Dest)
{
    // TODO: Destination buffer bounds checking!
    for (int Index = 0; Index < SourceACount; ++Index)
    {
        *Dest++ = *SourceA++;
    }
     
    for (int Index = 0; Index < SourceBCount; ++Index)
    {
        *Dest++ = *SourceB++;
    }
     
    *Dest++ = 0;
}

internal void
Win32GetEXEFileName(win32_state *State)
{
    // NOTE: Never use MAX_PATH in code that is user-facing because it can be dangerous and lead to bad results
    // Vstudio runs in the data file but we need to search for the game code in the same directory as the .exe
    DWORD SizeOfFilename = GetModuleFileNameA(0, State->EXEFileName, sizeof(State->EXEFileName));
    State->OnePastLastEXEFileNameSlash = State->EXEFileName;
    for (char *Scan = State->EXEFileName; *Scan; ++Scan)
    {
        if (*Scan == '\\')
        {
            State->OnePastLastEXEFileNameSlash = Scan + 1;
        }
    }
}

// Automatically excludes null terminator
internal int
StringLength(char *String)
{
    int Count = 0;
    while (*String++)
    {
        ++Count;
    }
    return Count;
}

internal void
Win32BuildEXEPathFileName(win32_state *State, char *FileName, int DestCount, char *Dest)
{
    CatStrings(State->OnePastLastEXEFileNameSlash - State->EXEFileName, State->EXEFileName, StringLength(FileName), FileName, DestCount, Dest);
}

// NOTE: The below are macros that let us set these to the value of function pointers we will use to dynamically load.
// The macros are useful so that the same parameter list and definitions don't appear in multiple places.
DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory)
{
    if (Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile)
{
    debug_read_file_result Result = {};
     
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if (GetFileSizeEx(FileHandle, &FileSize))
        {
            uint32 FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
            Result.Contents = VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if (Result.Contents)
            {
                // If we really want to read large file, we need a loop since ReadFile doesn't let
                // us read 64-bit values. Just takes a DWORD. Also need to check if there was a partial read.
                DWORD BytesRead;
                if (ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) && (FileSize32 == BytesRead))
                {
                    // NOTE: File read successfully
                    Result.ContentsSize = FileSize32;
                }
                else
                {
                    // If read fails then just free immediately
                    // TODO: Logging
                    DEBUGPlatformFreeFileMemory(Thread, Result.Contents);
                    Result.Contents = 0;
                }
            }
            else
            {
                // TODO: Logging
            }
        }
        else
        {
            // TODO: Logging
        }

        CloseHandle(FileHandle);
    }
    else
    {
        // TODO: Logging
    }
    return Result;
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile)
{
    bool32 Result = false;
     
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        // If we really want to read large file, we need a loop since ReadFile doesn't let
        // us read 64-bit values. Just takes a DWORD. Also need to check if there was a partial read.
        DWORD BytesWritten;
        if (WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0))
        {
            // NOTE: File written successfully
            Result = (BytesWritten == MemorySize);
        }
        else
        {
            // TODO: Logging
        }

        CloseHandle(FileHandle);
    }
    else
    {
        // TODO: Logging
    }
    return Result;
}

/*
  Function to dynamically load our game code
*/
inline FILETIME
Win32GetLastWriteTime(char *Filename)
{
    FILETIME LastWriteTime = {};
     
    WIN32_FILE_ATTRIBUTE_DATA Data;
    if (GetFileAttributesEx(Filename, GetFileExInfoStandard, &Data))
    {
        LastWriteTime = Data.ftLastWriteTime;
    }

    return LastWriteTime;
}

internal win32_game_code
Win32LoadGameCode(char *SourceDLLName, char *TempDLLName, char *LockFileName)
{
    win32_game_code Result = {};

    // NOTE: Don't lock handmade.exe while we're running. Lets us edit it while game is running.
    // NOTE: Check to see if the lock file is there. Indicates that compiler is still generating .dll and .pdb
    WIN32_FILE_ATTRIBUTE_DATA Ignored;
    if (!GetFileAttributesEx(LockFileName, GetFileExInfoStandard, &Ignored))
    {
        Result.DLLLastWriteTime = Win32GetLastWriteTime(SourceDLLName);
     
        CopyFile(SourceDLLName, TempDLLName, FALSE);
        Result.GameCodeDLL = LoadLibraryA(TempDLLName);
        if (Result.GameCodeDLL)
        {
            Result.UpdateAndRender = (game_update_and_render *) GetProcAddress(Result.GameCodeDLL, "GameUpdateAndRender");
            Result.GetSoundSamples = (game_get_sound_samples *) GetProcAddress(Result.GameCodeDLL, "GameGetSoundSamples");
            Result.IsValid = (Result.UpdateAndRender && Result.GetSoundSamples);
        }
    }

    if (!Result.IsValid)
    {
        Result.UpdateAndRender = 0;
        Result.GetSoundSamples = 0;
    }
     
    return Result;
}

internal void
Win32UnloadGameCode(win32_game_code *GameCode)
{
    if (GameCode->GameCodeDLL)
    {
        FreeLibrary(GameCode->GameCodeDLL);
        GameCode->GameCodeDLL = 0;
    }

    GameCode->IsValid = false;
    GameCode->UpdateAndRender = 0;
    GameCode->GetSoundSamples = 0;
}

/*
  Function to dynamically load XInput.
*/
internal void
Win32LoadXInput ()
{
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if (!XInputLibrary)
    {
        // TODO: Diagnostic
        XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
    }
    else if (!XInputLibrary)
    {
        // TODO: Diagnostic
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }
    else
    {
        // TODO: Diagnostic
        XInputGetState = (x_input_get_state *) GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state *) GetProcAddress(XInputLibrary, "XInputSetState");
    }
}

/*
  Works by creating 2 sample buffers. We buffer samples into the secondary buffer. Primary buffer simply
  gets a handle on the sound card. It just sets it to a mode that we want to play sound in.
*/
internal void
Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize)
{
    // NOTE: Load the library. Similar to loading XInput
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");

    if (DSoundLibrary)
    {
        // NOTE: Get a DirectSound object
        direct_sound_create *DirectSoundCreate = (direct_sound_create *) GetProcAddress(DSoundLibrary, "DirectSoundCreate");
        // Object is return in the pointer. SUCCEEDED checks the return code for us. Part of DirectSound
        /*
          When we loaded dsound.dll we implicitly loaded a vtable. When we get the handle to DirectSound it
          has a bunch of virtual function pointers. The compiler takes those and replaces them with a pointer
          to the vtable for DirectSound and then looks up the appropriate function pointer for whatever function
          we called. This is double indirection and requires 2 memory lookups. It's not a costless
          abstraction. Note the implications this has for virtual functions in C++.
        */
        LPDIRECTSOUND DirectSound;
        if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
        {
            // Set the sound format
            WAVEFORMATEX WaveFormat = {};
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = SamplesPerSecond;
            WaveFormat.wBitsPerSample = 16; // 16-bit audio
            WaveFormat.nBlockAlign = (WaveFormat.nChannels*WaveFormat.wBitsPerSample) / 8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec*WaveFormat.nBlockAlign;
            WaveFormat.cbSize = 0;

            // Attach instance to window
            if (SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

                // NOTE: "Create" a primary buffer
                // TODO: DSBCAPS_GLOBALFOCUS?
                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
                {
                    HRESULT Error = PrimaryBuffer->SetFormat(&WaveFormat);
                    if (SUCCEEDED(Error))
                    {
                        OutputDebugStringA("Primary buffer format was set.\n");
                    }
                    else
                    {
                        // TODO: Diagnostic
                    }
                }
                else
                {
                    // TODO: Diagnostic
                }
            }
            else
            {
                // TODO: Diagnostic
            }

            // TODO: DSBCAPS_GETCURRENTPOSITION2
            // This is the buffer we can actually write into
            DSBUFFERDESC BufferDescription = {};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
            BufferDescription.dwBufferBytes = BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;
            HRESULT Error = DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0);
            if (SUCCEEDED(Error))
            {
                OutputDebugStringA("Secondary buffer was created successfully.\n");
            }
            else
            {
                // TODO: Diagnostic
            }
        }
        else
        {
            // TODO: Diagnostic
        }
    }
}

internal win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;

    RECT  ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return Result;
}

/*
  Win32 prefix is for functions on the platform layer.
*/
internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
    // TODO: Bulletproof this.
    // Maybe don't free first, free after. Then free first if fails

    if (Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE); // size automatically tracked
        /*
          One way to prevent using pointers after freeing them is to use
          VirtualProtect() instead of VirtualFree().
        */
    }

    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4;
     
    // not using variable size color reference table
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height; // top-down blit
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32; // RBGA and DWORD-aligned (x86)
    Buffer->Info.bmiHeader.biCompression = BI_RGB; // uncompressed
     
    /*
      NOTE: Chris Hecker (Spy Party) says we don't need DC with
      StretchDIBits vs BitBlt
    */
    int BitmapMemorySize = Buffer->Width * Buffer->Height * Buffer->BytesPerPixel;
     
    /*
      Will use VirtualAlloc. Allocates a certain number of pages.
      HeapAlloc: variable amount, OS handles paging
      VirtualAlloc: returns fixed number of pages. We use 64-bit address space.
    */
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    Buffer->Pitch = Width * Buffer->BytesPerPixel;
}

/*
  Draws a bitmap buffer to the window buffer. Not using BITBLT because that entails
  creating bitmaps and device contexts. With this we just pass a pointer.
*/
internal void
Win32DisplayBufferInWindow(win32_offscreen_buffer *Buffer, HDC DeviceContext, int WindowWidth, int WindowHeight)
{
    // TODO: Centering / black bars?
    if ((WindowWidth >= Buffer->Width*2) &&
        (WindowHeight >= Buffer->Height*2))
    {
        StretchDIBits(DeviceContext,
                      0, 0, 2*Buffer->Width, 2*Buffer->Height,
                      0, 0, Buffer->Width, Buffer->Height,
                      Buffer->Memory,
                      &Buffer->Info,
                      DIB_RGB_COLORS, SRCCOPY);
    }
    else
    {
        // Clear buffer to black if not being drawn on
        int OffsetX = 10;
        int OffsetY = 10;
     
        PatBlt(DeviceContext, 0, 0, WindowWidth, OffsetY, BLACKNESS);
        PatBlt(DeviceContext, 0, OffsetY + Buffer->Height, WindowWidth, WindowHeight, BLACKNESS);
        PatBlt(DeviceContext, 0, 0, OffsetX, WindowHeight, BLACKNESS);
        PatBlt(DeviceContext, OffsetX + Buffer->Width, 0, WindowWidth, WindowHeight, BLACKNESS);
     
        // NOTE: For prototyping purposes, we're going to always blit 1-to-1 pixels to make sure we don't
        // introduce stretching artifacts while learning to code the renderer
        StretchDIBits(DeviceContext,
                      OffsetX, OffsetY, Buffer->Width, Buffer->Height,
                      0, 0, Buffer->Width, Buffer->Height,
                      Buffer->Memory,
                      &Buffer->Info,
                      DIB_RGB_COLORS, SRCCOPY);
    }
}

/*
  Callback function handles messages from Windows to our application.
  Required by WNDCLASS.
*/
internal LRESULT CALLBACK
Win32MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;
    switch(Message)
    {
        case WM_CLOSE:
        {
            // TODO: handle this with a message to user?
            GlobalRunning = false;
        } break;
        case WM_SETCURSOR:
        {
            if (DEBUGGlobalShowCursor)
            {
                Result = DefWindowProcA(Window, Message, WParam, LParam);
            }
            else
            {
                SetCursor(0);
            }
        } break;
        case WM_ACTIVATEAPP:
        {
#if 0
            if (WParam == TRUE)
            {
                SetLayeredWindowAttributes(Window, RGB(0, 0, 0), 255, LWA_ALPHA);
            }
            else
            {
                SetLayeredWindowAttributes(Window, RGB(0, 0, 0), 64, LWA_ALPHA);
            }
#endif
        } break;
        case WM_SIZE:
        {
        } break;
        case WM_DESTROY:
        {
            // TODO: Handlge this as an error - recreate window?
            GlobalRunning = false;
        } break;

        // NOTE: We directly handle all keyboard inputs
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            Assert(!"Keyboard input came in through a non-dispatch message!");
      
        } break;
        case WM_PAINT:
        {
            // Using Windows draw functions
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint); // get buffer
            win32_window_dimension Dimension = Win32GetWindowDimension(Window);
            Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height);
      
            EndPaint(Window, &Paint);
        } break;
        default:
        {
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        } break;
    }
    return Result;
}

/* STUDY: How does the circular buffer work?
   case 1: <LOCK> > <PLAY>
   |xxxx<PLAY>------<WRITE>-----<LOCK>xxxx|

   case 2: <LOCK> < <PLAY>
   |-------<LOCK>xxxxxxxxxx<PLAY>---------|

   x = write region
   - = playing/already written to
*/
// int16  int16  int16  int16  ...
// [LEFT  RIGHT] [LEFT  RIGHT] ...
internal void
Win32ClearBuffer(win32_sound_output *SoundOutput)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
     
    if (SUCCEEDED(GlobalSecondaryBuffer->Lock(0, SoundOutput->SecondaryBufferSize, &Region1, &Region1Size, &Region2, &Region2Size, 0)))
    {
        uint8 *DestSample = (uint8 *) Region1;
        for (DWORD ByteIndex = 0; ByteIndex < Region1Size; ++ByteIndex)
        {
            *DestSample++ = 0;
        }
        DestSample = (uint8 *) Region2;
        for (DWORD ByteIndex = 0; ByteIndex < Region2Size; ++ByteIndex)
        {
            *DestSample++ = 0;
        }
        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

/*
  Sound is inherently synchronous while video is not necessarily so. A delayed frame will only drop the frame rate but a delayed sample will sound buggy.
*/
void
Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD ByteToLock, DWORD BytesToWrite, game_sound_output_buffer *SourceBuffer)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
             
    if (SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite, &Region1, &Region1Size, &Region2, &Region2Size, 0)))
    {
        DWORD Region1SampleCount = Region1Size/SoundOutput->BytesPerSample;
        int16 *DestSample = (int16 *) Region1;
        int16 *SourceSample = SourceBuffer->Samples;
        for (DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex)
        {
            // Copy from source buffer to output buffer
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }

        DWORD Region2SampleCount = Region2Size/SoundOutput->BytesPerSample;
        DestSample = (int16 *) Region2;
        for (DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }

        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

internal void
Win32ProcessKeyboardMessage(game_button_state *NewState, bool32 IsDown)
{
    // We are not processing keyboard at Windows' frequency
    // That would mess up our half-transition implementation
    // Only process when a difference happens
    if (NewState->EndedDown != IsDown)
    {
        NewState->EndedDown = IsDown;
        ++NewState->HalfTransitionCount;
    }
}

internal void
Win32ProcessXInputDigitalButton(DWORD XInputButtonState, game_button_state *OldState, DWORD ButtonBit, game_button_state *NewState)
{
    NewState->EndedDown = ((XInputButtonState & ButtonBit) == ButtonBit);
    NewState->HalfTransitionCount = (OldState->EndedDown == NewState->EndedDown)? 1 : 0; // Sampling just once per frame
}

internal real32
Win32ProcessXInputStickValue(SHORT Value, SHORT DeadZoneThreshold)
{
    real32 Result = 0;

    // NOTE : Deadzone is handled square on purpose
    if (Value < -DeadZoneThreshold)
    {
        Result = (real32) (Value + DeadZoneThreshold) / (32768.0f - DeadZoneThreshold);
    }
    else if (Value > DeadZoneThreshold)
    {
        Result = (real32) (Value - DeadZoneThreshold) / (32767.0f - DeadZoneThreshold);
    }
     
    return Result;
}

internal void
Win32GetInputFileLocation(win32_state *State, bool32 InputStream, int SlotIndex, int DestCount, char *Dest)
{
    char Temp[64];
    wsprintf(Temp, "loop_edit_%d_%s.hmi", SlotIndex, InputStream ? "input" : "state");
    Win32BuildEXEPathFileName(State, Temp, DestCount, Dest);
}

internal win32_replay_buffer*
Win32GetReplayBuffer(win32_state *State, int unsigned Index)
{
    Assert(Index > 0);
    Assert(Index < ArrayCount(State->ReplayBuffers));
    win32_replay_buffer *Result = &State->ReplayBuffers[Index];
    return Result;
}

// NOTE: Below are function to support looped live code editing
internal void
Win32BeginRecordingInput(win32_state *State, int InputRecordingIndex)
{
    win32_replay_buffer *ReplayBuffer = Win32GetReplayBuffer(State, InputRecordingIndex);
    if (ReplayBuffer->MemoryBlock)
    {
        State->InputRecordingIndex = InputRecordingIndex;

        char FileName[WIN32_STATE_FILE_NAME_COUNT];
        Win32GetInputFileLocation(State, true, InputRecordingIndex, sizeof(FileName), FileName);
        State->RecordingHandle = CreateFileA(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

#if 0
        // NOTE: The file position is pointing to where last written to, need to recalculate to right spot
        LARGE_INTEGER FilePosition;
        FilePosition.QuadPart = State->TotalSize;
        SetFilePointerEx(State->RecordingHandle, FilePosition, 0, FILE_BEGIN);
#endif
      
        CopyMemory(ReplayBuffer->MemoryBlock, State->GameMemoryBlock, State->TotalSize);
    }
}

internal void
Win32EndRecordingInput(win32_state *State)
{
    CloseHandle(State->RecordingHandle);
    State->InputRecordingIndex = 0;
}

internal void
Win32BeginInputPlayBack(win32_state *State, int InputPlayingIndex)
{
    win32_replay_buffer *ReplayBuffer = Win32GetReplayBuffer(State, InputPlayingIndex);
    if (ReplayBuffer->MemoryBlock)
    {
        State->InputPlayingIndex = InputPlayingIndex;
      
        char FileName[WIN32_STATE_FILE_NAME_COUNT];
        Win32GetInputFileLocation(State, true, InputPlayingIndex, sizeof(FileName), FileName);
        State->PlaybackHandle = CreateFileA(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

#if 0
        // NOTE: The file position is pointing to where last read from, need to recalculate to right spot
        LARGE_INTEGER FilePosition;
        FilePosition.QuadPart = State->TotalSize;
        SetFilePointerEx(State->PlaybackHandle, FilePosition, 0, FILE_BEGIN);
#endif
      
        CopyMemory(State->GameMemoryBlock, ReplayBuffer->MemoryBlock, State->TotalSize);
    }
}

internal void
Win32EndInputPlayBack(win32_state *State)
{
    CloseHandle(State->PlaybackHandle);
    State->InputPlayingIndex = 0;
}

internal void
Win32RecordInput(win32_state *State, game_input *NewInput)
{
    DWORD BytesWritten;
    WriteFile(State->RecordingHandle, NewInput, sizeof(*NewInput), &BytesWritten, 0);
}

internal void
Win32PlayBackInput(win32_state *State, game_input *NewInput)
{
    DWORD BytesRead = 0;
    if (ReadFile(State->PlaybackHandle, NewInput, sizeof(*NewInput), &BytesRead, 0))
    {
        if (BytesRead == 0)
        {
            // NOTE: We've hit the end of the stream, go back to the beginning
            int PlayingIndex = State->InputPlayingIndex;
            Win32EndInputPlayBack(State);
            Win32BeginInputPlayBack(State, PlayingIndex);
            ReadFile(State->PlaybackHandle, NewInput, sizeof(*NewInput), &BytesRead, 0);
        }
    }
}

internal void
ToggleFullScreen(HWND Window)
{
    // NOTE: This follows Raymond Chen's prescription for fullscreen toggling
    // for fullscreen toggling see:
    // http://blogs.msdn.com/b/oldnewthing/archive/2010/04/12/9994016.aspx
    DWORD Style = GetWindowLong(Window, GWL_STYLE);
    if (Style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
        if (GetWindowPlacement(Window, &GlobalWindowPosition) &&
            GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
        {
            SetWindowLong(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(Window, HWND_TOP,
                         MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                         MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                         MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(Window, &GlobalWindowPosition);
        SetWindowPos(Window, 0, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

/* STUDY:
   
   What is a functional function? A functional function produces an output from its inputs with no side effects.
   A function with side effects (which is more typical of C style stuff) takes input and mutates them when called.
   Recall passing in a reference and getting the result back in the reference's object. The actual data state changes.

   Functional functions have nice properties, like being commutative. Increases understandability, removes burden of
   keeping track of the data state of the program from the programmer.
*/
/*
  Many people implement input detection as a queue of events. This is a variable-size solution. A more compact way is to encode data in variables.
*/
internal void
Win32ProcessPendingMessages(win32_state *State, game_controller_input *KeyboardController)
{
    // Windows puts messages in a queue
    MSG Message;
    while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch (Message.message) // WM_QUIT = 0, error < 0
        {
            case WM_QUIT:
            {
                GlobalRunning = false;
            } break;
      
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                uint32 VKCode = (uint32) Message.wParam;

                // NOTE: Since we are compaing WasDown to IsDown we MUST use == and != to convert bits to actual booleans
                bool32 WasDown = ((Message.lParam & (1 << 30)) != 0); // masks bit to check if was down
                bool32 IsDown = ((Message.lParam & (1 << 31)) == 0); // masks bit to check if is down
                if (WasDown != IsDown)
                {
                    if (VKCode == 'W')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveUp, IsDown);
                    }
                    else if (VKCode == 'A')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveLeft, IsDown);
                    }
                    else if (VKCode == 'S')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveDown, IsDown);
                    }
                    else if (VKCode == 'D')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveRight, IsDown);
                    }
                    else if (VKCode == 'Q')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->LeftShoulder, IsDown); 
                    }
                    else if (VKCode == 'E')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->RightShoulder, IsDown); 
                    }
                    else if (VKCode == VK_UP)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionUp, IsDown); 
                    }
                    else if (VKCode == VK_LEFT)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionLeft, IsDown); 
                    }
                    else if (VKCode == VK_DOWN)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionDown, IsDown); 
                    }
                    else if (VKCode == VK_RIGHT)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionRight, IsDown); 
                    }
                    else if (VKCode == VK_ESCAPE)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->Back, IsDown); 
                    }
                    else if (VKCode == VK_SPACE)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->Start, IsDown); 
                    }
#if HANDMADE_INTERNAL
                    else if (VKCode == 'P')
                    {
                        if (IsDown)
                        {
                            GlobalPause = !GlobalPause;
                        }
                    }
                    // NOTE: Save input sequences for later use
                    else if (VKCode == 'L')
                    {
                        if (IsDown)
                        {
                            // IMPORTANT: Save SSD, don't do it!
                            if (State->InputRecordingIndex == 0)
                            {
                                if (State->InputRecordingIndex == 0)
                                {
                                    //Win32BeginRecordingInput(State, 1);
                                }
                                else
                                {
                                    //Win32EndRecordingInput(State);
                                    //Win32BeginInputPlayBack(State, 1);
                                }
                            }
                            else
                            {
                                //Win32EndInputPlayback(State);
                            }
                        }
                    }
#endif
                    if (IsDown)
                    {
                        bool32 AltKeyWasDown = (Message.lParam & (1 << 29));
                        if ((VKCode == VK_F4) && AltKeyWasDown)
                        {
                            GlobalRunning = false;
                        }
                        if ((VKCode == VK_RETURN) && AltKeyWasDown)
                        {
                            if (Message.hwnd)
                            {
                                ToggleFullScreen(Message.hwnd);
                            }
                        }
                    }
                }
            } break;
            default:
            {
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
            } break;
        }
    }
}

inline LARGE_INTEGER
Win32GetWallClock()
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return Result;
}

inline real32
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{            
    real32 Result = ((real32) (End.QuadPart - Start.QuadPart) / (real32) GlobalPerfCountFrequency);
    return Result;
}

#if 0
internal void
Win32DebugDrawVertical(win32_offscreen_buffer *Backbuffer, int X, int Top, int Bottom, uint32 Color)
{
    if (Top < 0)
    {
        Top = 0;
    }
    if (Bottom > Backbuffer->Height)
    {
        Bottom = Backbuffer->Height;
    }
    if (X >= 0 && X < Backbuffer->Width)
    {
        uint8 *Pixel = (uint8 *) Backbuffer->Memory + X*Backbuffer->BytesPerPixel + Top*Backbuffer->Pitch;
        for (int Y = Top; Y < Bottom; ++Y)
        {
            *(uint32 *) Pixel = Color;
            Pixel += Backbuffer->Pitch;
        }
    }
}

inline void
Win32DrawSoundBufferMarker(win32_offscreen_buffer *Backbuffer, win32_sound_output *SoundOutput, real32 C, int PadX, int Top, int Bottom, DWORD Value, uint32 Color)
{
    real32 XReal32 = (C * (real32) Value);
    int X = PadX + (int) XReal32;
    Win32DebugDrawVertical(Backbuffer, X, Top, Bottom, Color);
}


internal void Win32DebugSyncDisplay(win32_offscreen_buffer *Backbuffer, int MarkerCount, win32_debug_time_marker *Markers, int CurrentMarker, win32_sound_output *SoundOutput, real32 TargetSecondsPerFrame)
{
    int PadX = 16;
    int PadY = 16;

    int LineHeight = 64;
     
    // scale the number of bytes in the buffer to the width of the screen buffer
    real32 C = (real32) (Backbuffer->Width - 2 * PadX) / (real32) SoundOutput->SecondaryBufferSize;
    for (int MarkerIndex = 0; MarkerIndex < MarkerCount; ++MarkerIndex)
    {
        win32_debug_time_marker *ThisMarker = &Markers[MarkerIndex];
        Assert(ThisMarker->OutputPlayCursor < SoundOutput->SecondaryBufferSize);
        Assert(ThisMarker->OutputWriteCursor < SoundOutput->SecondaryBufferSize);
        Assert(ThisMarker->OutputLocation < SoundOutput->SecondaryBufferSize);
        Assert(ThisMarker->OutputByteCount < SoundOutput->SecondaryBufferSize);
        Assert(ThisMarker->FlipPlayCursor < SoundOutput->SecondaryBufferSize);
        Assert(ThisMarker->FlipWriteCursor < SoundOutput->SecondaryBufferSize);
      
        DWORD PlayColor = 0xFFFFFFFF;
        DWORD WriteColor = 0xFFFF0000;
        DWORD ExpectedFlipColor = 0xFFFFFF00;
        DWORD PlayWindowColor = 0xFFFF00FF;

        int Top = PadY;
        int Bottom = PadY + LineHeight;
        if (MarkerIndex == CurrentMarker)
        {
            Top += LineHeight + PadY;
            Bottom += LineHeight + PadY;

            int FirstTop = Top;
           
            Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputPlayCursor, PlayColor);
            Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputWriteCursor, WriteColor);
           
            Top += LineHeight + PadY;
            Bottom += LineHeight + PadY;
           
            Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputLocation, PlayColor);
            Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputLocation + ThisMarker->OutputByteCount, WriteColor);

            Top += LineHeight + PadY;
            Bottom += LineHeight + PadY;

            Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, FirstTop, Bottom, ThisMarker->ExpectedFlipPlayCursor, ExpectedFlipColor);
        }
      
        Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->FlipPlayCursor, PlayColor);
        Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->FlipPlayCursor + 480 * SoundOutput->BytesPerSample, PlayWindowColor);
        Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->FlipWriteCursor, WriteColor);
    }
}
#endif

int CALLBACK
WinMain(HINSTANCE Instance, HINSTANCE prevInstance, LPSTR CommandLine, int ShowCode)
{
    // Create game tracking struct
    win32_state Win32State = {};

    // Grab the frequency of Windows counter
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    GlobalPerfCountFrequency = PerfCountFrequencyResult.QuadPart;

    // Generate path for all files game may generate
    Win32GetEXEFileName(&Win32State);
     
    char SourceGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "handmade.dll", sizeof(SourceGameCodeDLLFullPath), SourceGameCodeDLLFullPath);

    char TempGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "handmade_temp.dll", sizeof(TempGameCodeDLLFullPath), TempGameCodeDLLFullPath);

    char GameCodeLockFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "lock.tmp", sizeof(GameCodeLockFullPath), GameCodeLockFullPath);
     
    // NOTE: Set the Windows scheduler granularity to 1ms so that our Sleep() can be more granular
    UINT DesiredSchedulerMS = 1;
    bool32 SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);
     
    Win32LoadXInput();

#if HANDMADE_INTERNAL
    DEBUGGlobalShowCursor = true;
#endif
    WNDCLASSA WindowClass = {};

    // NOTE: 1080p display mode is 1920 x 1080 -> Half of that is 960 x 540
    /*
      STUDY: GPU is optimized for texture resolutions that are a power of 2.
      1920 -> 2048 = 2048 - 1920 -> 128 pixels wasted
      1080 -> 2048 = 2048 - 1080 -> 968 pixels wasted

      Solution: 2 chunks
      1024 + 128 = 1152 should give us a reasonable border
      1152 / 2 = 576

      2048 / 2 = 1024
    */
    Win32ResizeDIBSection(&GlobalBackBuffer, 960, 540);
    
    WindowClass.style = CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback; // not pointer because that would be in the virtual address space
    WindowClass.hInstance = Instance;
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
    // WindowClass.hIcon;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";
     
    // Used in Windows to register strings in an atom table
    if (RegisterClassA(&WindowClass))
    {
        HWND Window = CreateWindowExA(
            0, //WS_EX_TOPMOST|WS_EX_LAYERED,
            WindowClass.lpszClassName,
            "Handmade Hero",
            WS_OVERLAPPEDWINDOW|WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            Instance,
            0);
        if (Window)
        {
            // TODO: How do you reliably query this on Windows?
            int MonitorRefreshHz = 60;
            HDC RefreshDC = GetDC(Window);
            int Win32RefreshRate = GetDeviceCaps(RefreshDC, VREFRESH);
            ReleaseDC(Window, RefreshDC);
            if (Win32RefreshRate > 1)
            {
                MonitorRefreshHz = Win32RefreshRate;
            }
            real32 GameUpdateHz = MonitorRefreshHz / 2.0f;
            real32 TargetSecondsPerFrame = 1.0f / GameUpdateHz;
     
            win32_sound_output SoundOutput = {};
     
            SoundOutput.SamplesPerSecond = 48000; // 48kHz
            SoundOutput.BytesPerSample = sizeof(int16)*2; // LEFT RIGHT pair, each 16 bits
            SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond*SoundOutput.BytesPerSample;
            // TODO: Actually compute this variance and see what lowest reasonable value is
            SoundOutput.SafetyBytes = (int) ((real32) SoundOutput.SamplesPerSecond * (real32) SoundOutput.BytesPerSample / (GameUpdateHz * 3.0f));
            Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
            Win32ClearBuffer(&SoundOutput);
           
            GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
           
            GlobalRunning = true;

#if 0
            // NOTE: This tests the PlayCursor/WriteCursor update frequency
            while(GlobalRunning)
            {
                DWORD PlayCursor;
                DWORD WriteCursor;
                GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor);

                char TextBuffer[256];
                _snprintf_s(TextBuffer, sizeof(TextBuffer), "PC:%u WC:%u\n", PlayCursor, WriteCursor);
                OutputDebugStringA(TextBuffer);
            }
#endif

            // Reserve enough memory for our sound output
            /*
              Later we will pool all virtual allocations enough for everything we need
            */
            int16 *Samples = (int16 *) VirtualAlloc(0, SoundOutput.SecondaryBufferSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

#if HANDMADE_INTERNAL // going to try to set internal base address
            LPVOID BaseAddress = (LPVOID) Terabytes((uint64) 2);
#else
            LPVOID BaseAddress = 0;
#endif
            game_memory GameMemory = {};
            GameMemory.PermanentStorageSize = Megabytes(64);
            GameMemory.TransientStorageSize = Gigabytes(1); // C will compute the macros using 32-bit integers...
            GameMemory.DEBUGPlatformFreeFileMemory = DEBUGPlatformFreeFileMemory;
            GameMemory.DEBUGPlatformReadEntireFile = DEBUGPlatformReadEntireFile;
            GameMemory.DEBUGPlatformWriteEntireFile = DEBUGPlatformWriteEntireFile;
           
            // TODO: Handle various memory footprints (USING SYSTEM METRICS)
            //
            // TODO: Use MEM_LARGE_PAGES and call adjust token priviledges when not on Windows XP?
            //
            // TODO: TransientStorage needs to be broken up into game transient and cache transient
            // only the former needs be saved for state playback
            Win32State.TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
            // Using size_t so it can build correctly on x86 and x64
            Win32State.GameMemoryBlock = VirtualAlloc(BaseAddress,
                                                      (size_t) Win32State.TotalSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            GameMemory.PermanentStorage = Win32State.GameMemoryBlock;
            GameMemory.TransientStorage = ((uint8 *) GameMemory.PermanentStorage + GameMemory.PermanentStorageSize);

            /* IMPORTANT: Not doing this, save my SSD 
               for (int ReplayIndex = 1; ReplayIndex < ArrayCount(Win32State.ReplayBuffers); ++ReplayIndex)
               {
               win32_replay_buffer *ReplayBuffer = &Win32State.ReplayBuffers[ReplayIndex];

               // TODO: Recording system still seems to take too long on record start - find out what Windows
               // is doing and see if we can speed up / defer some of that processing
               Win32GetInputFileLocation(&Win32State, false, ReplayIndex, sizeof(ReplayBuffer->FileName), ReplayBuffer->FileName);
     
               ReplayBuffer->FileHandle = CreateFileA(ReplayBuffer->FileName, GENERIC_WRITE|GENERIC_READ, 0, 0, CREATE_ALWAYS, 0, 0);
            
               // NOTE: Asking Windows to set the file as a mapping to memory
               DWORD MaxSizeHigh = Win32State.TotalSize >> 32;
               DWORD MaxSizeLow = Win32State.TotalSize & 0xFFFFFFFF;
               ReplayBuffer->MemoryMap = CreateFileMapping(ReplayBuffer->FileHandle, 0, PAGE_READWRITE, MaxSizeHigh, MaxSizeLow, 0);
               ReplayBuffer->MemoryBlock = MapViewOfFile(ReplayBuffer->MemoryMap, FILE_MAP_ALL_ACCESS, 0, 0, Win32State.TotalSize);
               if (ReplayBuffer->MemoryBlock)
               {
             
               }
               else
               {
               // TODO: Diagnostic
               }
               }
            */
           
            // Check if we got the memory we requested
            if (Samples && GameMemory.PermanentStorage && GameMemory.TransientStorage)
            {
                game_input Input[2] = {};
                game_input *NewInput = &Input[0];
                game_input *OldInput = &Input[1];

                LARGE_INTEGER LastCounter = Win32GetWallClock();
                LARGE_INTEGER FlipWallClock = Win32GetWallClock();;
            
                int DebugTimeMarkerIndex = 0;
                win32_debug_time_marker DebugTimeMarkers[30] = {0};

                DWORD AudioLatencyBytes = 0;
                real32 AudioLatencySeconds = 0;
                bool32 SoundIsValid = false;

                // NOTE: load game code here
                win32_game_code Game = Win32LoadGameCode(SourceGameCodeDLLFullPath, TempGameCodeDLLFullPath, GameCodeLockFullPath);
            
                // CPU speed independent Windows timer function
                // generates rdtsc instruction, returns processor cycle count since last call
                /*NOTE: rdtsc is very specific for CPU profiling. Can't use it for timing that runs on user machine.
                 */
                /*
                  1. Gather input
                  2. Update render prep
                  3. Rendering
                  4. Wait
                  5. Flip
                  Can be pipelined or multithreaded
                  If we overlap the input and update render prep with the rendering of the previous frame then we can have sycned audio, but then we introduced input lag. Tradeoff between input lag and audio lag.
                  Since this game doesn't need to sync audio to frames, we will decouple the audio from the fdrame boundaries and simply accept a smaller, unsynced latency.
                */
                uint64 LastCycleCount = __rdtsc();
                while (GlobalRunning)
                {
                    // Set elapsed time for input
                    NewInput->dtForFrame = TargetSecondsPerFrame;
             
                    // NOTE: Reload game files if needed
                    FILETIME NewDLLWriteTime = Win32GetLastWriteTime(SourceGameCodeDLLFullPath);
                    if (CompareFileTime(&NewDLLWriteTime, &Game.DLLLastWriteTime))
                    {
                        Win32UnloadGameCode(&Game);
                        Game = Win32LoadGameCode(SourceGameCodeDLLFullPath, TempGameCodeDLLFullPath, GameCodeLockFullPath);
                    }
             
                    // TODO: Zeroing macro
                    // TODO: We can't zero everything because the up/down state will be wrong!!!
                    game_controller_input *OldKeyboardController = GetController(OldInput, 0);
                    game_controller_input *NewKeyboardController = GetController(NewInput, 0);
                    *NewKeyboardController = {};
                    NewKeyboardController->IsConnected = true;
                    for (int ButtonIndex = 0; ButtonIndex < ArrayCount(NewKeyboardController->Buttons); ++ButtonIndex)
                    {
                        NewKeyboardController->Buttons[ButtonIndex].EndedDown = OldKeyboardController->Buttons[ButtonIndex].EndedDown;
                    }
             
                    Win32ProcessPendingMessages(&Win32State, NewKeyboardController);


                    if (!GlobalPause)
                    {
                        POINT MouseP;
                        GetCursorPos(&MouseP);
                        ScreenToClient(Window, &MouseP);
                        NewInput->MouseX = MouseP.x;
                        NewInput->MouseY = MouseP.y;
                        NewInput->MouseZ = 0; // TODO: Support mouse wheel?
                        Win32ProcessKeyboardMessage(&NewInput->MouseButtons[0], GetKeyState(VK_LBUTTON) & (1 << 15));
                        Win32ProcessKeyboardMessage(&NewInput->MouseButtons[1], GetKeyState(VK_MBUTTON) & (1 << 15));
                        Win32ProcessKeyboardMessage(&NewInput->MouseButtons[2], GetKeyState(VK_RBUTTON) & (1 << 15));
                        Win32ProcessKeyboardMessage(&NewInput->MouseButtons[3], GetKeyState(VK_XBUTTON1) & (1 << 15));
                        Win32ProcessKeyboardMessage(&NewInput->MouseButtons[4], GetKeyState(VK_XBUTTON2) & (1 << 15));
                  
                        // TODO: Need to not poll disconnected controllers to avoid xinput frame rate hit on older libraries
                        // TODO: Should we poll this more frequently?
                        // NOTE: Reserving controller 0 for keyboard
                        DWORD MaxControllerCount = XUSER_MAX_COUNT;
                        if (MaxControllerCount > (ArrayCount(NewInput->Controllers) - 1))
                        {
                            MaxControllerCount = (ArrayCount(NewInput->Controllers) - 1);
                        }
                        for (DWORD ControllerIndex = 0; ControllerIndex < MaxControllerCount; ++ControllerIndex)
                        {
                            DWORD OurControllerIndex = ControllerIndex + 1;
                            game_controller_input *OldController = GetController(OldInput, OurControllerIndex);
                            game_controller_input *NewController = GetController(NewInput, OurControllerIndex);
             
                            XINPUT_STATE ControllerState;
                            if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
                            {
                                NewController->IsConnected = true;
                                NewController->IsAnalog = OldController->IsAnalog;
                   
                                // NOTE: Controller is plugged in
                                // TODO: See if ControllerState.dwPacketNumber increments too rapidly
                                XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

                                // TODO: This is a square deadzone, check XInput to verify whether it's round
                                NewController->StickAverageX = Win32ProcessXInputStickValue(Pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                                NewController->StickAverageY = Win32ProcessXInputStickValue(Pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

                                if ((NewController->StickAverageX != 0.0f) || (NewController->StickAverageY != 0.0f))
                                {
                                    NewController->IsAnalog = true;
                                }

                                // Translating D-Pad to stick movements
                                if (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP)
                                {
                                    NewController->StickAverageY = 1.0f;
                                    NewController->IsAnalog = false;
                                }
                                if (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
                                {
                                    NewController->StickAverageY = -1.0f;
                                    NewController->IsAnalog = false;
                                }
                                if (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
                                {
                                    NewController->StickAverageX = -1.0f;
                                    NewController->IsAnalog = false;
                                }
                                if (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
                                {
                                    NewController->StickAverageX = 1.0f;
                                    NewController->IsAnalog = false;
                                }
                    
                                real32 Threshold = 0.5f;
                                Win32ProcessXInputDigitalButton((NewController->StickAverageX < -Threshold)? 1 : 0,
                                                                &OldController->MoveLeft, 1,
                                                                &NewController->MoveLeft);
                                Win32ProcessXInputDigitalButton((NewController->StickAverageX > Threshold)? 1 : 0,
                                                                &OldController->MoveRight, 1,
                                                                &NewController->MoveRight);
                                Win32ProcessXInputDigitalButton((NewController->StickAverageY < -Threshold)? 1 : 0,
                                                                &OldController->MoveDown, 1,
                                                                &NewController->MoveDown);
                                Win32ProcessXInputDigitalButton((NewController->StickAverageY > Threshold)? 1 : 0,
                                                                &OldController->MoveUp, 1,
                                                                &NewController->MoveUp);

                                Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                                &OldController->ActionDown, XINPUT_GAMEPAD_A,
                                                                &NewController->ActionDown);
                                Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                                &OldController->ActionRight, XINPUT_GAMEPAD_B,
                                                                &NewController->ActionRight);
                                Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                                &OldController->ActionLeft, XINPUT_GAMEPAD_X,
                                                                &NewController->ActionLeft);
                                Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                                &OldController->ActionUp, XINPUT_GAMEPAD_Y,
                                                                &NewController->ActionUp);
                                Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                                &OldController->LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER,
                                                                &NewController->LeftShoulder);
                                Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                                &OldController->RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER,
                                                                &NewController->RightShoulder);

                                Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                                &OldController->Start, XINPUT_GAMEPAD_START,
                                                                &NewController->Start);
                                Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                                &OldController->Back, XINPUT_GAMEPAD_BACK,
                                                                &NewController->Back);
                            }
                            else
                            {
                                // NOTE: Controller is not available
                                NewController->IsConnected = false;
                            }
                        }

                        // Thread context for multi-threading
                        thread_context Thread = {};
             
                        // Platform-independent display buffer stuffs
                        game_offscreen_buffer Buffer = {};
                        Buffer.Memory = GlobalBackBuffer.Memory;
                        Buffer.Width = GlobalBackBuffer.Width;
                        Buffer.Height = GlobalBackBuffer.Height;
                        Buffer.Pitch = GlobalBackBuffer.Pitch;
                        Buffer.BytesPerPixel = GlobalBackBuffer.BytesPerPixel;

                        // Play back recorded input
                        if (Win32State.InputRecordingIndex)
                        {
                            Win32RecordInput(&Win32State, NewInput);
                        }
                        if (Win32State.InputPlayingIndex)
                        {
                            Win32PlayBackInput(&Win32State, NewInput);
                        }

                        if (Game.UpdateAndRender)
                        {
                            Game.UpdateAndRender(&Thread, &GameMemory, NewInput, &Buffer);
                        }

                        // Time audio for sync
                        LARGE_INTEGER AudioWallClock = Win32GetWallClock();
                        real32 FromBeginToAudioSeconds = Win32GetSecondsElapsed(FlipWallClock, AudioWallClock);
                  
                        DWORD PlayCursor;
                        DWORD WriteCursor;
                        if (GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor) == DS_OK)
                        {
                            // NOTE: Compute how much sound to write and where
                            /* STUDY
                               With unsynced audio, consider the fixed rendering rate.
                               When we query the sound buffer the
                               Write Cursor may be behind or ahead of the next frame boundary.
                               For syncing reasons we don't want to write only as much audio as will be used until the next frame.
                 
                               Finished rendering here
                               |
                               |------------|------------|------------|
                               33 ms
                               |---|~~~~(1)~~~~~|~(2)~|           Low latency, WC is behind the frame boundary.
                               PC  WC
                               1. Project WC forward: WC + number of samples per frame.
                               2. Target next frame boundary.

                               |-----------|~~~~(1)~~~~~|~(2)~|   High latency, WC is ahead of the frame boundary.
                               PC          WC
                               1. Project WC forward: WC + number of samples per frame.
                               2. Target the next safety margin

                               Safety margin is the number of samples we think game update loop will vary by
                               (Let's say up to 2 ms).
                            */
                            if (!SoundIsValid)
                            {
                                SoundOutput.RunningSampleIndex = WriteCursor / SoundOutput.BytesPerSample;
                                SoundIsValid = true;
                            }

                            DWORD ByteToLock = (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize; // index wraps

                            DWORD ExpectedSoundBytesPerFrame =  (int) ((real32) SoundOutput.SamplesPerSecond * (real32) SoundOutput.BytesPerSample / GameUpdateHz);
                            real32 SecondsLeftUntilFlip = TargetSecondsPerFrame - FromBeginToAudioSeconds;
                            DWORD ExpectedBytesUntilFlip = (DWORD) ((SecondsLeftUntilFlip / TargetSecondsPerFrame) * (real32) ExpectedSoundBytesPerFrame);
                            DWORD ExpectedFrameBoundaryByte = PlayCursor + ExpectedBytesUntilFlip;
                  
                            DWORD SafeWriteCursor = WriteCursor;
                            if (SafeWriteCursor < PlayCursor)
                            {
                                SafeWriteCursor += SoundOutput.SecondaryBufferSize;
                            }
                            Assert(SafeWriteCursor >= PlayCursor);
                            SafeWriteCursor += SoundOutput.SafetyBytes;
                  
                            bool32 AudioCardIsLowLatentency = SafeWriteCursor < ExpectedFrameBoundaryByte;
                  
                            DWORD TargetCursor = 0;
                            if (AudioCardIsLowLatentency)
                            {
                                TargetCursor = ExpectedFrameBoundaryByte + ExpectedSoundBytesPerFrame;
                            }
                            else
                            {
                                TargetCursor = WriteCursor + ExpectedSoundBytesPerFrame + SoundOutput.SafetyBytes;
                            }
                            TargetCursor = TargetCursor % SoundOutput.SecondaryBufferSize;
                  
                            DWORD BytesToWrite = 0;
                            if (ByteToLock > TargetCursor)
                            {
                                BytesToWrite = SoundOutput.SecondaryBufferSize - ByteToLock;
                                BytesToWrite += TargetCursor;
                            }
                            else
                            {
                                BytesToWrite = TargetCursor - ByteToLock;
                            }
                  
                            game_sound_output_buffer SoundBuffer = {};
                            SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
                            SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
                            SoundBuffer.Samples = Samples;

                            if (Game.GetSoundSamples)
                            {
                                Game.GetSoundSamples(&Thread, &GameMemory, &SoundBuffer);
                            }
                            /*
                              The kernel, controller w/e is controlling the primary buffer.
                              Remember we may be interrupted or w/e at any time. But the hardware keeps going.
                              We need to figure out where we need to fill by tracking the cursor positions.
                              We need to lock the buffer at whereever we left off.
                            */
#if HANDMADE_INTERNAL
                            win32_debug_time_marker *Marker = &DebugTimeMarkers[DebugTimeMarkerIndex];
                            Marker->OutputPlayCursor = PlayCursor;
                            Marker->OutputWriteCursor = WriteCursor;
                            Marker->OutputLocation = ByteToLock;
                            Marker->OutputByteCount = BytesToWrite;
                            Marker->ExpectedFlipPlayCursor = ExpectedFrameBoundaryByte;
                  
                            DWORD UnwrappedWriteCursor = WriteCursor;
                            if (UnwrappedWriteCursor < PlayCursor)
                            {
                                UnwrappedWriteCursor += SoundOutput.SecondaryBufferSize;
                            }
                            AudioLatencyBytes = UnwrappedWriteCursor - PlayCursor;
                            AudioLatencySeconds = ((real32) AudioLatencyBytes / (real32) SoundOutput.BytesPerSample) / (real32) SoundOutput.SamplesPerSecond;
#if 0
                            char TextBuffer[256];
                            _snprintf_s(TextBuffer, sizeof(TextBuffer),"BTL:%u TC:%u BTW:%u - PC:%u WC:%u DELTA:%u (%fs)\n", ByteToLock, TargetCursor, BytesToWrite, PlayCursor, WriteCursor, AudioLatencyBytes, AudioLatencySeconds);
                            OutputDebugStringA(TextBuffer);
#endif
#endif
                            Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer);
                        }
                        else
                        {
                            SoundIsValid = false;
                        }

                        LARGE_INTEGER WorkCounter = Win32GetWallClock();
                        real32 WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);

                        // TODO: NOT TESTED YET! PROBABLY BUGGY!!!!
                        real32 SecondsElapsedForFrame = WorkSecondsElapsed;
                        if (SecondsElapsedForFrame < TargetSecondsPerFrame)
                        {
                            // NOTE: Sleep makes the thread wait until the next scheduler check
                            if (SleepIsGranular)
                            {
                                DWORD SleepMS = (DWORD) (1000.0f * (TargetSecondsPerFrame - SecondsElapsedForFrame));
                                if (SleepMS > 0)
                                {
                                    Sleep(SleepMS);
                                }
                            }
                            real32 TestSecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter, Win32GetWallClock());
                            if (TestSecondsElapsedForFrame < TargetSecondsPerFrame)
                            {
                                // TODO: LOG MISSED SLEEP HERE
                            }
                            while (SecondsElapsedForFrame < TargetSecondsPerFrame)
                            {
                                SecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter, Win32GetWallClock());
                            }
                        }
                        else
                        {
                            // TODO: MISSED FRAME RATE!
                            // TODO: Logging
                        }

                        // NOTE: Snap this here so that the remaining time counts towards the next frame
                        LARGE_INTEGER EndCounter = Win32GetWallClock();
                        real32 MSPerFrame = 1000.0f * Win32GetSecondsElapsed(LastCounter, EndCounter);
                        LastCounter = EndCounter;

                        win32_window_dimension Dimension = Win32GetWindowDimension(Window);
#if HANDMADE_INTERNAL
                        // TODO: Wrong on the 0th index
                        //Win32DebugSyncDisplay(&GlobalBackBuffer, ArrayCount(DebugTimeMarkers), DebugTimeMarkers, DebugTimeMarkerIndex - 1, &SoundOutput, TargetSecondsPerFrame);
#endif
                  
                        // NOTE: Since we specified CS_OWNDC we can just get one device context and use it forever.
                        HDC DeviceContext = GetDC(Window);
                        Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height);
                        ReleaseDC(Window, DeviceContext);

                        FlipWallClock = Win32GetWallClock();
                        // NOTE: This is debug code
#if HANDMADE_INTERNAL
                        {
                            DWORD PlayCursor;
                            DWORD WriteCursor;
                            if (GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor) == DS_OK)
                            {
                                Assert(DebugTimeMarkerIndex < ArrayCount(DebugTimeMarkers));
                                win32_debug_time_marker *Marker = &DebugTimeMarkers[DebugTimeMarkerIndex];
                                Marker->FlipPlayCursor = PlayCursor;
                                Marker->FlipWriteCursor = WriteCursor;
                            }
                        }
#endif
             
                        game_input *Temp = NewInput;
                        NewInput = OldInput;
                        OldInput = Temp;
                        // TODO: Should I clear these here?
#if 0
                        uint64 EndCycleCount = __rdtsc();
                        uint64 CyclesElapsed = EndCycleCount - LastCycleCount;
                        LastCycleCount = EndCycleCount;

                        real64 FPS = 0.0f;
                        real64 MCPF = (real64) CyclesElapsed / (1000.0f * 1000.0f);
             
                        char FPSBuffer[256];
                        sprintf_s(FPSBuffer, "Milliseconds/Frame: %.02fms/f.  %.02ff/s, %.02fMc/f\n", MSPerFrame, FPS, MCPF);
                        OutputDebugStringA(FPSBuffer);
#endif
                  
#if HANDMADE_INTERNAL
                        ++DebugTimeMarkerIndex;
                        if (DebugTimeMarkerIndex == ArrayCount(DebugTimeMarkers))
                        {
                            DebugTimeMarkerIndex = 0;
                        }
#endif
                    }
                }
            }
            else
            {
                // TODO: Logging
            }
        }
        else
        {
            // TODO: Logging
        }
    }
    else
    {
        // TODO: Logging
    }

    return 0;
}
