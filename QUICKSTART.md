# Quick Start Guide - Tekla-Like CAD

## Prerequisites Installation (Quick Version)

### 1. Visual Studio 2019/2022
Download and install with "Desktop development with C++"

### 2. CMake
```powershell
winget install Kitware.CMake
```

### 3. Qt6
- Install from: https://www.qt.io/download
- Choose: Qt 6.5.0 MSVC 2019 64-bit
- Location: `C:\Qt\6.5.0\msvc2019_64`

### 4. OpenCascade
- Download from: https://dev.opencascade.org/release
- Extract to: `C:\OpenCASCADE-7.7.0`

## Quick Environment Setup

Run in PowerShell (Admin):
```powershell
# Set environment variables
$env:Qt6_DIR="C:\Qt\6.5.0\msvc2019_64\lib\cmake\Qt6"
$env:CASROOT="C:\OpenCASCADE-7.7.0"
$env:PATH+=";C:\Qt\6.5.0\msvc2019_64\bin;C:\OpenCASCADE-7.7.0\win64\vc14\bin"
```

## Build and Run

```powershell
# Navigate to project
cd C:\Users\lenovo\Documents\SOFTWARE_WORKSHOP\OCC

# Build
.\build.ps1

# Run
cd build\bin\Release
.\TeklaLikeCAD.exe
```

## Basic Usage

### Keyboard Shortcuts
- `F` - Fit all objects in view
- `Ctrl+N` - New project
- `Ctrl+O` - Open project
- `Ctrl+S` - Save project
- `Ctrl+Q` - Exit application
- `Delete` - Delete selected objects

### Mouse Controls
- **Left Click + Drag** - Rotate view
- **Middle Click + Drag** - Pan view  
- **Mouse Wheel** - Zoom in/out
- **Right Click** - Select object

### Creating Elements

**Menu: Create → [Element Type]**
- Beam: 5000 × 300 × 400 mm
- Column: 300 × 300 × 3000 mm
- Slab: 5000 × 4000 × 200 mm
- Wall: 5000 × 200 × 3000 mm
- Foundation: 2000 × 2000 × 500 mm

### View Options

**Menu: View → [View Type]**
- Top View
- Front View
- Right View
- Isometric View (default)

## File Structure

```
OCC/
├── CMakeLists.txt       # Build configuration
├── build.ps1            # Build script
├── README.md            # Full documentation
├── SETUP.md             # Detailed setup guide
├── QUICKSTART.md        # This file
├── include/             # Header files
│   ├── MainWindow.h
│   ├── OCCTViewer.h
│   └── GeometryBuilder.h
└── src/                 # Source files
    ├── main.cpp
    ├── MainWindow.cpp
    ├── OCCTViewer.cpp
    └── GeometryBuilder.cpp
```

## Troubleshooting Quick Fixes

### Qt not found
```powershell
cmake .. -DQt6_DIR="C:/Qt/6.5.0/msvc2019_64/lib/cmake/Qt6"
```

### OpenCascade not found
Edit `CMakeLists.txt` line 18 with correct path

### Missing DLLs at runtime
```powershell
cd build\bin\Release
windeployqt TeklaLikeCAD.exe
```

### Clean rebuild
```powershell
Remove-Item -Recurse -Force build
.\build.ps1
```

## Customization Quick Tips

### Change Element Dimensions
Edit `src/MainWindow.cpp`, find functions like:
```cpp
void MainWindow::onCreateBeam() {
    GeometryBuilder builder(m_viewer->getContext());
    builder.createBeam(0, 0, 0, 5000, 300, 400);  // Modify these numbers
}
```

### Change Colors
Edit `src/GeometryBuilder.cpp`, modify `Quantity_Color` parameters:
```cpp
// Available colors:
Quantity_NOC_STEEL, Quantity_NOC_GRAY, Quantity_NOC_LIGHTGRAY,
Quantity_NOC_BEIGE, Quantity_NOC_BLUE1, Quantity_NOC_RED, etc.
```

### Add New Menu Item
1. Declare action in `include/MainWindow.h`
2. Create action in `src/MainWindow.cpp` → `createActions()`
3. Add to menu in `createMenus()`
4. Implement slot function

## Next Steps

1. ✅ Build and run the application
2. ✅ Create some test elements
3. ✅ Explore the 3D viewer
4. 📝 Modify element dimensions
5. 📝 Add custom colors
6. 📝 Implement new element types
7. 📝 Add save/load functionality

## Resources

- **Full Documentation**: See `README.md`
- **Setup Guide**: See `SETUP.md`
- **Qt Docs**: https://doc.qt.io/qt-6/
- **OpenCascade Docs**: https://dev.opencascade.org/doc/

## Support

For issues:
1. Check `SETUP.md` troubleshooting section
2. Verify all prerequisites are installed correctly
3. Ensure environment variables are set
4. Try clean rebuild

---

**Happy Coding! 🏗️**
