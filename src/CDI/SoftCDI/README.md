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

## TODO

- rename sysgo to launcher
- rename ciapdriv to something like cdfmdriv because I do not have a ciap, and replace the /cd dev desc

## Useful notes

### Assembly labels and offsets
From the OS-9 Assembler/Linker document and binary analysis, we can understand how labels works.
See Chapter 1, section `Label Fields` and Chapter 7, sections `Program and Data Memory References` and onward.
Let's consider a sample below, where label CDDevice is at 0xFB position from the begining of the module header:

```
    psect ...

Main
    ...
	lea CDDevice(a0),a0         lea (0xfb,A0),A0
	lea CDDevice(pc),a0         lea (0x6b,PC),A0
	lea DiscLabel(a0),a0        lea (-0x7ff0,A0),A0
	lea DiscLabel(pc),a0        lea (-0x7ff0,PC),A0
	lea Main(a0),a0             lea (0x4e,A0),A0 -> address of the Main entry point.
	lea Main(pc),a0             lea (-0x42,PC),A0

T2Device	dc.b	"/t2",0
NvrDevice	dc.b	"/nvr",0
CDDevice	dc.b	"/cd",0

	vsect
unused		ds.b $10
DiscLabel	ds.b $8000
	ends

	ends
```

We can clearly see that every label in the vsect section is their offset from the start of the static storage section,
minus 0x8000 to account for the biased A6.

We can also see that labels in code relative NOT to PC like `CDDevice(a0)`, then the label is just its offset from the
start of the module header.
But labels in code relative to PC like `CDDevice(pc)` are the offset from the current instruction (+2).

Unsurprisingly, we can see that code labels are not different.

This doesn't seem to be affected by an org directive.
