# CeDImu

![CeDImu concept by jongg-eater](https://raw.githubusercontent.com/Stovent/CeDImu/master/resources/CeDImu-concept.png "CeDImu concept by jongg-eater")

Experimental Philips CD-I emulator
(I use a capital `I` instead of a lower case `i` because the Green Book capitalizes it)

Note: the project is still in a very early development stage, please open issues only to address problems in the existing code.

## How to use

See the [user manual](https://github.com/Stovent/CeDImu/blob/master/MANUAL.md).

## Compatibility

DVC support will be added when CeDImu will have a good compatibility with the base case system on several boards.

### BIOSes

Below are listed the known BIOSes that works in CeDImu along with their board settings. For more information, see [the ICDIA website](http://icdia.co.uk/players/comparison.html "icdia.co.uk").

- CDI 210/40 (Mono-3, 8 KB NVRAM)
- CDI 220/60 (Mono-3, 32 KB NVRAM)
- CDI 220/80 (Mono-4, 32 KB NVRAM)
- CDI 470/00 (Mono-4, 8 KB NVRAM)
- CDI 470/20 (Mono-4, 8 KB NVRAM)
- CDI 490/00 (Mono-4, 32 KB NVRAM)

Some BIOSes (e.g. CDI 220/80) may not show graphics on their first boot. To boot them, let them run for around a thousand frames on the first boot so they initializes their NVRAM, then reload the emulator.

### Boards

Compatible means it is capable of playing discs.

- [ ] Mini-MMC

  ​	Not working. SCC66470 and Timekeeper mapped.

- [ ] Mono-3, Mono-4

  BIOS boots to player shell. MCD212, Timekeeper (8KB and 32KB) and HLE IKAT mapped.

### Chips

- [x] SCC68070
- [ ] VDSCs
  - [ ] SCC66470 (Mini-MMC)
  - [x] MCD212 (Mono-1, 2, 3, 4)
- [ ] CD and Audio
  - [ ] CDIC (Mini-MMC, Mono-1)
  - [ ] DSP (Mono-2)
  - [ ] MCD221 CIAP (Mono-3, 4)
- [ ] Slave MCUs
  - [ ] Slave (Mini-MMC, Mono-1, 2)
  - [ ] IKAT (Mono-3, 4)
- [x] Timekeepers
  - [x] M48T08
  - [x] DS1216

## Features

- [ ] CDI-related functions
  - [x] CDI file system
  - [x] Export files
  - [x] Export audio
  - [ ] Export video
  - [x] Export raw video



- [ ] Tools
  - [x] CPU Viewer
  - [x] VDSC Viewer
  - [x] Debug (memory access logs and exception and system call tracing)
  - [ ] RAM Search
  - [ ] Memory Viewer
  - [ ] Savestates

## How to build

You need a compiler that supports C++26 and wxWidgets 3.2.

When cloning the repo, make sure you clone the submodules too (`git clone --recurse-submodules https://github.com/Stovent/CeDImu`).

### Build macros

CeDImu (the frontend) and libCeDImu (the emulation core) accepts the following macros:

`LIBCEDIMU_ENABLE_LOG`: if defined, allows the library to print some messages in the console and the use of OnLogMemoryAccess callback.

`LIBCEDIMU_ENABLE_RENDERERSIMD`: if on, uses the hardware-accelerated SIMD renderer. Requires the C++ header `<experimental/simd>`.

### CMake

#### CMake options

The CMake options below controls how to build CeDImu and the build macros listed upper.

`LIBCEDIMU_ENABLE_LOG`: see section `Build macros` (default: `ON`). The official build of CeDImu always enables it.

`LIBCEDIMU_ENABLE_RENDERERSIMD`: builds and uses the SIMD renderer, see section `Build macros` (default: `OFF`).

`LIBCEDIMU_PROFILE_GNU`: if true, adds profiling arguments to GCC (clang/MSVC not supported) (default: `OFF`).

`LIBCEDIMU_ENABLE_ASAN`: if true, uses address sanitizer for GCC and clang (MSVC not yet supported) (default: `OFF`).

`LIBCEDIMU_REBUILD_SOFTCDI`: If ON, will build the SoftCDI modules from sources (requires DOSBox). If OFF, will use the prebuilt module binaries (default: `OFF`).

`CEDIMU_BUILD_CDITOOL`: If ON, builds the little `cditool` program (Linux only, requires libcdio) (default: `OFF`).

`CEDIMU_BENCHMARKS`: if ON, builds the benchmarks (default: `OFF`).

`CEDIMU_TESTS`: If ON, builds the unit tests (requires Catch2 cloned) (default: `ON`).

`CEDIMU_TESTS_ASAN`: If ON, builds the unit tests with address sanitizer options (default: `OFF`).

`CEDIMU_ENABLE_LTO`: If ON, compiles the executable with link-time optimisations (default: `ON`).

#### Windows

First install [CMake](https://cmake.org/download/) and [vcpkg](https://github.com/microsoft/vcpkg/), and use vcpkg to install wxWidgets: `vcpkg.exe install wxwidgets:x64-windows-static`

Then build CeDImu (make sure to change the `DCMAKE_TOOLCHAIN_FILE` to the path your vcpkg install):
```sh
cmake -B build -S . "-DCMAKE_TOOLCHAIN_FILE=C:/Dev/vcpkg/scripts/buildsystems/vcpkg.cmake" "-DVCPKG_TARGET_TRIPLET=x64-windows-static" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CONFIGURATION_TYPES=Release -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
cmake --build .\build\ --config Release
```

For a Debug build, execute these commands instead:
```sh
cmake -B build -S . "-DCMAKE_TOOLCHAIN_FILE=C:/Dev/vcpkg/scripts/buildsystems/vcpkg.cmake" "-DVCPKG_TARGET_TRIPLET=x64-windows-static" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CONFIGURATION_TYPES=Debug -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug
cmake --build .\build\ --config Debug
```

The executable will be in the `build` directory.

#### Linux

First install cmake and wxWidgets-3.1 (or later).

With apt the command is: `sudo apt install cmake libwxgtk3.2-dev`

Install the dependency, then open a terminal in the root directory of the repo and type:

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc --all)
```

The executable will be in the `build` directory.

#### macOS

Package dependency: `wxwidgets`, `cmake` and `llvm` (e.g. if using brew run: `brew install wxwidgets cmake llvm`). Also make sure to have Xcode or just it's Command Line Tools installed.

LLVM clang is used to build because AppleClang lacks recent features.

For keyboard input to work properly, enable 'Keyboard Navigation' in macOS. See this [support article](https://support.apple.com/en-us/HT204434#fullkeyboard).

Open a terminal in the root directory of the repo and type:

```sh
mkdir build
cd build
CXX=$(brew --prefix llvm)/bin/clang++ cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.physicalcpu)
```

## libCeDImu

If I get everything listed upper working, stable and fully functional, the goal is to create libCeDImu, a complete library (static and/or dynamic) to allow any program to implement CDI applications (other emulators like Bizhawk, MAME, etc).

## Special Thanks

- [CD-i Fan](https://www.cdiemu.org/) for his help and his information that made me progress way faster than I could imagine.
- jongg-eater for the logo.
