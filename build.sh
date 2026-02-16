#!/bin/bash
set -e

ARCH=$(uname -m)
echo "Detected: $ARCH"

case "$ARCH" in
    x86_64)
        echo "Building x86_64 AVX2/AES-NI"
        CFLAGS="-O3 -march=native -maes -mavx2 -mpclmul"
        ;;
    i686|i386)
        echo "Building x86 SSE4.1/AES-NI"
        CFLAGS="-O3 -march=native -maes -msse4.1 -mpclmul"
        ;;
    armv7l|armv8l)
        echo "Building ARM NEON"
        CFLAGS="-O3 -march=armv7-a -mfpu=neon -mfloat-abi=hard"
        if gcc -march=armv7-a+crypto -E - </dev/null 2>&1 | grep -q crypto; then
            CFLAGS="$CFLAGS -march=armv7-a+crypto"
        fi
        export ARM_NEON=1
        ;;
    aarch64|arm64)
        echo "Building ARM64"
        CFLAGS="-O3 -march=armv8-a+crypto"
        export ARM_NEON=1
        ;;
esac

export CFLAGS CXXFLAGS="$CFLAGS"
[ ! -f configure ] && ./autogen.sh
./configure --with-curl
make -j$(nproc)
echo "Done: ./ccminer"
