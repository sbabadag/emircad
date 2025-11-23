# OpenCascade Installation Script
# This script extracts and sets up OpenCascade from the zip file

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  OpenCascade Installation Script" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$zipFile = "occt-combined-release-no-pch.zip"
$installPath = "C:\OpenCASCADE-7.7.0"
$workspaceExtractPath = Join-Path $PSScriptRoot "opencascade"

# Check if zip file exists
if (-not (Test-Path $zipFile)) {
    Write-Host "Error: $zipFile not found in current directory" -ForegroundColor Red
    Write-Host "  Please ensure the zip file is in the workspace" -ForegroundColor Yellow
    exit 1
}

Write-Host "Found: $zipFile" -ForegroundColor Green
Write-Host ""

# Option 1: Extract to workspace (local installation)
Write-Host "Installation Options:" -ForegroundColor Yellow
Write-Host "  1. Extract to workspace (Recommended for this project)" -ForegroundColor White
Write-Host "     Location: $workspaceExtractPath" -ForegroundColor Gray
Write-Host ""
Write-Host "  2. Extract to system folder (Global installation)" -ForegroundColor White
Write-Host "     Location: $installPath (requires admin rights)" -ForegroundColor Gray
Write-Host ""

$choice = Read-Host "Select option (1 or 2)"

if ($choice -eq "2") {
    # System-wide installation
    Write-Host ""
    Write-Host "Installing to system folder..." -ForegroundColor Yellow
    
    # Check admin rights
    $isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
    
    if (-not $isAdmin) {
        Write-Host "Administrator rights required for system installation" -ForegroundColor Red
        Write-Host "  Please run PowerShell as Administrator or choose option 1" -ForegroundColor Yellow
        exit 1
    }
    
    $extractPath = $installPath
} else {
    # Local installation (default)
    Write-Host ""
    Write-Host "Installing to workspace..." -ForegroundColor Yellow
    $extractPath = $workspaceExtractPath
}

# Create extraction directory
Write-Host "Creating directory: $extractPath" -ForegroundColor Cyan
if (Test-Path $extractPath) {
    Write-Host "  Directory exists, cleaning..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $extractPath
}
New-Item -ItemType Directory -Path $extractPath -Force | Out-Null

# Extract zip file
Write-Host ""
Write-Host "Extracting OpenCascade..." -ForegroundColor Cyan
Write-Host "  This may take a few minutes..." -ForegroundColor Gray

try {
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    [System.IO.Compression.ZipFile]::ExtractToDirectory($zipFile, $extractPath)
    Write-Host "Extraction complete" -ForegroundColor Green
} catch {
    Write-Host "Extraction failed: $_" -ForegroundColor Red
    exit 1
}

# Check if extraction created a subdirectory
$subDirs = Get-ChildItem -Path $extractPath -Directory
if ($subDirs.Count -eq 1) {
    $actualPath = $subDirs[0].FullName
    Write-Host ""
    Write-Host "Files extracted to: $actualPath" -ForegroundColor Gray
} else {
    $actualPath = $extractPath
}

# Look for common OpenCascade directories
$commonDirs = @("win64", "cmake", "inc", "include", "lib", "bin")
$foundDirs = @()
foreach ($dir in $commonDirs) {
    $searchPath = Get-ChildItem -Path $extractPath -Recurse -Directory -Filter $dir -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($searchPath) {
        $foundDirs += $dir
    }
}

Write-Host ""
Write-Host "Found directories: $($foundDirs -join ', ')" -ForegroundColor Green

# Update CMakeLists.txt with the correct path
Write-Host ""
Write-Host "Updating CMakeLists.txt..." -ForegroundColor Cyan

$cmakeFile = Join-Path $PSScriptRoot "CMakeLists.txt"
if (Test-Path $cmakeFile) {
    $content = Get-Content $cmakeFile -Raw
    
    # Find the cmake directory
    $cmakeDir = Get-ChildItem -Path $extractPath -Recurse -Directory -Filter "cmake" -ErrorAction SilentlyContinue | Select-Object -First 1
    
    if ($cmakeDir) {
        $cmakePath = $cmakeDir.FullName.Replace('\', '/')
        Write-Host "  CMake config found at: $cmakePath" -ForegroundColor Gray
    } else {
        # Use the extraction path
        $cmakePath = $actualPath.Replace('\', '/')
        Write-Host "  Using base path: $cmakePath" -ForegroundColor Gray
    }
    
    # Update the CMakeLists.txt
    $content = $content -replace 'set\(OpenCASCADE_DIR ".*?" CACHE PATH', "set(OpenCASCADE_DIR `"$cmakePath`" CACHE PATH"
    Set-Content -Path $cmakeFile -Value $content
    Write-Host "CMakeLists.txt updated" -ForegroundColor Green
} else {
    Write-Host "CMakeLists.txt not found" -ForegroundColor Yellow
}

# Set environment variables (temporary for current session)
Write-Host ""
Write-Host "Setting environment variables..." -ForegroundColor Cyan
$env:CASROOT = $extractPath

# Find bin directory
$binDir = Get-ChildItem -Path $extractPath -Recurse -Directory -Filter "bin" -ErrorAction SilentlyContinue | Select-Object -First 1
if ($binDir) {
    $env:PATH += ";$($binDir.FullName)"
    Write-Host "  Added to PATH: $($binDir.FullName)" -ForegroundColor Gray
}

Write-Host "  CASROOT set to: $extractPath" -ForegroundColor Gray
Write-Host "  These are temporary. See below for permanent setup." -ForegroundColor Yellow

# Summary
Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "  Installation Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "OpenCascade installed to:" -ForegroundColor Cyan
Write-Host "  $extractPath" -ForegroundColor White
Write-Host ""
Write-Host "For permanent environment variables, run:" -ForegroundColor Yellow
Write-Host ""
Write-Host '[System.Environment]::SetEnvironmentVariable("CASROOT", "' + $extractPath + '", "User")' -ForegroundColor White
if ($binDir) {
    Write-Host '$path = [System.Environment]::GetEnvironmentVariable("PATH", "User")' -ForegroundColor White
    Write-Host '$newPath = $path + ";' + $binDir.FullName + '"' -ForegroundColor White
    Write-Host '[System.Environment]::SetEnvironmentVariable("PATH", $newPath, "User")' -ForegroundColor White
}
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "  1. Verify environment: .\verify.ps1" -ForegroundColor White
Write-Host "  2. Build project: .\build.ps1" -ForegroundColor White
Write-Host ""
