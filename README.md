

# CeDImu

![CeDImu concept by jongg-eater](https://raw.githubusercontent.com/Stovent/CeDImu/master/resources/CeDImu-concept.png "CeDImu concept by jongg-eater")

Experimental Philips CD-I emulator  
(I use a capital `I` instead of a lower case `i` because the Green Book capitalizes it)

Note: the project is still in a very early development stage, please open issues only to address problems in the existing code.

## How to build
CeDImu depends on wxWidgets 3.0.

#### Build macros

``USE_STD_FILESYSTEM``: if defined, will use the C++17 filesystem functions. Otherwise, will be using wxWidgets' functions (default).

``FILESYSTEM_EXPERIMENTAL``: if defined with ``USE_STD_FILESYSTEM``, will include ``<experimental/filesystem>``. Otherwise, will include ``<filesystem>`` (default).

### CMake

#### Variables

``LIBRARY_TYPE``: used to build the core as a static or shared library (default: ``STATIC``).

``BUILD_CDITOOL``: build cditool or not (default: ``OFF``).

#### Windows

Use CMake-GUI

#### Linux

Package dependency: ``libwxgtk3.0-dev``

Install the dependency, then open a terminal in the root directory of the git and type:

```sh
mkdir build
cd build
cmake ..
make -j$(nproc --all)
```

### Makefile (deprecated)

#### Windows

Use MinGW
1. Open the Makefile and set the following variables:
* ``wxLibPath``: contains the static libraries (e.g. libwxbase30u.a)
* ``wxIPATH``: include directory (path must not contain the ``wx/`` subdirectory)
* ``wxIMSWU``: mswu directory inside the static libraries directory
2. Open a terminal and type ``make -f WinMakefile`` (Assuming ``make`` is in your PATH variable).

## Compatibility

DVC support will be added when CeDImu will have a good compatibility with the base case system on several boards.

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

### Boards

Compatible means it is capable of playing discs.

- [ ] Mini-MMC

  ​	Not working. SCC66470 and Timekeeper mapped.

- [ ] Mono-3

  BIOS boots to player shell. MCD212, Timekeeper and HLE IKAT mapped.

## Features

- [ ] CDI-related functions

  - [x] CDI file system
  - [x] Export files
  - [x] Export audio
  - [ ] Export video

  

- [ ] Tools
  - [ ] RAM Search
  - [ ] Memory Viewer
  - [x] CPU Viewer
  - [x] VDSC Viewer
  - [ ] Savestates

## libCeDImu
If I get everything listed upper working, stable and fully functional, the goal is to create libCeDImu, a complete library (static and/or dynamic) to allow any program to implement CDI applications (other emulators like Bizhawk, MAME, etc).

## Special Thanks

- [CD-i Fan](https://github.com/cdifan) for his help and his information that made me progress way faster than I could imagine.
-  jongg-eater for the logo.

