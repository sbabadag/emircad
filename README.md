# Tekla-Like 3D Structural CAD Application

A 3D structural engineering CAD application similar to Tekla Structures, built with C++, Qt6, and OpenCascade Technology (OCCT).

## Features

- **3D Visualization**: Real-time 3D viewport with OpenCascade rendering
- **Structural Elements**: Create beams, columns, slabs, walls, and foundations
- **Multiple Views**: Top, Front, Right, and Isometric views
- **Interactive Navigation**: Rotate, pan, and zoom in 3D space
- **Project Management**: Project tree and properties panel
- **Modern UI**: Qt-based interface with toolbars and menus
- **Export Capabilities**: Export to STEP, IGES, and STL formats

## Prerequisites

Before building this application, you need to install the following:

### 1. CMake
- Download from: https://cmake.org/download/
- Version 3.16 or higher
- Add to PATH during installation

### 2. Qt6
- Download from: https://www.qt.io/download
- Install Qt 6.5 or higher
- Required modules: Core, Gui, Widgets, OpenGL, OpenGLWidgets
- Add Qt bin directory to PATH (e.g., `C:\Qt\6.5.0\msvc2019_64\bin`)

### 3. OpenCascade Technology (OCCT)
- Download from: https://dev.opencascade.org/release
- Version 7.7.0 or higher
- Installation options:
  - **Windows**: Download pre-built binaries or build from source
  - Install to: `C:\OpenCASCADE-7.7.0\`
- Required modules: Foundation, Modeling, Visualization

### 4. Visual Studio (Windows)
- Visual Studio 2019 or 2022
- Install "Desktop development with C++" workload
- Ensure C++17 support is installed

### 5. Compiler (Alternative)
- MinGW-w64 (if not using Visual Studio)
- GCC 7+ or Clang 5+

## Installation Instructions

### Step 1: Install OpenCascade

**Option A: Pre-built Binaries (Recommended for Windows)**
1. Download OCCT from https://dev.opencascade.org/release
2. Extract to `C:\OpenCASCADE-7.7.0\`
3. Set environment variables:
   ```
   CASROOT=C:\OpenCASCADE-7.7.0
   ```
4. Add to PATH:
   ```
   C:\OpenCASCADE-7.7.0\win64\vc14\bin
   ```

**Option B: Build from Source**
```powershell
git clone https://git.dev.opencascade.org/repos/occt.git
cd occt
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
cmake --install . --prefix C:\OpenCASCADE-7.7.0
```

### Step 2: Install Qt6

1. Download Qt Online Installer from https://www.qt.io/download
2. Install Qt 6.5.0 or higher with MSVC 2019 64-bit
3. Add Qt to PATH:
   ```
   C:\Qt\6.5.0\msvc2019_64\bin
   ```
4. Set Qt6_DIR environment variable:
   ```
   Qt6_DIR=C:\Qt\6.5.0\msvc2019_64\lib\cmake\Qt6
   ```

### Step 3: Configure the Project

1. Open `CMakeLists.txt` and verify the OpenCascade path:
   ```cmake
   set(OpenCASCADE_DIR "C:/OpenCASCADE-7.7.0/cmake" CACHE PATH "Path to OpenCASCADE CMake directory")
   ```
   
2. Adjust the path if your OpenCascade installation is different.

## Building the Project

### Using Visual Studio (Recommended)

1. Open PowerShell in the project directory
2. Create build directory:
   ```powershell
   mkdir build
   cd build
   ```

3. Generate Visual Studio solution:
   ```powershell
   cmake .. -G "Visual Studio 16 2019" -A x64
   ```
   Or for VS 2022:
   ```powershell
   cmake .. -G "Visual Studio 17 2022" -A x64
   ```

4. Build the project:
   ```powershell
   cmake --build . --config Release
   ```

5. Run the application:
   ```powershell
   .\bin\Release\TeklaLikeCAD.exe
   ```

### Using CMake and Make

```powershell
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
.\bin\TeklaLikeCAD.exe
```

## Project Structure

```
OCC/
├── CMakeLists.txt          # CMake configuration
├── README.md               # This file
├── build/                  # Build directory (generated)
├── include/                # Header files
│   ├── MainWindow.h        # Main application window
│   ├── OCCTViewer.h        # OpenCascade 3D viewer widget
│   └── GeometryBuilder.h   # Geometry creation utilities
├── src/                    # Source files
│   ├── main.cpp            # Application entry point
│   ├── MainWindow.cpp      # Main window implementation
│   ├── OCCTViewer.cpp      # 3D viewer implementation
│   └── GeometryBuilder.cpp # Geometry creation implementation
└── resources/              # Resources (icons, etc.)
```

## Usage Guide

### Creating Structural Elements

1. **Create a Beam**:
   - Click "Create" → "Create Beam" or use the toolbar button
   - Default dimensions: 5000mm length, 300mm width, 400mm height

2. **Create a Column**:
   - Click "Create" → "Create Column"
   - Default dimensions: 300mm × 300mm × 3000mm height

3. **Create a Slab**:
   - Click "Create" → "Create Slab"
   - Default dimensions: 5000mm × 4000mm × 200mm thickness

4. **Create a Wall**:
   - Click "Create" → "Create Wall"
   - Default dimensions: 5000mm length × 200mm thickness × 3000mm height

5. **Create a Foundation**:
   - Click "Create" → "Create Foundation"
   - Default dimensions: 2000mm × 2000mm × 500mm height

### Navigation Controls

- **Rotate View**: Left mouse button + drag
- **Pan View**: Middle mouse button + drag
- **Zoom**: Mouse wheel scroll
- **Select Object**: Right mouse button click
- **Fit All**: Press 'F' key or View → Fit All

### View Modes

- **Top View**: View → Top View
- **Front View**: View → Front View
- **Right View**: View → Right View
- **Isometric View**: View → Isometric View (default)

## Troubleshooting

### CMake Cannot Find Qt6

Set the Qt6_DIR environment variable:
```powershell
$env:Qt6_DIR="C:\Qt\6.5.0\msvc2019_64\lib\cmake\Qt6"
```

### CMake Cannot Find OpenCascade

Edit `CMakeLists.txt` and update the OpenCASCADE_DIR path:
```cmake
set(OpenCASCADE_DIR "C:/YourPath/OpenCASCADE-7.7.0/cmake")
```

### Runtime Error: Missing DLLs

Add these directories to your PATH:
- Qt bin directory: `C:\Qt\6.5.0\msvc2019_64\bin`
- OpenCascade bin directory: `C:\OpenCASCADE-7.7.0\win64\vc14\bin`

### Black Screen in Viewer

Ensure you have:
- Updated graphics drivers
- OpenGL 3.3+ support
- Proper Qt OpenGL modules installed

## Development

### Adding New Structural Elements

1. Add method declaration in `GeometryBuilder.h`
2. Implement in `GeometryBuilder.cpp`
3. Add menu action in `MainWindow.h`
4. Connect action in `MainWindow.cpp`

Example:
```cpp
// In GeometryBuilder.h
Handle(AIS_Shape) createNewElement(double x, double y, double z, ...);

