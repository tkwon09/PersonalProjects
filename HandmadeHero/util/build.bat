@echo off

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

REM we are using vcvarsall.bat compiler. Check shell.bat for details.
REM user32.lib Gdi32.lib are windows stuff we need to link to so we have access to windows functions

REM DHANDMADE_INTERNAL   flags developer-only build
REM DHANDMANDE_WIN32     flags Windows build

REM -Zi produces a debug log. It's the .pdb files and such read by the debugger to tell it where the stuff is in the executable.
REM -Z7 is an older version of -Zi that has better multi-processor support and such... more compatible

REM A higher warning level will help us catch small, subtle errors that would otherwise require us to catch manually
REM -WX treats warnings as errors
REM -W4 sets warning level to 1 below -Wall since we get windows.h warnings otherwise
REM -wd4201 turns off nameless struct/union warning
REM -wd4100 turns off unused formal parameter warning
REM -wd4189 turns off initialized but unused variable warning. Compiler will optimize them out anyway.
REM -wd4505 turns off unused function warning.

REM -Od removes all optimization. For debugging purposes.
REM -Oi turns on compiler intrinsics. Let's compiler bypass calls if it can directly compute something on the CPU.
REM -GR- turns off runtime type information. Won't be using C++ runtime type information capabilities.
REM -EHa turns off exception handling. Won't be using exceptions.
REM -nologo removes the trademark printout when we compile.

REM -LD tells compiler to make a .dll
REM /EXPORT:<function name> tells the compiler which functions to expose for the .dll

REM fp:fast makes floating point faster but less accurate

REM TODO - can we just build x64 and x86 with one exe?

REM You may notice that many games require DirectX runtimes and redistributables.
REM Even the linker, bylike most truths,  default, will not create a XP-compatible binding.
REM /link will let us pass linker options.
REM -subsystem:windows,5.1 will allow the linker to create XP-compatible bindings 
REM -opt:ref will aggressively not link if a reference is unused

REM -MDd will dynamically link to possibly hidden dependencies. Debug version
REM -MT will statically link hidden dependencies as needed

REM -Fm tells linker to create a map file. Play around with it to check out where references originate from.
REM Can see the result of overloaded functions, inheritance: Name-mangling.

REM -incremental:no tells linker not to do incremental linking. We do everything ourself.

REM -PDB:<file name> let's us name our debug file so we can dynamically load while debugging in Vstudio

REM consuming error messages since we expect it to fail when Vstudio is debugging
del *.pdb > NUL 2> NUL

set CommonCompilerFlags= /fp:fast -MTd -nologo -GR- -EHa- -Od -Oi -WX -W4 -wd4505 -wd4201 -wd4100 -wd4189 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 -FC -Z7
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib Gdi32.lib winmm.lib

REM 32-bit build
REM cl %CommonCompilerFlags% ..\source\win32_handmade.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%
REM cl %CommonCompilerFlags% ..\source\handmade.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%

REM 64-bit build - CHECK shell.bat FOR x86 or x64 FLAG!!!
del *pdb > NUL 2> NUL
REM optimization switches: /O2
REM using a lock file to prevent visual studio from trying to load before pdb is finished (sometimes compiler finishes compiling
REM before pdb is finished being generated)
echo WAITING FOR PDB > lock.tmp
cl %CommonCompilerFlags% ..\source\handmade.cpp -Fmhandmade.map -LD /link -incremental:no /PDB:handmade_%random%.pdb /EXPORT:GameGetSoundSamples /EXPORT:GameUpdateAndRender
del lock.tmp
cl %CommonCompilerFlags% ..\source\win32_handmade.cpp -Fmwin32_handmade.map /link %CommonLinkerFlags%
popd
