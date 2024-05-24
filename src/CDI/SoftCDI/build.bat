@echo off

rem r68 csd.a -o=csd.r
rem l68 csd.r -n=csd -o=csd

r68 nvdrv.a -o=build/nvdrv.r
l68 build/nvdrv.r -n=nvdrv -o=build/nvdrv -l=c:/LIB/SYS.L

r68 video.a -o=build/video.r
l68 build/video.r -n=video -o=build/video -l=c:/LIB/SYS.L

r68 sysgo.a -o=build/sysgo.r
l68 build/sysgo.r -n=sysgo -o=build/sysgo -l=c:/LIB/SYS.L
