#!/bin/bash
# Build script for betting_by_time_cxx

set -e

echo "=== Building Betting-by-Time C++20 Implementation ==="
echo ""

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: CMakeLists.txt not found. Please run this script from the betting_by_time_cxx directory."
    exit 1
fi

# Create build directory
BUILD_DIR="build"
if [ -d "$BUILD_DIR" ]; then
    echo "Removing old cmake cache..."
    rm -rf "$BUILD_DIR"/CMakeCache.txt
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -G Ninja \
         -DCMAKE_INSTALL_PREFIX=../../dist \
         -DBUILD_SHARED_LIBS=OFF

# Build
echo ""
echo "Building..."
cmake --build . -j$(nproc)

# Install
echo ""
echo "Installing to ../dist..."
cmake --install .

echo ""
echo "=== Build Complete ==="
echo ""
echo "Library installed to ../dist/"
echo "  Headers: ../dist/include/betting_by_time/"
echo "  Library: ../dist/lib/"
echo ""
echo "To run tests (when implemented):"
echo "  ctest"
echo ""
