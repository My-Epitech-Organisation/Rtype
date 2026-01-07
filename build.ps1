# build.ps1 - Build R-Type project for release on Windows
#
# This script:
#   1. Checks/sets up vcpkg
#   2. Configures CMake with release preset
#   3. Builds the project
#   4. Copies executables to repository root
#
# Usage:
#   .\build.ps1 [-r] [-t]
#   -r : Hot reload mode (incremental build, don't clean build directory)
#   -t : Build with tests included

param(
    [switch]$r,  # Incremental build (hot reload mode)
    [switch]$t,  # Build with tests
    [switch]$c,  # Force CPM (skip vcpkg)
    [switch]$h   # Help
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = $ScriptDir

if ($h) {
    Write-Host "Usage: .\build.ps1 [-r] [-t] [-c] [-h]"
    Write-Host "  -r   Incremental build (reuse existing build directory)"
    Write-Host "  -t   Build with unit tests"
    Write-Host "  -c   Force CPM (skip vcpkg even if available)"
    Write-Host "  -h   Show this help message"
    exit 0
}

Write-Host "╔══════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║           R-Type - Release Build Script (Windows)        ║" -ForegroundColor Cyan
Write-Host "╚══════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

if ($t) {
    Write-Host "→ Build mode: Client + Server + Tests"
} else {
    Write-Host "→ Build mode: Client + Server"
}
if ($r) {
    Write-Host "→ Build type: Incremental"
} else {
    Write-Host "→ Build type: Full rebuild"
}
Write-Host ""

# ========================================================================
# Step 1: Determine dependency strategy (vcpkg first, CPM fallback)
# ========================================================================
Write-Host "→ Step 1/4: Determining dependency strategy..."
Write-Host ""

$VcpkgAvailable = $false
$VcpkgDir = Join-Path $ProjectRoot "external\vcpkg"
$VcpkgExe = Join-Path $VcpkgDir "vcpkg.exe"
$CMakePreset = "windows-release"
$BuildDir = "build"

function Verify-VcpkgReady {
    param([string]$VcpkgPath)
    $VcpkgExePath = Join-Path $VcpkgPath "vcpkg.exe"
    if (-not (Test-Path $VcpkgExePath)) {
        return $false
    }
    try {
        $output = & $VcpkgExePath version 2>&1
        return $LASTEXITCODE -eq 0
    } catch {
        return $false
    }
}

if (-not $c) {
    if ($env:VCPKG_ROOT -and (Verify-VcpkgReady $env:VCPKG_ROOT)) {
        Write-Host "  ✓ Using personal vcpkg from VCPKG_ROOT: $env:VCPKG_ROOT"
        $VcpkgAvailable = $true
    } elseif (Verify-VcpkgReady $VcpkgDir) {
        Write-Host "  ✓ vcpkg found and bootstrapped in project: $VcpkgDir"
        $VcpkgAvailable = $true
    } elseif (Test-Path $VcpkgDir) {
        Write-Host "  ⚠ vcpkg directory exists but not bootstrapped."
        Write-Host "    Bootstrap vcpkg by running:"
        Write-Host "      cd $VcpkgDir && .\bootstrap-vcpkg.bat"
        Write-Host "    Then install dependencies:"
        Write-Host "      .\vcpkg install"
        Write-Host "    Or use -c flag to skip vcpkg and use CPM instead."
        Write-Host "  → Falling back to CPM..."
        $VcpkgAvailable = $false
    } else {
        Write-Host "  ⚠ vcpkg not found. Will use CPM as fallback."
        $VcpkgAvailable = $false
    }
} else {
    Write-Host "  → CPM forced via -c flag (skipping vcpkg check)"
    $VcpkgAvailable = $false
}

if ($VcpkgAvailable) {
    Write-Host "  → Using vcpkg-based build"
    $CMakePreset = "windows-release"
    $BuildDir = "build"
} else {
    Write-Host "  → Falling back to CPM for dependency management"
    $CMakePreset = "windows-release-cpm"
    $BuildDir = "build-cpm"
}

Write-Host ""

# ========================================================================
# Step 2: Configure CMake
# ========================================================================
Write-Host "→ Step 2/4: Configuring CMake ($CMakePreset preset)..." -ForegroundColor Yellow
Set-Location $ProjectRoot

$VSInstallPath = & "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if ($VSInstallPath) {
    Write-Host "  Found Visual Studio Build Tools at: $VSInstallPath" -ForegroundColor Green

    $vcpkgGitDir = Join-Path $VSInstallPath "VC\vcpkg\downloads\tools"
    if (Test-Path $vcpkgGitDir) {
        $gitDirs = Get-ChildItem -Path $vcpkgGitDir -Filter "git-*" -Directory | Sort-Object Name -Descending | Select-Object -First 1
        if ($gitDirs) {
            $gitPath = Join-Path $gitDirs.FullName "windows\bin"
            if (Test-Path $gitPath) {
                $env:PATH = "$gitPath;$env:PATH"
                Write-Host "  Added vcpkg git to PATH: $gitPath" -ForegroundColor Green
            }
        }
    }

    $VCVarsAll = Join-Path $VSInstallPath "VC\Auxiliary\Build\vcvars64.bat"
    if (Test-Path $VCVarsAll) {
        Write-Host "  Importing MSVC environment..." -ForegroundColor Green
        cmd /c "`"$VCVarsAll`" && set" | ForEach-Object {
            if ($_ -match "^([^=]+)=(.*)$") {
                [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2])
            }
        }
    }
}

# Handle build directory based on incremental mode
if (-not $r -and (Test-Path $BuildDir)) {
    Write-Host "  Cleaning existing build directory: $BuildDir"
    Remove-Item -Recurse -Force $BuildDir
}

if (-not (Test-Path $BuildDir)) {
    cmake --preset $CMakePreset
    Write-Host "✓ CMake configuration complete" -ForegroundColor Green
} else {
    Write-Host "✓ Using existing build configuration (incremental build)" -ForegroundColor Green
}
Write-Host ""

# ========================================================================
# Step 3: Build
# ========================================================================
Write-Host "→ Step 3/4: Building project..." -ForegroundColor Yellow

$NumProcs = (Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors
if (-not $NumProcs) { $NumProcs = 4 }

if ($t) {
    Write-Host "  Building with tests..." -ForegroundColor Green
    cmake --build --preset $CMakePreset --parallel $NumProcs
} else {
    Write-Host "  Building client and server only..." -ForegroundColor Green
    cmake --build --preset $CMakePreset --target r-type_client r-type_server --parallel $NumProcs
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "✗ Build failed" -ForegroundColor Red
    exit 1
}
Write-Host "✓ Build complete" -ForegroundColor Green
Write-Host ""

# ========================================================================
# Step 4: Copy executables to root
# ========================================================================
Write-Host "→ Step 4/4: Copying executables to repository root..." -ForegroundColor Yellow

$ClientExe = Join-Path $ProjectRoot "$BuildDir\src\client\Release\r-type_client.exe"
$ServerExe = Join-Path $ProjectRoot "$BuildDir\src\server\Release\r-type_server.exe"

if (-not (Test-Path $ClientExe)) {
    $ClientExe = Join-Path $ProjectRoot "$BuildDir\src\client\r-type_client.exe"
}
if (-not (Test-Path $ServerExe)) {
    $ServerExe = Join-Path $ProjectRoot "$BuildDir\src\server\r-type_server.exe"
}

if (Test-Path $ClientExe) {
    Copy-Item $ClientExe $ProjectRoot -Force
    Write-Host "  ✓ Copied r-type_client.exe" -ForegroundColor Green
} else {
    Write-Host "  ⚠ Warning: Client executable not found" -ForegroundColor Yellow
}

if (Test-Path $ServerExe) {
    Copy-Item $ServerExe $ProjectRoot -Force
    Write-Host "  ✓ Copied r-type_server.exe" -ForegroundColor Green
} else {
    Write-Host "  ⚠ Warning: Server executable not found" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "╔══════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║                   Build Complete!                        ║" -ForegroundColor Cyan
Write-Host "╚══════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""
Write-Host "Dependency strategy used: $(if ($VcpkgAvailable) { 'vcpkg' } else { 'CPM' })"
Write-Host "Build directory: $BuildDir"
Write-Host ""
Write-Host "Executables are now in the repository root:" -ForegroundColor White

$Executables = Get-ChildItem -Path $ProjectRoot -Filter "r-type_*.exe" -ErrorAction SilentlyContinue
if ($Executables) {
    $Executables | ForEach-Object {
        Write-Host "  $($_.Name) - $([math]::Round($_.Length / 1MB, 2)) MB" -ForegroundColor White
    }
} else {
    Write-Host "  (No executables found)" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Usage examples:" -ForegroundColor Cyan
Write-Host "  .\build.ps1       # Clean build without tests" -ForegroundColor Gray
Write-Host "  .\build.ps1 -r    # Incremental build (reuse existing build)" -ForegroundColor Gray  
Write-Host "  .\build.ps1 -t    # Clean build with tests" -ForegroundColor Gray
Write-Host "  .\build.ps1 -c    # Clean build with CPM (skip vcpkg)" -ForegroundColor Gray
Write-Host "  .\build.ps1 -r -t # Incremental build with tests" -ForegroundColor Gray
Write-Host "  .\build.ps1 -c -r # Incremental build with CPM" -ForegroundColor Gray
Write-Host ""