// In MainWindow.cpp
void MainWindow::onCreateNewElement() {
    GeometryBuilder builder(m_viewer->getContext());
    builder.createNewElement(0, 0, 0, ...);
    m_viewer->fitAll();
}
```

### Customizing Colors

Colors are defined using OpenCascade's `Quantity_Color`:
- `Quantity_NOC_STEEL` - Steel gray
- `Quantity_NOC_GRAY` - Gray
- `Quantity_NOC_LIGHTGRAY` - Light gray
- `Quantity_NOC_BEIGE` - Beige

## Future Enhancements

- [ ] Property editing for created elements
- [ ] Save/Load project functionality
- [ ] STEP/IGES file import
- [ ] Dimension annotations
- [ ] Interference checking
- [ ] Material properties
- [ ] Steel profiles library (IPE, HEA, etc.)
- [ ] Parametric modeling
- [ ] BIM integration

## Dependencies

- **Qt 6.5+**: UI framework
- **OpenCascade 7.7+**: 3D geometry kernel
- **CMake 3.16+**: Build system
- **C++17**: Language standard

## License

This is a demonstration project for educational purposes.

## Contributing

This is a template project. Feel free to extend it with additional features:
- Add more structural element types
- Implement property editing
- Add file I/O capabilities
- Create a steel profiles library
- Implement snapping and constraints

## Contact & Support

For OpenCascade documentation: https://dev.opencascade.org/doc
For Qt documentation: https://doc.qt.io/

## Acknowledgments

- OpenCascade Technology for the 3D kernel
- Qt Project for the UI framework
- Tekla Structures for design inspiration
