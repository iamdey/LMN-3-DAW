# Simple build script for LMN-3-DAW using local lmn3-builder Docker image
# Run this with: powershell -ExecutionPolicy Bypass -File build-local-simple.ps1

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Building LMN-3-DAW for Raspberry Pi" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$workspace = Get-Location
Write-Host "Workspace: $workspace" -ForegroundColor Yellow
Write-Host ""

# Full build command in one go
Write-Host "Starting Docker build (this will take several minutes)..." -ForegroundColor Green
Write-Host ""

docker run --rm -v "${workspace}:/workspace" -w /workspace lmn3-builder bash -c @"
set -e
echo '==> Initializing git submodules...'
git submodule update --init --recursive

echo '==> Configuring CMake...'
cmake -B build -DCMAKE_BUILD_TYPE=Release -DPACKAGE_TESTS=OFF -DCMAKE_TOOLCHAIN_FILE=/toolchain/toolchain.cmake

echo '==> Building (using 8 cores)...'
cmake --build build -j8

echo '==> Creating ZIP package...'
zip -r -j LMN-3-aarch64-linux-gnu.zip ./build/LMN-3_artefacts/Release/LMN-3 ./LICENSE

echo '==> Build complete!'
"@

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "BUILD SUCCESSFUL!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Output file: LMN-3-aarch64-linux-gnu.zip" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Next steps:" -ForegroundColor Yellow
    Write-Host "1. Copy LMN-3-aarch64-linux-gnu.zip to your Raspberry Pi" -ForegroundColor Gray
    Write-Host "2. Extract: unzip LMN-3-aarch64-linux-gnu.zip" -ForegroundColor Gray
    Write-Host "3. Run: ./LMN-3" -ForegroundColor Gray
} else {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Red
    Write-Host "BUILD FAILED!" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Red
    Write-Host ""
    Write-Host "Check the error messages above." -ForegroundColor Yellow
    exit 1
}
