# Simple OpenCascade Installation Script
# Automatically installs to workspace

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  OpenCascade Installation" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$zipFile = "occt-combined-release-no-pch.zip"
$extractPath = Join-Path $PSScriptRoot "opencascade"

# Check if zip file exists
if (-not (Test-Path $zipFile)) {
    Write-Host "Error: $zipFile not found" -ForegroundColor Red
    exit 1
}

Write-Host "Found: $zipFile" -ForegroundColor Green
Write-Host "Installing to: $extractPath" -ForegroundColor Cyan
Write-Host ""

# Clean existing directory
if (Test-Path $extractPath) {
    Write-Host "Removing existing installation..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $extractPath
}

# Create directory
New-Item -ItemType Directory -Path $extractPath -Force | Out-Null

# Extract
Write-Host "Extracting files (this may take a few minutes)..." -ForegroundColor Cyan
try {
    Expand-Archive -Path $zipFile -DestinationPath $extractPath -Force
    Write-Host "Extraction complete!" -ForegroundColor Green
} catch {
    Write-Host "Extraction failed: $_" -ForegroundColor Red
    exit 1
}

# Find the actual OpenCascade directory
$subDirs = Get-ChildItem -Path $extractPath -Directory
if ($subDirs.Count -eq 1) {
    $occRoot = $subDirs[0].FullName
} else {
    $occRoot = $extractPath
}

Write-Host ""
Write-Host "OpenCascade root: $occRoot" -ForegroundColor Gray

# Find cmake directory
$cmakeDir = Get-ChildItem -Path $occRoot -Recurse -Directory -Filter "cmake" -ErrorAction SilentlyContinue | Select-Object -First 1

if ($cmakeDir) {
    $cmakePath = $cmakeDir.FullName.Replace('\', '/')
    Write-Host "CMake config: $cmakePath" -ForegroundColor Gray
} else {
    $cmakePath = $occRoot.Replace('\', '/')
    Write-Host "Using root path for CMake" -ForegroundColor Yellow
}

# Update CMakeLists.txt
Write-Host ""
Write-Host "Updating CMakeLists.txt..." -ForegroundColor Cyan
$cmakeFile = Join-Path $PSScriptRoot "CMakeLists.txt"

if (Test-Path $cmakeFile) {
    $content = Get-Content $cmakeFile -Raw
    $content = $content -replace 'set\(OpenCASCADE_DIR "[^"]*" CACHE PATH', "set(OpenCASCADE_DIR `"$cmakePath`" CACHE PATH"
    Set-Content -Path $cmakeFile -Value $content -NoNewline
    Write-Host "CMakeLists.txt updated!" -ForegroundColor Green
}

# Set environment variables
Write-Host ""
Write-Host "Setting environment variables..." -ForegroundColor Cyan
$env:CASROOT = $occRoot

# Find bin directory
$binDir = Get-ChildItem -Path $occRoot -Recurse -Directory -Filter "bin" -ErrorAction SilentlyContinue | Select-Object -First 1
if ($binDir) {
    $env:PATH = "$($binDir.FullName);$env:PATH"
    Write-Host "Added bin to PATH: $($binDir.FullName)" -ForegroundColor Gray
}

Write-Host "CASROOT: $occRoot" -ForegroundColor Gray

# Show summary
Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "  Installation Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "OpenCascade installed to:" -ForegroundColor White
Write-Host "  $occRoot" -ForegroundColor Cyan
Write-Host ""
Write-Host "Environment variables set for this session." -ForegroundColor Yellow
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "  1. .\verify.ps1    - Verify all dependencies" -ForegroundColor White
Write-Host "  2. .\build.ps1     - Build the project" -ForegroundColor White
Write-Host ""
