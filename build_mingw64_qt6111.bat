@echo off
setlocal

set "QT_DIR=C:\Qt\6.11.1\mingw_64"
set "BUILD_DIR=%~dp0build-mingw64"

if not exist "%QT_DIR%\bin\qt-cmake.bat" (
    echo ERROR: Qt 6.11.1 MinGW 64-bit was not found at:
    echo %QT_DIR%
    echo Edit QT_DIR in this file if Qt is installed elsewhere.
    exit /b 1
)

if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

call "%QT_DIR%\bin\qt-cmake.bat" -G Ninja -DCMAKE_BUILD_TYPE=Release "%~dp0"
if errorlevel 1 exit /b 1

cmake --build . --parallel
if errorlevel 1 exit /b 1

echo.
echo BUILD SUCCESSFUL
echo Paid lecture price: $0.03 USD
echo EXE:
echo %BUILD_DIR%\SabaiBooksQt.exe
endlocal
