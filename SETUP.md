# Environment Setup Guide for Windows

## Quick Setup Checklist

- [ ] Visual Studio 2019/2022 installed
- [ ] CMake 3.16+ installed and in PATH
- [ ] Qt 6.5+ installed
- [ ] OpenCascade 7.7+ installed
- [ ] Environment variables configured

## Step-by-Step Installation

### 1. Install Visual Studio

1. Download Visual Studio 2019 or 2022 Community Edition
   - URL: https://visualstudio.microsoft.com/downloads/

2. During installation, select:
   - "Desktop development with C++"
   - Ensure C++17 support is included

### 2. Install CMake

1. Download CMake from: https://cmake.org/download/
2. Choose "Windows x64 Installer"
3. During installation, select "Add CMake to system PATH"
4. Verify installation:
   ```powershell
   cmake --version
   ```

### 3. Install Qt6

#### Option A: Qt Online Installer (Recommended)

1. Download Qt Online Installer: https://www.qt.io/download
2. Create a free Qt account
3. Select Qt 6.5.0 or higher
4. Choose: MSVC 2019 64-bit component
5. Install to: `C:\Qt\6.5.0\msvc2019_64`

#### Option B: Qt Offline Installer

1. Download from Qt archives: https://download.qt.io/archive/qt/
2. Install the same components as above

#### Configure Qt Environment

Add to System Environment Variables:
```
Qt6_DIR=C:\Qt\6.5.0\msvc2019_64\lib\cmake\Qt6
PATH=%PATH%;C:\Qt\6.5.0\msvc2019_64\bin
```

To set in PowerShell (temporary):
```powershell
$env:Qt6_DIR="C:\Qt\6.5.0\msvc2019_64\lib\cmake\Qt6"
$env:PATH+=";C:\Qt\6.5.0\msvc2019_64\bin"
```

### 4. Install OpenCascade

#### Option A: Download Pre-built Binaries (Recommended)

1. Visit: https://dev.opencascade.org/release
2. Download OCCT 7.7.0 for Windows
3. Extract to: `C:\OpenCASCADE-7.7.0\`

#### Option B: Build from Source

```powershell
# Clone repository
git clone https://git.dev.opencascade.org/repos/occt.git
cd occt

# Create build directory
mkdir build
cd build

# Configure
cmake .. -G "Visual Studio 16 2019" -A x64 `
  -DCMAKE_BUILD_TYPE=Release `
  -DINSTALL_DIR="C:\OpenCASCADE-7.7.0"

# Build (this takes time)
cmake --build . --config Release --target INSTALL
```

#### Configure OpenCascade Environment

Add to System Environment Variables:
```
CASROOT=C:\OpenCASCADE-7.7.0
PATH=%PATH%;C:\OpenCASCADE-7.7.0\win64\vc14\bin
```

To set in PowerShell (temporary):
```powershell
$env:CASROOT="C:\OpenCASCADE-7.7.0"
$env:PATH+=";C:\OpenCASCADE-7.7.0\win64\vc14\bin"
```

### 5. Verify Installation

Create a test script `verify.ps1`:
```powershell
Write-Host "Verifying Installation..." -ForegroundColor Cyan

# Check CMake
Write-Host "`nChecking CMake..." -ForegroundColor Yellow
cmake --version

# Check Qt
Write-Host "`nChecking Qt..." -ForegroundColor Yellow
if (Test-Path "C:\Qt\6.5.0\msvc2019_64\bin\qmake.exe") {
    Write-Host "Qt Found: C:\Qt\6.5.0\msvc2019_64" -ForegroundColor Green
} else {
    Write-Host "Qt NOT FOUND!" -ForegroundColor Red
}

# Check OpenCascade
Write-Host "`nChecking OpenCascade..." -ForegroundColor Yellow
if (Test-Path "C:\OpenCASCADE-7.7.0") {
    Write-Host "OpenCascade Found: C:\OpenCASCADE-7.7.0" -ForegroundColor Green
} else {
    Write-Host "OpenCascade NOT FOUND!" -ForegroundColor Red
}

# Check Visual Studio
Write-Host "`nChecking Visual Studio..." -ForegroundColor Yellow
if (Get-Command "MSBuild.exe" -ErrorAction SilentlyContinue) {
    Write-Host "Visual Studio Found" -ForegroundColor Green
} else {
    Write-Host "Visual Studio NOT FOUND!" -ForegroundColor Red
}

Write-Host "`nVerification Complete!" -ForegroundColor Cyan
```

Run it:
```powershell
.\verify.ps1
```

## Building the Project

### Method 1: Using Build Script

```powershell
cd C:\Users\lenovo\Documents\SOFTWARE_WORKSHOP\OCC
.\build.ps1
```

### Method 2: Manual Build

```powershell
# Navigate to project
cd C:\Users\lenovo\Documents\SOFTWARE_WORKSHOP\OCC

