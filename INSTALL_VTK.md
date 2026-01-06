# Installing VTK for GeantCAD

## Quick Install (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install libvtk9-dev libvtk9-qt-dev
```

This should install VTK 9 with Qt6 support.

## Verify Installation

After installation, verify VTK is found:

```bash
# Check if VTK libraries are installed
dpkg -l | grep vtk9

# Check if Qt6 support is available
ldconfig -p | grep vtk
```

## Rebuild GeantCAD

After installing VTK, rebuild the project:

```bash
cd build
rm -rf *
cmake ..
cmake --build . -j$(nproc)
```

The viewport should now work with 3D rendering!

## Troubleshooting

If CMake still doesn't find VTK:
1. Make sure `libvtk9-dev` and `libvtk9-qt-dev` are installed
2. Check that Qt6 development packages are installed (see INSTALL_QT6.md)
3. Try: `sudo ldconfig` to update library cache

If viewport shows errors:
- Check that OpenGL is working: `glxinfo | grep "OpenGL"`
- Verify Qt6 OpenGL support is installed

