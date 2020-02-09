# CeDImu
Experimental Philips CD-I emulator
(I use a capital `I` instead of a lower case `i` because the Green Book capitalizes it)

Note: the project is still in a very early development stage, please open issues only to address problems in the existing code.

## How to build
CeDImu depends on wxWidgets.

### Windows
#### Makefile
Use MinGW
1. Open the Makefile and set the following variables:
* `wxLibPath`: contains the static libraries (e.g. libwxbase30u.a)
* `wxIPATH`: include directory (path must not contain the `wx/` subdirectory)
* `wxIMSWU`: mswu directory inside the static libraries directory
2. Open a terminal and type `make -f WinMakefile` (Assuming `make` is in your PATH variable).

#### CMake
Use CMake-GUI.

### Linux
Package dependency: `libwxgtk3.0-dev`

Install the dependency, then open a terminal in the root directory of the git and type:

```sh
cmake .
make -j$(nproc --all)
```
Find the executable in the bin folder

## TODO
* Fix potential bugs in SCC68070 emulation
* MCD212 and SCC66470 emulation
* Export video
* Export audio
* Sound emulation
* Basic TAS Tools (RAM Watch, save states, movie recording, avi recording)

Release 1.0 when this TODO list will be implemented.

### TODO later
* MCD251 (MPEG Full Motion Video Decoder (FMV))

## libCDI
If I get everything listed upper (except TAS Tools) working, stable and fully functional, the goal is to create libCDI, a complete library (static and/or dynamic) to allow any program to implement CDI application (other emulators like Bizhawk, MAME, etc)
