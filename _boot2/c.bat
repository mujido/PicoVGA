@echo off
rem Compilation...

if EXIST ..\arm_path.txt (
    set /p ARM_PATH=<..\arm_path.txt
) else (
    set ARM_PATH=C:\ARM10\bin
)

set PATH=..\_tools;%ARM_PATH%;%PATH%

call _c1.bat boot2_generic_03h
if not exist boot2_generic_03h.bin goto end

call _c1.bat boot2_is25lp080
if not exist boot2_is25lp080.bin goto end

call _c1.bat boot2_usb_blinky
if not exist boot2_usb_blinky.bin goto end

call _c1.bat boot2_w25q080
if not exist boot2_w25q080.bin goto end

call _c1.bat boot2_w25x10cl

:end
