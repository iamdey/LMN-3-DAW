# Build script for LMN-3-DAW using Docker containers (Windows PowerShell)
# Usage: .\build-docker.ps1 [amd64|armv7|arm64]

param(
    [Parameter(Position=0)]
    [ValidateSet("amd64", "armv7", "arm64")]
    [string]$Arch = "arm64"
)

$currentDir = Get-Location

switch ($Arch) {
    "amd64" {
        Write-Host "Building for x86_64 (amd64)..." -ForegroundColor Green
        docker run --rm -v "${currentDir}:/workspace" -w /workspace `
            iamdey/lmn-3-daw-docker-compiler:amd64 `
            bash -c "git submodule update --init --recursive && cmake -B build -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-fuse-ld=lld && cmake --build build -j8 && zip -r -j LMN-3-x86_64.zip ./build/LMN-3_artefacts/Release/LMN-3 ./LICENSE"

        if ($LASTEXITCODE -eq 0) {
            Write-Host "Build complete: LMN-3-x86_64.zip" -ForegroundColor Cyan
        } else {
            Write-Host "Build failed!" -ForegroundColor Red
            exit 1
        }
    }

    "armv7" {
        Write-Host "Building for ARM v7 (32-bit Raspberry Pi)..." -ForegroundColor Green
        docker run --rm -v "${currentDir}:/workspace" -w /workspace `
            iamdey/lmn-3-daw-docker-compiler:armv7 `
            bash -c "git submodule update --init --recursive && (cd tracktion_engine && patch -i ../patches_armv7/BufferedFileReader.patch -p1) && (cd tracktion_engine/modules/juce && patch -i ../../../patches_armv7/juce_dsp.h.patch -p1) && (cd tracktion_engine && patch -i ../patches_armv7/crill-21.patch -p1) && (cd tracktion_engine && patch -i ../patches_armv7/progressive_backoff_wait.h.patch -p1) && (cd tracktion_engine && patch -i ../patches_armv7/tracktion_CPU.patch -p1) && (cd tracktion_engine && patch -i ../patches_armv7/tracktion_NodePlayerUtilities.patch -p1) && cmake -B build -DCMAKE_BUILD_TYPE=Release -DPACKAGE_TESTS=OFF -DCMAKE_TOOLCHAIN_FILE=/toolchain/toolchain.cmake && cmake --build build -j8 && zip -r -j LMN-3-arm-linux-gnueabihf.zip ./build/LMN-3_artefacts/Release/LMN-3 ./LICENSE"

        if ($LASTEXITCODE -eq 0) {
            Write-Host "Build complete: LMN-3-arm-linux-gnueabihf.zip" -ForegroundColor Cyan
        } else {
            Write-Host "Build failed!" -ForegroundColor Red
            exit 1
        }
    }

    "arm64" {
        Write-Host "Building for ARM 64-bit (aarch64 Raspberry Pi)..." -ForegroundColor Green
        docker run --rm -v "${currentDir}:/workspace" -w /workspace `
            iamdey/lmn-3-daw-docker-compiler:arm64 `
            bash -c "git submodule update --init --recursive && cmake -B build -DCMAKE_BUILD_TYPE=Release -DPACKAGE_TESTS=OFF -DCMAKE_TOOLCHAIN_FILE=/toolchain/toolchain.cmake && cmake --build build -j8 && zip -r -j LMN-3-aarch64-linux-gnu.zip ./build/LMN-3_artefacts/Release/LMN-3 ./LICENSE"

        if ($LASTEXITCODE -eq 0) {
            Write-Host "Build complete: LMN-3-aarch64-linux-gnu.zip" -ForegroundColor Cyan
        } else {
            Write-Host "Build failed!" -ForegroundColor Red
            exit 1
        }
    }
}

Write-Host ""
Write-Host "Available architectures:" -ForegroundColor Yellow
Write-Host "  amd64  - x86_64 (standard PC)" -ForegroundColor Gray
Write-Host "  armv7  - 32-bit ARM (Raspberry Pi 3/4 32-bit OS)" -ForegroundColor Gray
Write-Host "  arm64  - 64-bit ARM (Raspberry Pi 3/4/5 64-bit OS)" -ForegroundColor Gray
