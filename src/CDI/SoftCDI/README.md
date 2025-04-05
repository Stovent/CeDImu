# SoftCDI

SoftCDI patches a regular CDI BIOS with custom modules for programs and file managers to intercept CDRTOS calls and reimplements the Green Book in software.

The following modules are reimplemented:
- csd
- sysgo
- nvdrv
- video

## csd

csd as of the Green Book Appendix VII.2.

## sysgo

The custom sysgo module does the following:
- Change the current data and execution directory to "/cd"
- F$Load the disc main module
- F$Chain to the module

## nvdrv

The NVRAM driver, allows to save the data on the host.

## video

Reimplementation of the Green Book in software.

## CMake toolchain

It is compiled along with the C++ code. You need to have DOSBox findable by CMake's `find_program`, usually setting dosbox in PATH is sufficient.

You also need to set the CMake variable `OS9C` to the root directory of the original OS9 compiler (root directory must have `OS9C/BIN/R68.exe`).

## OS9C toolchain

See `The OS-9 guru - chapter 6` and the [OS-9 Assembler/Linker](http://icdia.co.uk/microware/77165106.pdf) for how to use the compiler and conventions.
`$SOFTCDI` is the path to the repo's directory `CeDImu/src/CDI/SoftCDI`.

- Install DOSBox
- Mount the compiler with `mount c <path to host OS9C>`
- Mount the destination with `mount d $SOFTCDI`
- Add the `C:\BIN` directory to MS DOS' PATH.
- go to `d:` and run `build.bat`.
- If successful, the host folder `$SOFTCDI/build` contains the compiled modules.
- On Linux, cd to `$SOFTCDI` and run `generate_headers.sh` to embed the binaries in .h files.
