

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
- [ ] SCC66470 (Mini-MMC)
- [x] MCD212 (Mono-1, 2, 3, 4)
- [ ] CDIC (Mini-MMC, Mono-1)
- [ ] DSP (Mono-2)
- [ ] MCD221 CIAP (Mono-3, 4)
- [ ] Slave (Mini-MMC, Mono-1, 2)
- [ ] IKAT (Mono-3, 4)
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

#### Build macros

`ENABLE_LOG`: if defined, allows the library to print some messages in the console and the use of OnLogMemoryAccess callback (default: `OFF`).

The official build of CeDImu always enables it.

### CMake

#### Variables

`LIBRARY_TYPE`: used to build the core as a static or shared library (default: `STATIC`).

`BUILD_CDITOOL`: build cditool or not (default: `OFF`).

#### Windows

Use CMake-GUI

#### Linux

Package dependency: `libwxgtk3.0-gtk3-dev`

Install the dependency, then open a terminal in the root directory of the git and type:

```sh
mkdir build
cd build
cmake ..
make -j$(nproc --all)
```

If you want to have the memory access logs, replace `cmake ..` with `cmake .. -DENABLE_LOG=1`

## libCeDImu

If I get everything listed upper working, stable and fully functional, the goal is to create libCeDImu, a complete library (static and/or dynamic) to allow any program to implement CDI applications (other emulators like Bizhawk, MAME, etc).

## Special Thanks

- [CD-i Fan](https://github.com/cdifan) for his help and his information that made me progress way faster than I could imagine.
-  jongg-eater for the logo.

