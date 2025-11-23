# Simple Environment Verification Script

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Environment Verification" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$allGood = $true

# Check CMake
Write-Host "[1/5] Checking CMake..." -ForegroundColor Yellow
try {
    $cmakeVersion = cmake --version 2>$null
    if ($LASTEXITCODE -eq 0) {
        Write-Host "  [OK] CMake found" -ForegroundColor Green
        Write-Host "       $($cmakeVersion[0])" -ForegroundColor Gray
    } else {
        throw
    }
} catch {
    Write-Host "  [FAIL] CMake NOT FOUND" -ForegroundColor Red
    Write-Host "         Install from: https://cmake.org/download/" -ForegroundColor Yellow
    $allGood = $false
}

# Check Visual Studio / MSBuild
Write-Host "`n[2/5] Checking Visual Studio..." -ForegroundColor Yellow
try {
    $msbuild = Get-Command "MSBuild.exe" -ErrorAction SilentlyContinue
    if ($msbuild) {
        Write-Host "  [OK] Visual Studio found" -ForegroundColor Green
        Write-Host "       Path: $($msbuild.Source)" -ForegroundColor Gray
    } else {
        throw
    }
} catch {
    Write-Host "  [FAIL] Visual Studio NOT FOUND" -ForegroundColor Red
    Write-Host "         Install VS 2019 or 2022 with C++ workload" -ForegroundColor Yellow
    $allGood = $false
}

# Check Qt5
Write-Host "`n[3/5] Checking Qt5..." -ForegroundColor Yellow
$qtPaths = @(
    "C:\Qt\Qt5.12.12\5.12.12\msvc2017_64",
    "C:\Qt\5.12.12\msvc2017_64",
    "C:\Qt\Qt5.12.12\5.12.12\msvc2015_64"
)
$qtFound = $false
foreach ($qtPath in $qtPaths) {
    if (Test-Path "$qtPath\bin\qmake.exe") {
        Write-Host "  [OK] Qt5 found" -ForegroundColor Green
        Write-Host "       Path: $qtPath" -ForegroundColor Gray
        $qtFound = $true
        break
    }
}
if (-not $qtFound) {
    Write-Host "  [FAIL] Qt5 NOT FOUND" -ForegroundColor Red
    Write-Host "         Expected: C:\Qt\Qt5.12.12\5.12.12\msvc2017_64" -ForegroundColor Yellow
    $allGood = $false
}

# Check OpenCascade (workspace)
Write-Host "`n[4/5] Checking OpenCascade..." -ForegroundColor Yellow
$occPath = Join-Path $PSScriptRoot "opencascade\occt-vc14-64"
if (Test-Path "$occPath\cmake") {
    Write-Host "  [OK] OpenCascade found (workspace)" -ForegroundColor Green
    Write-Host "       Path: $occPath" -ForegroundColor Gray
} else {
    Write-Host "  [FAIL] OpenCascade NOT FOUND" -ForegroundColor Red
    Write-Host "         Run: .\install-occ-simple.ps1" -ForegroundColor Yellow
    $allGood = $false
}

# Check CMakeLists.txt
Write-Host "`n[5/5] Checking CMakeLists.txt..." -ForegroundColor Yellow
$cmakeFile = Join-Path $PSScriptRoot "CMakeLists.txt"
if (Test-Path $cmakeFile) {
    Write-Host "  [OK] CMakeLists.txt found" -ForegroundColor Green
} else {
    Write-Host "  [FAIL] CMakeLists.txt NOT FOUND" -ForegroundColor Red
    $allGood = $false
}

# Summary
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
if ($allGood) {
    Write-Host "  All checks passed!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Ready to build!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Next step:" -ForegroundColor Yellow
    Write-Host "  .\build.ps1" -ForegroundColor White
    Write-Host ""
} else {
    Write-Host "  Some checks failed" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Please install missing dependencies" -ForegroundColor Yellow
    Write-Host "See SETUP.md for details" -ForegroundColor Yellow
    Write-Host ""
}
