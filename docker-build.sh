#!/bin/bash
# LMN-3 Docker Build Script for Windows Git Bash
# Generates: LMN-3-aarch64-linux-gnu_0.7.0.zip

VERSION="0.7.0"
ZIP_NAME="LMN-3-aarch64-linux-gnu_${VERSION}.zip"

echo "========================================"
echo "LMN-3-DAW aarch64 Release Builder"
echo "Version: $VERSION"
echo "========================================"
echo ""

# Convert Windows path to Docker-compatible path
WORKSPACE_WIN="D:/DEV/LMN-3-DAW-iamdey-repo"
WORKSPACE_DOCKER="/d/DEV/LMN-3-DAW-iamdey-repo"

echo "[1/4] Checking Docker..."
if ! docker info >/dev/null 2>&1; then
    echo "ERROR: Docker is not running!"
    exit 1
fi
echo "OK: Docker is running"
echo ""

echo "[2/4] Cleaning previous build..."
cd "$WORKSPACE_WIN" || exit 1
rm -rf build
rm -f "$ZIP_NAME"
echo "OK: Cleaned"
echo ""

echo "[3/4] Building with Docker..."
echo "This will take 5-15 minutes, please wait..."
echo ""

# Use MSYS_NO_PATHCONV to prevent Git Bash path conversion
export MSYS_NO_PATHCONV=1

docker run --rm \
    -v "$WORKSPACE_DOCKER:/workspace" \
    -w /workspace \
    iamdey/lmn-3-daw-docker-compiler:arm64 \
    bash -c "
set -e
echo '==> Step 1/4: Initializing git submodules...'
git submodule update --init --recursive

echo ''
echo '==> Step 2/4: Configuring CMake...'
cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DPACKAGE_TESTS=OFF \
    -DCMAKE_TOOLCHAIN_FILE=/toolchain/toolchain.cmake

echo ''
echo '==> Step 3/4: Building (8 cores)...'
cmake --build build -j8

echo ''
echo '==> Step 4/4: Creating ZIP...'
cd build/LMN-3_artefacts/Release
zip -j ../../../$ZIP_NAME LMN-3
cd ../../../
zip -u $ZIP_NAME LICENSE

echo ''
echo '==> Build complete!'
ls -lh $ZIP_NAME
"

BUILD_EXIT=$?

if [ $BUILD_EXIT -ne 0 ]; then
    echo ""
    echo "========================================"
    echo "BUILD FAILED!"
    echo "========================================"
    exit 1
fi

echo ""
echo "[4/4] Verifying..."
if [ -f "$ZIP_NAME" ]; then
    echo "OK: $ZIP_NAME created"
    echo ""
    echo "Contents:"
    unzip -l "$ZIP_NAME"
    echo ""
    SIZE=$(du -h "$ZIP_NAME" | cut -f1)
    echo "Size: $SIZE"
else
    echo "ERROR: ZIP was not created!"
    exit 1
fi

echo ""
echo "========================================"
echo "BUILD SUCCESSFUL!"
echo "========================================"
echo ""
echo "Output: $ZIP_NAME"
echo ""
echo "Next steps:"
echo "1. scp $ZIP_NAME pi@raspberrypi:~"
echo "2. unzip $ZIP_NAME"
echo "3. chmod +x LMN-3"
echo "4. ./LMN-3"
echo ""
echo "New Features:"
echo "✓ FourOsc Arpeggiator (Tab 7: ARP)"
echo "✓ 5 modes: Up, Down, Up-Down, Random, Off"
echo "✓ Real-time MIDI injection"
echo ""
