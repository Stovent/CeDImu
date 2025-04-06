@echo off

rem TODO: make it a function

set GRPUSER=0.0

echo ciapdriv
r68 ciapdriv.a -o=build/ciapdriv.r
l68 build/ciapdriv.r -n=ciapdriv -o=build/ciapdriv

echo csd_450
r68 csd_450.a -o=build/csd_450.r
l68 build/csd_450.r -n=csd_450 -o=build/csd_450

echo nvdrv
r68 nvdrv.a -o=build/nvdrv.r
l68 build/nvdrv.r -n=nvdrv -o=build/nvdrv -l=c:/LIB/SYS.L

echo video
r68 video.a -o=build/video.r
l68 build/video.r -n=video -o=build/video -l=c:/LIB/SYS.L

echo sysgo
r68 sysgo.a -o=build/sysgo.r
l68 build/sysgo.r -n=sysgo -o=build/sysgo -l=c:/LIB/SYS.L
