# Git Repository Cleanup Instructions

## Problem
The repository contains large files (~240MB zip files) and OpenCascade libraries (~10GB) that prevent pushing to GitHub.

## Solution: Fresh Start

### Step 1: Prepare Source Code Only
```powershell
# Navigate out of current directory
cd ..

# Create clean directory
mkdir TeklaLikeCAD-Clean
cd TeklaLikeCAD-Clean

# Initialize fresh git repository
git init
```

### Step 2: Copy Only Essential Files
From the old `OCC` directory, copy:
- `src/` folder (all .cpp files)
- `include/` folder (all .h files)
- `CMakeLists.txt`
- `*.md` files (README, SETUP, etc.)
- `*.ps1` scripts (build.ps1, run.ps1, etc.)
- `resources/` folder (if exists and not large)

**DO NOT COPY:**
- `opencascade/` directory
- `*.zip` files
- `build/` directory
- `.vs/` directory

### Step 3: Create Proper .gitignore
```gitignore
# Build directories
build/
bin/
lib/

# OpenCascade libraries (install separately)
opencascade/

# Large files
*.zip

# CMake
CMakeCache.txt
CMakeFiles/
cmake_install.cmake

# Visual Studio
.vs/
*.sln
*.vcxproj
*.vcxproj.filters
*.vcxproj.user

# Qt generated
moc_*.cpp
ui_*.h
qrc_*.cpp

# Compiled
*.exe
*.dll
*.so
*.lib
*.obj

# IDE
.vscode/
.idea/

# OS
.DS_Store
Thumbs.db
```

### Step 4: Commit and Push
```powershell
git add -A
git commit -m "Initial commit - TeklaLikeCad source code"

# If repository exists on GitHub
git remote add origin https://github.com/sbabadag/TeklaLikeCad.git
git branch -M main
git push -f origin main
```

### Step 5: Add OpenCascade Installation Instructions to README

Add to your README.md:

```markdown
## Prerequisites

### OpenCascade Installation
OpenCascade is not included in this repository due to size (10GB+).

**Download:**
- Download OpenCascade from official site or use provided scripts
- Place in `opencascade/` directory at project root
- Directory structure should be: `opencascade/occt-vc14-64/`

**Or run:**
```powershell
.\install-occ.ps1
```
```

## Alternative: Keep Current Repository

If you want to salvage your current repository:

```powershell
# Go back to master branch
cd c:\Users\lenovo\Documents\SOFTWARE_WORKSHOP\OCC
git checkout master

# Use BFG Repo-Cleaner (download from https://rtyley.github.io/bfg-repo-cleaner/)
java -jar bfg.jar --strip-blobs-bigger-than 50M
git reflog expire --expire=now --all
git gc --prune=now --aggressive
git push --force
```

## Summary

The cleanest solution is to **start fresh with a new directory** containing only source code. This avoids complicated git history rewriting and ensures a clean repository from the start.

Your working OpenCascade installation in `C:\Users\lenovo\Documents\SOFTWARE_WORKSHOP\OCC\opencascade` stays local and is documented for other users to install separately.
