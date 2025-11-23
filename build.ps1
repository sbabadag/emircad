# Build script for Tekla-Like CAD Application
# Run this script from PowerShell

Write-Host "==================================" -ForegroundColor Cyan
Write-Host "Tekla-Like CAD Build Script" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
Write-Host ""

# Set up Visual Studio environment
$vsPath = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools"
if (-not (Test-Path $vsPath)) {
    $vsPath = "C:\Program Files\Microsoft Visual Studio\2022\Community"
}
if (-not (Test-Path $vsPath)) {
    $vsPath = "C:\Program Files\Microsoft Visual Studio\2022\Professional"
}

$vcvarsPath = "$vsPath\VC\Auxiliary\Build\vcvars64.bat"

if (-not (Test-Path $vcvarsPath)) {
    Write-Host "Visual Studio 2022 not found!" -ForegroundColor Red
    Write-Host "Looking for VS 2019..." -ForegroundColor Yellow
    $vsPath = "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools"
    $vcvarsPath = "$vsPath\VC\Auxiliary\Build\vcvars64.bat"
}

if (-not (Test-Path $vcvarsPath)) {
    Write-Host "Visual Studio not found!" -ForegroundColor Red
    Write-Host "Please install Visual Studio 2019 or 2022" -ForegroundColor Yellow
    exit 1
}

Write-Host "Found Visual Studio at: $vsPath" -ForegroundColor Green

Write-Host "Found Visual Studio at: $vsPath" -ForegroundColor Green

# Check if build directory exists
if (Test-Path "build") {
    Write-Host "Build directory exists. Cleaning..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force build
}

# Create build directory
Write-Host "Creating build directory..." -ForegroundColor Green
New-Item -ItemType Directory -Path "build" | Out-Null
Set-Location build

# Set environment variables for Qt5 and OpenCascade
$env:CMAKE_PREFIX_PATH = "C:\Qt\Qt5.12.12\5.12.12\msvc2017_64"
$env:Qt5_DIR = "C:\Qt\Qt5.12.12\5.12.12\msvc2017_64\lib\cmake\Qt5"
$occPath = Join-Path $PSScriptRoot "opencascade\occt-vc14-64"
$env:CASROOT = $occPath

Write-Host ""
Write-Host "Environment variables set:" -ForegroundColor Cyan
Write-Host "  CMAKE_PREFIX_PATH = $env:CMAKE_PREFIX_PATH" -ForegroundColor Gray
Write-Host "  CASROOT = $env:CASROOT" -ForegroundColor Gray

# Configure with CMake
Write-Host ""
Write-Host "Configuring project with CMake..." -ForegroundColor Green
Write-Host "Using Visual Studio 2022..." -ForegroundColor Yellow

cmake .. -G "Visual Studio 17 2022" -A x64

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    Write-Host "Please check that:" -ForegroundColor Yellow
    Write-Host "  1. Qt6 is installed and Qt6_DIR is set" -ForegroundColor Yellow
    Write-Host "  2. OpenCascade is installed at C:\OpenCASCADE-7.7.0" -ForegroundColor Yellow
    Write-Host "  3. Visual Studio 2019 is installed" -ForegroundColor Yellow
    Set-Location ..
    exit 1
}

# Build the project
Write-Host ""
Write-Host "Building project (Release)..." -ForegroundColor Green
cmake --build . --config Release

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "Build failed!" -ForegroundColor Red
    Set-Location ..
    exit 1
}

# Success
Write-Host ""
Write-Host "==================================" -ForegroundColor Green
Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host "==================================" -ForegroundColor Green
Write-Host ""
Write-Host "Executable location: .\build\bin\Release\TeklaLikeCAD.exe" -ForegroundColor Cyan
Write-Host ""
Write-Host "To run the application:" -ForegroundColor Yellow
Write-Host "  cd build\bin\Release" -ForegroundColor White
Write-Host "  .\TeklaLikeCAD.exe" -ForegroundColor White
Write-Host ""

Set-Location ..
