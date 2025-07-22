@echo off

set GRPUSER=0.0

echo %1
r68 %1.a -o=build/%1.r
if ERRORLEVEL 1 pause
l68 build/%1.r -n=%1 -o=build/%1 -l=c:/LIB/SYS.L -l=c:/LIB/CDISYS.L
if ERRORLEVEL 1 pause
