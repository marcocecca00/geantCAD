#!/bin/bash
#
# Build VTK 9.3 with Qt6 support for GeantCAD
# Run this script to compile VTK with Qt6 bindings
#

set -e

VTK_VERSION="9.3.1"
VTK_DIR="$HOME/vtk-qt6"
BUILD_DIR="$VTK_DIR/build"
INSTALL_DIR="$VTK_DIR/install"
JOBS=$(nproc)

echo "============================================"
echo "VTK $VTK_VERSION with Qt6 Build Script"
echo "============================================"
echo ""
echo "This will:"
echo "  - Download VTK $VTK_VERSION"
echo "  - Build with Qt6 support"
echo "  - Install to $INSTALL_DIR"
echo ""
echo "Required packages:"
echo "  sudo apt install qt6-base-dev libqt6opengl6-dev libqt6openglwidgets6"
echo "  sudo apt install qt6-declarative-dev qml6-module-qtquick"
echo "  sudo apt install cmake ninja-build build-essential"
echo "  sudo apt install libgl1-mesa-dev libx11-dev"
echo ""

# Check dependencies
if ! command -v cmake &> /dev/null; then
    echo "ERROR: cmake not found. Install with: sudo apt install cmake"
    exit 1
fi

if ! dpkg -l | grep -q qt6-base-dev; then
    echo "ERROR: qt6-base-dev not found. Install with: sudo apt install qt6-base-dev"
    exit 1
fi

# Create directories
mkdir -p "$VTK_DIR"
cd "$VTK_DIR"

# Download VTK if not present
if [ ! -d "VTK-$VTK_VERSION" ]; then
    echo "Downloading VTK $VTK_VERSION..."
    wget -q --show-progress "https://www.vtk.org/files/release/9.3/VTK-$VTK_VERSION.tar.gz"
    echo "Extracting..."
    tar -xzf "VTK-$VTK_VERSION.tar.gz"
    rm "VTK-$VTK_VERSION.tar.gz"
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo ""
echo "Configuring VTK with Qt6..."
echo ""

cmake "../VTK-$VTK_VERSION" \
    -GNinja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
    -DVTK_QT_VERSION=6 \
    -DVTK_GROUP_ENABLE_Qt=YES \
    -DVTK_MODULE_ENABLE_VTK_GUISupportQt=YES \
    -DVTK_MODULE_ENABLE_VTK_GUISupportQtQuick=YES \
    -DVTK_MODULE_ENABLE_VTK_RenderingQt=YES \
    -DVTK_BUILD_TESTING=OFF \
    -DVTK_BUILD_EXAMPLES=OFF \
    -DBUILD_SHARED_LIBS=ON

echo ""
echo "Building VTK (this may take 15-30 minutes)..."
echo ""

ninja -j$JOBS

echo ""
echo "Installing VTK..."
echo ""

ninja install

echo ""
echo "============================================"
echo "VTK $VTK_VERSION with Qt6 installed successfully!"
echo "============================================"
echo ""
echo "Installation directory: $INSTALL_DIR"
echo ""
echo "To use with GeantCAD, configure CMake with:"
echo ""
echo "  cd /path/to/geantcad/build"
echo "  cmake .. -DCMAKE_PREFIX_PATH=$INSTALL_DIR -DGEANTCAD_PREFER_QT6=ON"
echo "  cmake --build ."
echo ""
echo "Or add to your environment:"
echo ""
echo "  export VTK_DIR=$INSTALL_DIR"
echo "  export CMAKE_PREFIX_PATH=\$VTK_DIR:\$CMAKE_PREFIX_PATH"
echo ""

