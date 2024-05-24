#!/bin/sh

cd build || exit 1
xxd -i -c16 NVDRV ../include/NVDRV.h
xxd -i -c16 VIDEO ../include/VIDEO.h
xxd -i -c16 SYSGO ../include/SYSGO.h