# Create build directory
mkdir build
cd build

# Configure
cmake .. -G "Visual Studio 16 2019" -A x64

# Build
cmake --build . --config Release

# Run
cd bin\Release
.\TeklaLikeCAD.exe
```

### Method 3: Using Visual Studio IDE

```powershell
# Generate solution
cd C:\Users\lenovo\Documents\SOFTWARE_WORKSHOP\OCC
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64

# Open in Visual Studio
start TeklaLikeCAD.sln
```

Then in Visual Studio:
- Set configuration to "Release"
- Build → Build Solution (F7)
- Debug → Start Without Debugging (Ctrl+F5)

## Common Issues and Solutions

### Issue 1: "CMake cannot find Qt6"

**Solution:**
```powershell
# Set Qt6_DIR explicitly
$env:Qt6_DIR="C:\Qt\6.5.0\msvc2019_64\lib\cmake\Qt6"

# Or pass it to CMake
cmake .. -DQt6_DIR="C:/Qt/6.5.0/msvc2019_64/lib/cmake/Qt6"
```

### Issue 2: "CMake cannot find OpenCASCADE"

**Solution:**
Edit `CMakeLists.txt` line 18:
```cmake
set(OpenCASCADE_DIR "C:/OpenCASCADE-7.7.0/cmake" CACHE PATH "Path to OpenCASCADE CMake directory")
```

Or pass it to CMake:
```powershell
cmake .. -DOpenCASCADE_DIR="C:/OpenCASCADE-7.7.0/cmake"
```

### Issue 3: "MSVCP140.dll not found"

**Solution:**
Install Visual C++ Redistributable:
- Download from: https://aka.ms/vs/17/release/vc_redist.x64.exe
- Run and install

### Issue 4: "Qt6Core.dll not found"

**Solution:**
Add Qt to PATH:
```powershell
$env:PATH+=";C:\Qt\6.5.0\msvc2019_64\bin"
```

Or copy Qt DLLs to exe directory:
```powershell
cd build\bin\Release
windeployqt TeklaLikeCAD.exe
```

### Issue 5: Black/Empty 3D Viewer

**Solution:**
- Update graphics drivers
- Ensure OpenGL 3.3+ support
- Check OpenCascade DLLs are accessible

### Issue 6: "LNK1104: cannot open file"

**Solution:**
- Ensure all paths use forward slashes `/` in CMakeLists.txt
- Rebuild from clean state
- Check Visual Studio version matches Qt build

## Setting Permanent Environment Variables

### Using PowerShell (Admin):

```powershell
# Qt
[System.Environment]::SetEnvironmentVariable('Qt6_DIR', 'C:\Qt\6.5.0\msvc2019_64\lib\cmake\Qt6', 'Machine')

# OpenCascade
[System.Environment]::SetEnvironmentVariable('CASROOT', 'C:\OpenCASCADE-7.7.0', 'Machine')

# Add to PATH
$path = [System.Environment]::GetEnvironmentVariable('PATH', 'Machine')
$newPath = $path + ';C:\Qt\6.5.0\msvc2019_64\bin;C:\OpenCASCADE-7.7.0\win64\vc14\bin'
[System.Environment]::SetEnvironmentVariable('PATH', $newPath, 'Machine')
```

### Using GUI:

1. Open "Environment Variables":
   - Press Win + R
   - Type: `sysdm.cpl`
   - Go to "Advanced" tab
   - Click "Environment Variables"

2. Add/Edit System Variables:
   - `Qt6_DIR` = `C:\Qt\6.5.0\msvc2019_64\lib\cmake\Qt6`
   - `CASROOT` = `C:\OpenCASCADE-7.7.0`

3. Edit PATH, add:
   - `C:\Qt\6.5.0\msvc2019_64\bin`
   - `C:\OpenCASCADE-7.7.0\win64\vc14\bin`
   - `C:\Program Files\CMake\bin`

4. Click OK and restart PowerShell

## Testing the Installation

After building, test basic functionality:

1. Run the application
2. Create a beam: Create → Create Beam
3. Try different views: View → Top/Front/Right/Isometric
4. Test navigation: Left drag (rotate), Middle drag (pan), Wheel (zoom)
5. Create multiple elements

## Next Steps

Once built successfully:
1. Explore the source code structure
2. Modify element dimensions in GeometryBuilder.cpp
3. Add new element types
4. Customize colors and materials
5. Implement save/load functionality

## Additional Resources

- CMake Documentation: https://cmake.org/documentation/
- Qt Documentation: https://doc.qt.io/qt-6/
- OpenCascade Documentation: https://dev.opencascade.org/doc/overview/html/
- C++ Reference: https://en.cppreference.com/
