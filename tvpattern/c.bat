@echo off
rem Compilation...

if exist vga.pio.h del vga.pio.h
..\_exe\pioasm.exe -o c-sdk ..\_picovga\vga.pio vga.pio.h
if exist vga.pio.h goto start
goto stop

:start
call ..\_c1.bat

:stop
