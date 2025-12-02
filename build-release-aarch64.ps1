# Build script for LMN-3-DAW - aarch64 Release with Arpeggiator
# Generates: LMN-3-aarch64-linux-gnu_X.X.X.zip

param(
    [string]$Version = "0.7.0"
)

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "LMN-3-DAW aarch64 Release Builder" -ForegroundColor Cyan
Write-Host "Version: $Version" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$workspace = Get-Location
$zipName = "LMN-3-aarch64-linux-gnu_$Version.zip"

Write-Host "Workspace: $workspace" -ForegroundColor Yellow
Write-Host "Output: $zipName" -ForegroundColor Yellow
Write-Host ""

# Check if Docker is running
Write-Host "[1/5] Checking Docker..." -ForegroundColor Green
docker info > $null 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Docker is not running!" -ForegroundColor Red
    Write-Host "Please start Docker Desktop and try again." -ForegroundColor Yellow
    exit 1
}
Write-Host "✓ Docker is running" -ForegroundColor Green
Write-Host ""

# Check if lmn3-builder image exists
Write-Host "[2/5] Checking Docker image..." -ForegroundColor Green
docker images lmn3-builder --format "{{.Repository}}" | Select-String -Pattern "lmn3-builder" -Quiet
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Docker image 'lmn3-builder' not found!" -ForegroundColor Red
    Write-Host "Available images:" -ForegroundColor Yellow
    docker images
    exit 1
}
Write-Host "✓ Image 'lmn3-builder' found" -ForegroundColor Green
Write-Host ""

# Clean previous build
Write-Host "[3/5] Cleaning previous build..." -ForegroundColor Green
if (Test-Path "build") {
    Remove-Item -Recurse -Force "build" -ErrorAction SilentlyContinue
    Write-Host "✓ Cleaned build directory" -ForegroundColor Green
} else {
    Write-Host "✓ No previous build found" -ForegroundColor Green
}

if (Test-Path $zipName) {
    Remove-Item -Force $zipName
    Write-Host "✓ Removed old $zipName" -ForegroundColor Green
}
Write-Host ""

# Run Docker build
Write-Host "[4/5] Building with Docker (this may take 5-15 minutes)..." -ForegroundColor Green
Write-Host "Container: lmn3-builder" -ForegroundColor Gray
Write-Host "Target: aarch64 (Raspberry Pi 64-bit)" -ForegroundColor Gray
Write-Host ""

$buildCommand = @"
set -e
echo '==> Step 1/4: Initializing git submodules...'
git submodule update --init --recursive

echo ''
echo '==> Step 2/4: Configuring CMake...'
cmake -B build -DCMAKE_BUILD_TYPE=Release -DPACKAGE_TESTS=OFF -DCMAKE_TOOLCHAIN_FILE=/toolchain/toolchain.cmake

echo ''
echo '==> Step 3/4: Building (using 8 cores)...'
cmake --build build -j8

echo ''
echo '==> Step 4/4: Creating ZIP package...'
cd build/LMN-3_artefacts/Release
zip -j ../../../$zipName LMN-3
cd ../../../
zip -u $zipName LICENSE

echo ''
echo '==> Build complete!'
"@

docker run --rm -v "${workspace}:/workspace" -w /workspace lmn3-builder bash -c $buildCommand

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Red
    Write-Host "BUILD FAILED!" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Red
    Write-Host ""
    Write-Host "Check the error messages above for details." -ForegroundColor Yellow
    exit 1
}

Write-Host ""
Write-Host "[5/5] Verifying build..." -ForegroundColor Green

if (Test-Path $zipName) {
    $fileSize = (Get-Item $zipName).Length / 1MB
    Write-Host "✓ ZIP created: $zipName" -ForegroundColor Green
    Write-Host "  Size: $([math]::Round($fileSize, 2)) MB" -ForegroundColor Gray

    Write-Host ""
    Write-Host "Checking ZIP contents..." -ForegroundColor Gray
    docker run --rm -v "${workspace}:/workspace" -w /workspace lmn3-builder bash -c "unzip -l $zipName"
} else {
    Write-Host "ERROR: ZIP file was not created!" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "BUILD SUCCESSFUL!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Output: $zipName" -ForegroundColor Cyan
Write-Host "Size: $([math]::Round($fileSize, 2)) MB" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "1. Copy $zipName to your Raspberry Pi" -ForegroundColor Gray
Write-Host "   scp $zipName pi@raspberrypi:~" -ForegroundColor Gray
Write-Host ""
Write-Host "2. On Raspberry Pi, extract and run:" -ForegroundColor Gray
Write-Host "   unzip $zipName" -ForegroundColor Gray
Write-Host "   chmod +x LMN-3" -ForegroundColor Gray
Write-Host "   ./LMN-3" -ForegroundColor Gray
Write-Host ""
Write-Host "Features included:" -ForegroundColor Yellow
Write-Host "✓ FourOsc Synthesizer with Arpeggiator (Tab 7: ARP)" -ForegroundColor Green
Write-Host "✓ Real-time MIDI injection" -ForegroundColor Green
Write-Host "✓ 5 arpeggio modes (Up, Down, Up-Down, Random, Off)" -ForegroundColor Green
Write-Host "✓ Adjustable Rate, Octaves, and Gate" -ForegroundColor Green
Write-Host ""
