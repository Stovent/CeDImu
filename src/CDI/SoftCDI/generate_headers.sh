#!/bin/sh

cd build || exit 1
xxd -i -c16 CIAPDRIV ../include/CIAPDRIV.h
xxd -i -c16 CSD_450 ../include/CSD_450.h
xxd -i -c16 NVDRV ../include/NVDRV.h
xxd -i -c16 VIDEO ../include/VIDEO.h
xxd -i -c16 SYSGO ../include/SYSGO.h
