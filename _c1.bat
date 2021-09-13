@echo off
rem Compilation...

if EXIST ..\arm_path.txt (
    set /p ARM_PATH=<..\arm_path.txt
) else (
    set ARM_PATH=C:\ARM10\bin
)

set PATH=..\_tools;%ARM_PATH%;%PATH%

if exist program.uf2 del program.uf2
make all
if errorlevel 1 goto err
if not exist program.uf2 goto err
echo.
type program.siz
goto end

:err
pause ERROR!
:end
