@echo off
REM Build script for LMN-3-DAW using local Docker image (lmn3-builder)
REM This script is designed for Windows

echo ========================================
echo Building LMN-3-DAW for Raspberry Pi (aarch64)
echo Using Docker image: lmn3-builder
echo ========================================
echo.

REM Get the current directory
set WORKSPACE=%CD%

echo Workspace: %WORKSPACE%
echo.

echo Step 1: Initializing git submodules...
docker run --rm -v "%WORKSPACE%:/workspace" -w /workspace lmn3-builder bash -c "git submodule update --init --recursive"

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to initialize submodules
    pause
    exit /b 1
)

echo.
echo Step 2: Configuring CMake...
docker run --rm -v "%WORKSPACE%:/workspace" -w /workspace lmn3-builder bash -c "cmake -B build -DCMAKE_BUILD_TYPE=Release -DPACKAGE_TESTS=OFF -DCMAKE_TOOLCHAIN_FILE=/toolchain/toolchain.cmake"

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configuration failed
    pause
    exit /b 1
)

echo.
echo Step 3: Building (this may take several minutes)...
docker run --rm -v "%WORKSPACE%:/workspace" -w /workspace lmn3-builder bash -c "cmake --build build -j8"

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed
    pause
    exit /b 1
)

echo.
echo Step 4: Creating ZIP package...
docker run --rm -v "%WORKSPACE%:/workspace" -w /workspace lmn3-builder bash -c "zip -r -j LMN-3-aarch64-linux-gnu.zip ./build/LMN-3_artefacts/Release/LMN-3 ./LICENSE"

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: ZIP creation failed
    pause
    exit /b 1
)

echo.
echo ========================================
echo BUILD SUCCESSFUL!
echo ========================================
echo Output: %WORKSPACE%\LMN-3-aarch64-linux-gnu.zip
echo.
echo You can now copy this file to your Raspberry Pi.
echo.
pause
