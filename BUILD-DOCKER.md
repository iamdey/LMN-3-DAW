# Building LMN-3-DAW with Docker

This guide explains how to build LMN-3-DAW for Raspberry Pi (and other platforms) using Docker containers.

## Prerequisites

- Docker installed and running
- Git (for cloning the repository)

**Note:** You don't need to install build tools, compilers, or initialize git submodules manually. The Docker containers handle everything!

## Quick Start

### On Linux/macOS:

```bash
# Make the script executable
chmod +x build-docker.sh

# Build for Raspberry Pi 64-bit (aarch64)
./build-docker.sh arm64

# Build for Raspberry Pi 32-bit (armv7)
./build-docker.sh armv7

# Build for x86_64 PC
./build-docker.sh amd64
```

### On Windows (PowerShell):

```powershell
# Build for Raspberry Pi 64-bit (aarch64)
.\build-docker.ps1 arm64

# Build for Raspberry Pi 32-bit (armv7)
.\build-docker.ps1 armv7

# Build for x86_64 PC
.\build-docker.ps1 amd64
```

## Available Architectures

| Architecture | Target Platform | Output File |
|--------------|----------------|-------------|
| `arm64` | Raspberry Pi 3/4/5 (64-bit OS) | `LMN-3-aarch64-linux-gnu.zip` |
| `armv7` | Raspberry Pi 3/4 (32-bit OS) | `LMN-3-arm-linux-gnueabihf.zip` |
| `amd64` | Standard PC (x86_64) | `LMN-3-x86_64.zip` |

## What the Build Does

1. **Pulls the appropriate Docker image** containing:
   - Clang/LLVM compiler
   - CMake build system
   - Cross-compilation toolchain (for ARM builds)
   - All required dependencies

2. **Initializes git submodules** (JUCE, Tracktion Engine, yaml-cpp)

3. **Applies patches** (for armv7 builds only):
   - ARM-specific compatibility fixes
   - Performance optimizations

4. **Compiles the project**:
   - Configures CMake with the correct toolchain
   - Builds the LMN-3 executable
   - Creates a ZIP file with the binary and LICENSE

## Docker Images

The build uses pre-configured Docker images hosted on Docker Hub:

- `iamdey/lmn-3-daw-docker-compiler:amd64` - x86_64 build environment
- `iamdey/lmn-3-daw-docker-compiler:armv7` - ARMv7 cross-compilation
- `iamdey/lmn-3-daw-docker-compiler:arm64` - AArch64 cross-compilation

## Manual Docker Build (Advanced)

If you prefer to run Docker commands directly:

### For ARM64 (Raspberry Pi 64-bit):

```bash
docker run --rm -v "$(pwd):/workspace" -w /workspace \
    iamdey/lmn-3-daw-docker-compiler:arm64 \
    bash -c "git submodule update --init --recursive && \
             cmake -B build -DCMAKE_BUILD_TYPE=Release -DPACKAGE_TESTS=OFF \
                   -DCMAKE_TOOLCHAIN_FILE=/toolchain/toolchain.cmake && \
             cmake --build build -j8 && \
             zip -r -j LMN-3-aarch64-linux-gnu.zip \
                   ./build/LMN-3_artefacts/Release/LMN-3 ./LICENSE"
```

### For ARMv7 (Raspberry Pi 32-bit):

```bash
docker run --rm -v "$(pwd):/workspace" -w /workspace \
    iamdey/lmn-3-daw-docker-compiler:armv7 \
    bash -c "git submodule update --init --recursive && \
             (cd tracktion_engine && patch -i ../patches_armv7/BufferedFileReader.patch -p1) && \
             (cd tracktion_engine/modules/juce && patch -i ../../../patches_armv7/juce_dsp.h.patch -p1) && \
             (cd tracktion_engine && patch -i ../patches_armv7/crill-21.patch -p1) && \
             (cd tracktion_engine && patch -i ../patches_armv7/progressive_backoff_wait.h.patch -p1) && \
             (cd tracktion_engine && patch -i ../patches_armv7/tracktion_CPU.patch -p1) && \
             (cd tracktion_engine && patch -i ../patches_armv7/tracktion_NodePlayerUtilities.patch -p1) && \
             cmake -B build -DCMAKE_BUILD_TYPE=Release -DPACKAGE_TESTS=OFF \
                   -DCMAKE_TOOLCHAIN_FILE=/toolchain/toolchain.cmake && \
             cmake --build build -j8 && \
             zip -r -j LMN-3-arm-linux-gnueabihf.zip \
                   ./build/LMN-3_artefacts/Release/LMN-3 ./LICENSE"
```

### For x86_64:

```bash
docker run --rm -v "$(pwd):/workspace" -w /workspace \
    iamdey/lmn-3-daw-docker-compiler:amd64 \
    bash -c "git submodule update --init --recursive && \
             cmake -B build -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang \
                   -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-fuse-ld=lld && \
             cmake --build build -j8 && \
             zip -r -j LMN-3-x86_64.zip \
                   ./build/LMN-3_artefacts/Release/LMN-3 ./LICENSE"
```

## Troubleshooting

### Docker Permission Issues (Linux)

If you get permission errors:

```bash
# Add your user to the docker group
sudo usermod -aG docker $USER

# Log out and back in for changes to take effect
```

### Build Fails with "No space left on device"

Docker may need more disk space:

```bash
# Clean up unused Docker images/containers
docker system prune -a
```

### Submodule Clone Failures

If submodules fail to clone inside Docker, ensure:
- You have a stable internet connection
- GitHub is accessible from your network
- No firewall blocking git protocols

## CI/CD Integration

This project uses GitHub Actions for automated builds. Check `.github/workflows/release.yaml` to see how the Docker builds are configured in CI.

## Output Files

After a successful build, you'll find:

- **ZIP file** in the project root directory
- **Executable** in `./build/LMN-3_artefacts/Release/LMN-3`

The ZIP contains:
- `LMN-3` - The compiled executable
- `LICENSE` - GPL-3.0 license file

## Next Steps

After building:

1. Extract the ZIP file
2. Copy the `LMN-3` executable to your target device
3. Make it executable: `chmod +x LMN-3`
4. Run: `./LMN-3`

For Raspberry Pi deployment, see the main README.md for system requirements and setup instructions.
