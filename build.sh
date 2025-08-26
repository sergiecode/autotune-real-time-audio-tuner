#!/bin/bash

echo "AutoTune Real-Time Audio Engine - Build Script"
echo "============================================="

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

cd build

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake not found!"
    echo "Please install CMake:"
    echo "  Ubuntu/Debian: sudo apt-get install cmake"
    echo "  macOS:         brew install cmake"
    exit 1
fi

# Check for compiler
if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo "ERROR: No C++ compiler found!"
    echo "Please install a C++ compiler:"
    echo "  Ubuntu/Debian: sudo apt-get install build-essential"
    echo "  macOS:         xcode-select --install"
    exit 1
fi

echo "Configuring project with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release
if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed!"
    exit 1
fi

echo "Building project..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
if [ $? -ne 0 ]; then
    echo "ERROR: Build failed!"
    exit 1
fi

echo ""
echo "============================================="
echo "Build completed successfully!"
echo ""
echo "You can now run:"
echo "  - ./autotune_example  (Example application)"
echo "  - ./autotune_tests    (Test suite)"
echo ""

# Ask user if they want to run the example
read -p "Run the example application? (y/n): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    if [ -f "./autotune_example" ]; then
        echo "Running example application..."
        ./autotune_example
    else
        echo "Example executable not found!"
    fi
fi
