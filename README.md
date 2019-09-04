# CeDImu
Experimental Philips CD-I emulator
(I use a capital `I` instead of a lower case `i` because the Green Book capitalizes it)

## How to build
CeDImu depends on wxWidgets.

### Windows
#### Makefile
Use MinGW
1. Create the bin/ and bin/obj/ directories first.
2. Open the Makefile and set the following variables:
* `wxLibPath`: contains the static libraries (e.g. libwxbase30u.a)
* `wxIPATH`: include directory (path must not contain the `wx/` subdirectory)
* `wxIMSWU`: mswu directory inside the static libraries directory
3. Open a terminal and type `make`.

#### CMake
Use Cmake-GUI.

### Linux
Dependency:
`` libwxgtk3.0-dev ``
```sh
cmake .
make -j$(nproc --all)
```
Find the executable in the bin folder

## TODO
* fix potential bugs in SCC68070 emulation
* MCD212 and SCC66470 emulation
* Export video 
* Export audio
* Sound emulation
* TAS Tools (RAM Watch, save states, movie recording, avi recording)

### TODO later
* MCD251 (MPEG Full Motion Video Decoder (FMV))
* More TAS tools (TAStudio, ...)

### libCDI
If I get everything listed upper (except TAS Tools) working, stable and fully functionnal, 
the goal is to create libCDI, a complete library to allow any program to implement CDI application (other emulators like Bizhawk, MAME, etc)

## Credits
* Stovent (creator of the project and primary developper)

## License
SoonTM
