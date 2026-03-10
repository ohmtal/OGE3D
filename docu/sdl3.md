# SDL3 and platform

Goal is to remove platformX86UNIX, platformWin32 and platformMac
if possible. Some stuff like input is done in platformSDL already.  
I will not be able to test Mac so it will be in a works maybe state.

- [X] use system sdl2 and openAL first (on windows: vcpkg)
- [ ] port sdl2 sources to sdl3
- [ ] port platform code to sdl3 / c++20 / other cross platform libs:
    - Threads
    - Mutex
    - Semaphore
    - File Dialog => libtinyfiledialogs
    - File System handling **carefully because of zip support**
- [ ] ASM not sdl related but check if it's used:
    - ./platform/platformCPUInfo.asm
    - ./math/mMath_ASM.asm
    - ./math/mMathSSE_ASM.asm
    - ./math/mMathAMD_ASM.asm
