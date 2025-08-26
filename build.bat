@echo off
echo AutoTune Real-Time Audio Engine - Build Script
echo ============================================

REM Check if build directory exists
if not exist "build" (
    echo Creating build directory...
    mkdir build
)

cd build

REM Check for CMake
cmake --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: CMake not found!
    echo Please install CMake from https://cmake.org/download/
    echo Add CMake to your system PATH and try again.
    pause
    exit /b 1
)

echo Configuring project with CMake...
cmake .. -G "Visual Studio 16 2019" -A x64
if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed!
    echo Trying with Visual Studio 17 2022...
    cmake .. -G "Visual Studio 17 2022" -A x64
    if %errorlevel% neq 0 (
        echo ERROR: CMake configuration failed with both VS versions!
        echo Please check your Visual Studio installation.
        pause
        exit /b 1
    )
)

echo Building project...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo ============================================
echo Build completed successfully!
echo.
echo You can now run:
echo   - .\Release\autotune_example.exe  (Example application)
echo   - .\Release\autotune_tests.exe    (Test suite)
echo.
echo Press any key to run the example...
pause

if exist "Release\autotune_example.exe" (
    echo Running example application...
    .\Release\autotune_example.exe
) else (
    echo Example executable not found!
)

pause
