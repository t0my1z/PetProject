@echo off
REM Build script for PetProject on Windows

REM Change to project root directory (two levels up from scripts\windows)
cd /d "%~dp0..\.."

echo Cleaning build and install directories...
rmdir /s /q build 2>nul
rmdir /s /q install 2>nul

echo Configuring CMake...
cmake -B build
if errorlevel 1 (
    echo Error: CMake configuration failed
    exit /b 1
)

echo Building project...
cmake --build build --config Release
if errorlevel 1 (
    echo Error: CMake build failed
    exit /b 1
)

echo Installing project...
cmake --install build --prefix ./install --config Release
if errorlevel 1 (
    echo Error: CMake install failed
    exit /b 1
)

echo.
echo Build completed successfully!
pause
