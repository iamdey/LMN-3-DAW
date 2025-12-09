@echo off
REM Build script for LMN-3-DAW aarch64 Release
REM Generates: LMN-3-aarch64-linux-gnu_0.6.2.zip

set VERSION=0.6.2
set ZIP_NAME=LMN-3-aarch64-linux-gnu_%VERSION%.zip

echo ========================================
echo LMN-3-DAW aarch64 Release Builder
echo Version: %VERSION%
echo ========================================
echo.

echo [1/4] Checking Docker...
docker info >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Docker is not running!
    pause
    exit /b 1
)
echo OK: Docker is running
echo.

echo [2/4] Cleaning previous build...
if exist build rmdir /s /q build
if exist %ZIP_NAME% del /f %ZIP_NAME%
echo OK: Cleaned
echo.

echo [3/4] Building with Docker (5-15 minutes)...
echo This will take a while, please wait...
echo.

docker run --rm -v "%CD%:/workspace" -w /workspace lmn3-builder bash -c "set -e && echo '==> Initializing submodules...' && git submodule update --init --recursive && echo '==> Configuring CMake...' && cmake -B build -DCMAKE_BUILD_TYPE=Release -DPACKAGE_TESTS=OFF -DCMAKE_TOOLCHAIN_FILE=/toolchain/toolchain.cmake && echo '==> Building...' && cmake --build build -j8 && echo '==> Creating ZIP...' && cd build/LMN-3_artefacts/Release && zip -j ../../../%ZIP_NAME% LMN-3 && cd ../../../ && zip -u %ZIP_NAME% LICENSE && echo '==> Done!'"

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ========================================
    echo BUILD FAILED!
    echo ========================================
    pause
    exit /b 1
)

echo.
echo [4/4] Verifying...
if exist %ZIP_NAME% (
    echo OK: %ZIP_NAME% created successfully
    docker run --rm -v "%CD%:/workspace" -w /workspace lmn3-builder bash -c "unzip -l %ZIP_NAME%"
) else (
    echo ERROR: ZIP was not created
    pause
    exit /b 1
)

echo.
echo ========================================
echo BUILD SUCCESSFUL!
echo ========================================
echo.
echo Output: %ZIP_NAME%
echo.
echo Next steps:
echo 1. Copy to Raspberry Pi: scp %ZIP_NAME% pi@raspberrypi:~
echo 2. Extract: unzip %ZIP_NAME%
echo 3. Run: ./LMN-3
echo.
echo Features:
echo - FourOsc Arpeggiator (Tab 7: ARP)
echo - 5 modes: Up, Down, Up-Down, Random, Off
echo - Real-time MIDI injection
echo.
pause
