# Environment Verification Script
# Run this to check if all dependencies are properly installed

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Tekla-Like CAD - Environment Check" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$allGood = $true

# Check CMake
Write-Host "[1/5] Checking CMake..." -ForegroundColor Yellow
try {
    $cmakeVersion = cmake --version 2>$null
    if ($LASTEXITCODE -eq 0) {
        Write-Host "  ✓ CMake found" -ForegroundColor Green
        Write-Host "    $($cmakeVersion[0])" -ForegroundColor Gray
    } else {
        throw
    }
} catch {
    Write-Host "  ✗ CMake NOT FOUND" -ForegroundColor Red
    Write-Host "    Install from: https://cmake.org/download/" -ForegroundColor Yellow
    $allGood = $false
}

# Check Visual Studio / MSBuild
Write-Host "`n[2/5] Checking Visual Studio..." -ForegroundColor Yellow
try {
    $msbuild = Get-Command "MSBuild.exe" -ErrorAction SilentlyContinue
    if ($msbuild) {
        Write-Host "  ✓ Visual Studio found" -ForegroundColor Green
        Write-Host "    Path: $($msbuild.Source)" -ForegroundColor Gray
    } else {
        throw
    }
} catch {
    Write-Host "  ✗ Visual Studio NOT FOUND" -ForegroundColor Red
    Write-Host "    Install VS 2019 or 2022 with C++ workload" -ForegroundColor Yellow
    $allGood = $false
}

# Check Qt6
Write-Host "`n[3/5] Checking Qt6..." -ForegroundColor Yellow
$qtPaths = @(
    "C:\Qt\6.5.0\msvc2019_64",
    "C:\Qt\6.6.0\msvc2019_64",
    "C:\Qt\6.7.0\msvc2019_64"
)
$qtFound = $false
foreach ($qtPath in $qtPaths) {
    if (Test-Path "$qtPath\bin\qmake.exe") {
        Write-Host "  ✓ Qt6 found" -ForegroundColor Green
        Write-Host "    Path: $qtPath" -ForegroundColor Gray
        $qtFound = $true
        
        # Check if Qt6_DIR is set
        if ($env:Qt6_DIR) {
            Write-Host "    Qt6_DIR: $env:Qt6_DIR" -ForegroundColor Gray
        } else {
            Write-Host "    ⚠ Qt6_DIR not set (might cause issues)" -ForegroundColor Yellow
        }
        break
    }
}
if (-not $qtFound) {
    Write-Host "  ✗ Qt6 NOT FOUND" -ForegroundColor Red
    Write-Host "    Install from: https://www.qt.io/download" -ForegroundColor Yellow
    Write-Host "    Expected location: C:\Qt\6.5.0\msvc2019_64" -ForegroundColor Yellow
    $allGood = $false
}

# Check OpenCascade
Write-Host "`n[4/5] Checking OpenCascade..." -ForegroundColor Yellow
$occPaths = @(
    "C:\OpenCASCADE-7.7.0",
    "C:\OpenCASCADE-7.8.0",
    "C:\Program Files\OpenCASCADE-7.7.0"
)
$occFound = $false
foreach ($occPath in $occPaths) {
    if (Test-Path "$occPath\cmake") {
        Write-Host "  ✓ OpenCascade found" -ForegroundColor Green
        Write-Host "    Path: $occPath" -ForegroundColor Gray
        $occFound = $true
        
        # Check if CASROOT is set
        if ($env:CASROOT) {
            Write-Host "    CASROOT: $env:CASROOT" -ForegroundColor Gray
        } else {
            Write-Host "    ⚠ CASROOT not set (recommended to set)" -ForegroundColor Yellow
        }
        break
    }
}
if (-not $occFound) {
    Write-Host "  ✗ OpenCascade NOT FOUND" -ForegroundColor Red
    Write-Host "    Download from: https://dev.opencascade.org/release" -ForegroundColor Yellow
    Write-Host "    Expected location: C:\OpenCASCADE-7.7.0" -ForegroundColor Yellow
    $allGood = $false
}

# Check PATH entries
Write-Host "`n[5/5] Checking PATH..." -ForegroundColor Yellow
$pathEntries = @{
    "Qt bin" = "C:\Qt\6.5.0\msvc2019_64\bin"
    "OpenCascade bin" = "C:\OpenCASCADE-7.7.0\win64\vc14\bin"
}

foreach ($entry in $pathEntries.GetEnumerator()) {
    if ($env:PATH -like "*$($entry.Value)*") {
        Write-Host "  ✓ $($entry.Key) in PATH" -ForegroundColor Green
    } else {
        Write-Host "  ⚠ $($entry.Key) NOT in PATH" -ForegroundColor Yellow
        Write-Host "    Add: $($entry.Value)" -ForegroundColor Gray
    }
}

# Summary
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
if ($allGood) {
    Write-Host "  ✓ All checks passed!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "You're ready to build the project!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Next steps:" -ForegroundColor Yellow
    Write-Host "  1. cd C:\Users\lenovo\Documents\SOFTWARE_WORKSHOP\OCC" -ForegroundColor White
    Write-Host "  2. .\build.ps1" -ForegroundColor White
    Write-Host ""
} else {
    Write-Host "  ✗ Some checks failed" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Please install missing dependencies." -ForegroundColor Yellow
    Write-Host "See SETUP.md for detailed instructions." -ForegroundColor Yellow
    Write-Host ""
}

# Environment variable setup helper
Write-Host "Quick fix for environment variables:" -ForegroundColor Cyan
Write-Host "Copy and run these commands if needed:" -ForegroundColor Gray
Write-Host ""
Write-Host '$env:Qt6_DIR="C:\Qt\6.5.0\msvc2019_64\lib\cmake\Qt6"' -ForegroundColor White
Write-Host '$env:CASROOT="C:\OpenCASCADE-7.7.0"' -ForegroundColor White
Write-Host '$env:PATH+=";C:\Qt\6.5.0\msvc2019_64\bin;C:\OpenCASCADE-7.7.0\win64\vc14\bin"' -ForegroundColor White
Write-Host ""
