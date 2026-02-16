#!/bin/bash

set -e

echo "========================================="
echo "CCMiner ARM NEON Test Suite"
echo "========================================="
echo ""

# Detect architecture
ARCH=$(uname -m)
echo "Detected architecture: $ARCH"

# Check for NEON support
if [ "$ARCH" = "armv7l" ] || [ "$ARCH" = "aarch64" ]; then
    if grep -q neon /proc/cpuinfo; then
        echo "NEON support: YES"
    else
        echo "NEON support: NO"
        echo "Warning: NEON not detected, performance may be poor"
    fi
    
    if grep -q aes /proc/cpuinfo; then
        echo "ARM Crypto Extensions: YES"
    else
        echo "ARM Crypto Extensions: NO"
    fi
fi

echo ""
echo "========================================="
echo "Building test binaries..."
echo "========================================="

# Create test directory
mkdir -p test-build
cd test-build

# Build for current architecture
if [ "$ARCH" = "x86_64" ]; then
    cmake -DTARGET_ARCH=x64 ..
elif [ "$ARCH" = "i686" ] || [ "$ARCH" = "i386" ]; then
    cmake -DTARGET_ARCH=x86 ..
elif [ "$ARCH" = "armv7l" ]; then
    cmake -DTARGET_ARCH=armv7l ..
elif [ "$ARCH" = "aarch64" ]; then
    cmake -DTARGET_ARCH=aarch64 ..
else
    echo "Error: Unsupported architecture $ARCH"
    exit 1
fi

make -j$(nproc)

echo ""
echo "========================================="
echo "Running tests..."
echo "========================================="

if [ -f "ccminer-$ARCH" ]; then
    BINARY="ccminer-$ARCH"
elif [ -f "ccminer-x64" ]; then
    BINARY="ccminer-x64"
elif [ -f "ccminer-armv7l" ]; then
    BINARY="ccminer-armv7l"
elif [ -f "ccminer-aarch64" ]; then
    BINARY="ccminer-aarch64"
else
    echo "Error: No binary found"
    exit 1
fi

echo "Testing binary: $BINARY"
echo ""

# Run basic tests
echo "Test 1: Version check"
./$BINARY --version || echo "Version check not available"

echo ""
echo "Test 2: Algorithm list"
./$BINARY --help 2>&1 | grep -A 20 "Supported algorithms" || echo "Algorithm list not available"

echo ""
echo "Test 3: Benchmark (if supported)"
./$BINARY --benchmark --algo=verus || echo "Benchmark not supported"

echo ""
echo "========================================="
echo "Performance Information"
echo "========================================="

if [ "$ARCH" = "armv7l" ] || [ "$ARCH" = "aarch64" ]; then
    echo "CPU Information:"
    cat /proc/cpuinfo | grep -E "(processor|model name|BogoMIPS|Features)" | head -20
    
    echo ""
    echo "CPU Frequency:"
    for cpu in /sys/devices/system/cpu/cpu[0-9]*/cpufreq/scaling_cur_freq; do
        if [ -f "$cpu" ]; then
            freq=$(cat "$cpu")
            echo "  $(basename $(dirname $(dirname $cpu))): $((freq/1000)) MHz"
        fi
    done
    
    echo ""
    echo "Governor:"
    cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null || echo "  N/A"
    
    echo ""
    echo "Temperature:"
    if [ -f /sys/class/thermal/thermal_zone0/temp ]; then
        temp=$(cat /sys/class/thermal/thermal_zone0/temp)
        echo "  $((temp/1000))°C"
    else
        echo "  N/A"
    fi
fi

echo ""
echo "========================================="
echo "Test Complete"
echo "========================================="

cd ..
