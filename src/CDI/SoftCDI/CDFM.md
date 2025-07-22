# CDFM

This document explains how CDFM works from reverse-engineered data.

## Path Descriptor Static Storage

This lists the data stored in the file manager static storage section of a path descriptor (offset 0x2A).
The offsets below are from the beginning of the path descriptor.

- [0xE] PD_BUF: this fields holds a pointer to a sector buffer (2048 bytes).
- [0x3E]: Set to 0 in FUN_00001086.
- [0x42]: Set to 0 in FUN_00001086.
- [0x4A]: Looks to be the current process ID.
- [0x50]: Set to 1 in FUN_00001086.
- [0x52]: Pointer to a Play Control Block used when calling I$Read/IChgDir.
- [0x56]: Pointer to a Play Control List used when calling I$Read.
- [0x5A]: Pointer to the Channel Index List for Video/Program related data.
- [0x5E]: Pointer to the drive table.
- [0x84]: Drive number of this path (starting at 0).
- [0x8E]: initial value when initializing PCB_Chan of play control block.
