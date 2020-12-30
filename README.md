

# CeDImu

Experimental Philips CD-I emulator  
(I use a capital `I` instead of a lower case `i` because the Green Book capitalizes it)

Note: the project is still in a very early development stage, please open issues only to address problems in the existing code.

## How to build
CeDImu depends on wxWidgets 3.0.

#### Build macros

USE_STD_FILESYSTEM: if this macro is defined, will use the C++17 filesystem functions. Otherwise, will be using wxWidgets' functions (default).

### CMake

#### Variables

LIBRARY_TYPE: used to build the core as a static or shared library (default: STATIC).

#### Windows

Use CMake-GUI

#### Linux

Package dependency: `libwxgtk3.0-dev`

Install the dependency, then open a terminal in the root directory of the git and type:

```sh
mkdir build
cd build
cmake ..
make -j$(nproc --all)
```

### Makefile

#### Windows

Use MinGW
1. Open the Makefile and set the following variables:
* `wxLibPath`: contains the static libraries (e.g. libwxbase30u.a)
* `wxIPATH`: include directory (path must not contain the `wx/` subdirectory)
* `wxIMSWU`: mswu directory inside the static libraries directory
2. Open a terminal and type `make -f WinMakefile` (Assuming `make` is in your PATH variable).

## TODO
- [x] SCC68070

- [x] MCD212

- [ ] SCC66470

- [ ] MCD221 (audio)

- [ ] MC68HC05 (slave)

- [ ] CDIC

  

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
