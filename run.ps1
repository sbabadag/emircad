# Run script for Tekla-Like CAD Application

Write-Host "Starting Tekla-Like CAD Application..." -ForegroundColor Cyan

# Set up environment paths
$qtPath = "C:\Qt\Qt5.12.12\5.12.12\msvc2017_64\bin"
$occPath = "$PSScriptRoot\opencascade\occt-vc14-64\win64\vc14\bin"
$vtk3rdPartyPath = "$PSScriptRoot\opencascade\3rdparty-vc14-64"

# Add to PATH for this session
$env:PATH = "$qtPath;$occPath;$vtk3rdPartyPath\vtk-9.4.1-x64\bin;$vtk3rdPartyPath\freetype-2.13.3-x64\bin;$env:PATH"

# Set CASROOT
$env:CASROOT = "$PSScriptRoot\opencascade\occt-vc14-64"

# Check if executable exists
$exePath = "$PSScriptRoot\build\bin\Release\TeklaLikeCAD.exe"
if (-not (Test-Path $exePath)) {
    Write-Host "Error: Executable not found!" -ForegroundColor Red
    Write-Host "Please build the project first: .\build.ps1" -ForegroundColor Yellow
    exit 1
}

$exeDir = "$PSScriptRoot\build\bin\Release"

# Copy all required DLLs to executable directory (one-time setup)
Write-Host "Ensuring all DLLs are in place..." -ForegroundColor Cyan

# Copy Qt5 DLLs
$qtBin = "C:\Qt\Qt5.12.12\5.12.12\msvc2017_64\bin"
Copy-Item "$qtBin\Qt5Core.dll" $exeDir -Force -ErrorAction SilentlyContinue
Copy-Item "$qtBin\Qt5Gui.dll" $exeDir -Force -ErrorAction SilentlyContinue
Copy-Item "$qtBin\Qt5Widgets.dll" $exeDir -Force -ErrorAction SilentlyContinue
Copy-Item "$qtBin\Qt5OpenGL.dll" $exeDir -Force -ErrorAction SilentlyContinue

# Copy Qt platform plugin
$qtPlugins = "C:\Qt\Qt5.12.12\5.12.12\msvc2017_64\plugins"
New-Item -ItemType Directory -Path "$exeDir\platforms" -Force | Out-Null
Copy-Item "$qtPlugins\platforms\qwindows.dll" "$exeDir\platforms\" -Force -ErrorAction SilentlyContinue

# Copy OpenCascade DLLs
$occBin = "$PSScriptRoot\opencascade\occt-vc14-64\win64\vc14\bin"
Copy-Item "$occBin\*.dll" $exeDir -Force -ErrorAction SilentlyContinue

# Copy VTK DLLs
$vtkBin = "$PSScriptRoot\opencascade\3rdparty-vc14-64\vtk-9.4.1-x64\bin"
if (Test-Path $vtkBin) {
    Copy-Item "$vtkBin\*.dll" $exeDir -Force -ErrorAction SilentlyContinue
}

# Copy FreeType DLLs
$freetypeBin = "$PSScriptRoot\opencascade\3rdparty-vc14-64\freetype-2.13.3-x64\bin"
if (Test-Path $freetypeBin) {
    Copy-Item "$freetypeBin\*.dll" $exeDir -Force -ErrorAction SilentlyContinue
}

# Copy additional 3rdparty DLLs
$third = "$PSScriptRoot\opencascade\3rdparty-vc14-64"
Copy-Item "$third\freeimage-3.18.0-x64\bin\*.dll" $exeDir -Force -ErrorAction SilentlyContinue
Copy-Item "$third\ffmpeg-3.3.4-64\bin\*.dll" $exeDir -Force -ErrorAction SilentlyContinue
Copy-Item "$third\openvr-1.14.15-64\bin\win64\*.dll" $exeDir -Force -ErrorAction SilentlyContinue
Copy-Item "$third\tbb-2021.13.0-x64\bin\*.dll" $exeDir -Force -ErrorAction SilentlyContinue
Copy-Item "$third\zlib-1.2.8-vc14-64\bin\*.dll" $exeDir -Force -ErrorAction SilentlyContinue
Copy-Item "$third\jemalloc-vc14-64\bin\*.dll" $exeDir -Force -ErrorAction SilentlyContinue

# Run the application
Write-Host ""
Write-Host "Launching Tekla-Like CAD Application..." -ForegroundColor Green
Write-Host "- Use left mouse to rotate view" -ForegroundColor Gray
Write-Host "- Use middle mouse to pan view" -ForegroundColor Gray
Write-Host "- Use mouse wheel to zoom" -ForegroundColor Gray
Write-Host "- Use Create menu to add structural elements" -ForegroundColor Gray
Write-Host ""

Start-Process -FilePath $exePath
