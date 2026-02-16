#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

print_usage() {
    echo "Usage: $0 [architecture]"
    echo ""
    echo "Architectures:"
    echo "  x64      - Build for x86_64 (Intel/AMD 64-bit)"
    echo "  x86      - Build for x86 (Intel/AMD 32-bit)"
    echo "  armv7l   - Build for ARMv7l 32-bit with NEON"
    echo "  armv9    - Build for ARMv9 32-bit with NEON"
    echo "  aarch64  - Build for ARM 64-bit"
    echo "  all      - Build for all architectures"
    echo "  auto     - Auto-detect and build for current architecture"
    echo ""
    echo "Example:"
    echo "  $0 armv7l    # Build for ARMv7l"
    echo "  $0 auto      # Auto-detect and build"
}

detect_arch() {
    local arch=$(uname -m)
    case "$arch" in
        x86_64)
            echo "x64"
            ;;
        i686|i386)
            echo "x86"
            ;;
        armv7l|armv7)
            echo "armv7l"
            ;;
        aarch64|arm64)
            if [ -n "$ARM_32BIT" ]; then
                echo "armv9"
            else
                echo "aarch64"
            fi
            ;;
        *)
            echo "unknown"
            ;;
    esac
}

build_arch() {
    local arch=$1
    echo "========================================="
    echo "Building for: $arch"
    echo "========================================="
    make -f Makefile.arm clean
    make -f Makefile.arm TARGET_ARCH=$arch -j$(nproc)
    echo ""
    echo "Build complete: ccminer-$arch"
    echo ""
}

if [ $# -eq 0 ]; then
    print_usage
    exit 1
fi

case "$1" in
    x64|x86|armv7l|armv9|aarch64)
        build_arch "$1"
        ;;
    all)
        for arch in x64 x86 armv7l armv9 aarch64; do
            build_arch "$arch" || echo "Warning: Failed to build $arch"
        done
        ;;
    auto)
        detected=$(detect_arch)
        if [ "$detected" = "unknown" ]; then
            echo "Error: Unable to detect architecture"
            exit 1
        fi
        echo "Detected architecture: $detected"
        build_arch "$detected"
        ;;
    -h|--help)
        print_usage
        exit 0
        ;;
    *)
        echo "Error: Unknown architecture '$1'"
        echo ""
        print_usage
        exit 1
        ;;
esac

echo ""
echo "========================================="
echo "Build Summary"
echo "========================================="
ls -lh ccminer-* 2>/dev/null || echo "No binaries found"
