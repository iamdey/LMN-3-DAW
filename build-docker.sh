#!/bin/bash
# Build script for LMN-3-DAW using Docker containers
# Usage: ./build-docker.sh [amd64|armv7|arm64]

ARCH=${1:-arm64}

case $ARCH in
    amd64)
        echo "Building for x86_64 (amd64)..."
        docker run --rm -v "$(pwd):/workspace" -w /workspace \
            iamdey/lmn-3-daw-docker-compiler:amd64 \
            bash -c "
                cmake -B build -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang \
                    -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-fuse-ld=lld && \
                cmake --build build -j8 && \
                zip -r -j LMN-3-x86_64.zip ./build/LMN-3_artefacts/Release/LMN-3 ./LICENSE
            "
        echo "Build complete: LMN-3-x86_64.zip"
        ;;

    armv7)
        echo "Building for ARM v7 (32-bit Raspberry Pi)..."
        echo "Applying ARM v7 patches..."

        # Apply patches
        (cd tracktion_engine && patch -i ../patches_armv7/BufferedFileReader.patch -p1)
        (cd tracktion_engine/modules/juce && patch -i ../../../patches_armv7/juce_dsp.h.patch -p1)
        (cd tracktion_engine && patch -i ../patches_armv7/crill-21.patch -p1)
        (cd tracktion_engine && patch -i ../patches_armv7/progressive_backoff_wait.h.patch -p1)
        (cd tracktion_engine && patch -i ../patches_armv7/tracktion_CPU.patch -p1)
        (cd tracktion_engine && patch -i ../patches_armv7/tracktion_NodePlayerUtilities.patch -p1)

        docker run --rm -v "$(pwd):/workspace" -w /workspace \
            iamdey/lmn-3-daw-docker-compiler:armv7 \
            bash -c "
                cmake -B build -DCMAKE_BUILD_TYPE=Release -DPACKAGE_TESTS=OFF \
                    -DCMAKE_TOOLCHAIN_FILE=/toolchain/toolchain.cmake && \
                cmake --build build -j8 && \
                zip -r -j LMN-3-arm-linux-gnueabihf.zip ./build/LMN-3_artefacts/Release/LMN-3 ./LICENSE
            "

        # Revert patches
        echo "Reverting patches..."
        git checkout tracktion_engine/

        echo "Build complete: LMN-3-arm-linux-gnueabihf.zip"
        ;;

    arm64)
        echo "Building for ARM 64-bit (aarch64 Raspberry Pi)..."
        docker run --rm -v "$(pwd):/workspace" -w /workspace \
            iamdey/lmn-3-daw-docker-compiler:arm64 \
            bash -c "
                cmake -B build -DCMAKE_BUILD_TYPE=Release -DPACKAGE_TESTS=OFF \
                    -DCMAKE_TOOLCHAIN_FILE=/toolchain/toolchain.cmake && \
                cmake --build build -j8 && \
                zip -r -j LMN-3-aarch64-linux-gnu.zip ./build/LMN-3_artefacts/Release/LMN-3 ./LICENSE
            "
        echo "Build complete: LMN-3-aarch64-linux-gnu.zip"
        ;;

    *)
        echo "Usage: $0 [amd64|armv7|arm64]"
        echo ""
        echo "Architectures:"
        echo "  amd64  - x86_64 (standard PC)"
        echo "  armv7  - 32-bit ARM (Raspberry Pi 3/4 32-bit OS)"
        echo "  arm64  - 64-bit ARM (Raspberry Pi 3/4/5 64-bit OS)"
        exit 1
        ;;
esac
