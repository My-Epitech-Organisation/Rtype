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
    [switch]$r,  # Hot reload mode
    [switch]$t   # Build with tests
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = $ScriptDir

Write-Host "╔══════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║           R-Type - Release Build Script (Windows)        ║" -ForegroundColor Cyan
Write-Host "╚══════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

Write-Host "→ Step 1/4: Checking vcpkg installation..." -ForegroundColor Yellow

$VcpkgDir = Join-Path $ProjectRoot "external\vcpkg"
$VcpkgExe = Join-Path $VcpkgDir "vcpkg.exe"

if ($env:VCPKG_ROOT -and (Test-Path (Join-Path $env:VCPKG_ROOT "vcpkg.exe"))) {
    Write-Host "✓ Using personal vcpkg from VCPKG_ROOT: $env:VCPKG_ROOT" -ForegroundColor Green
} elseif (Test-Path $VcpkgExe) {
    Write-Host "✓ vcpkg found in project: $VcpkgDir" -ForegroundColor Green
} else {
    Write-Host "→ vcpkg not found, setting up..." -ForegroundColor Yellow

    if (-not (Test-Path $VcpkgDir)) {
        Write-Host "  Cloning vcpkg..."
        git clone https://github.com/Microsoft/vcpkg.git $VcpkgDir
    }

    $BootstrapScript = Join-Path $VcpkgDir "bootstrap-vcpkg.bat"
    if (Test-Path $BootstrapScript) {
        Write-Host "  Bootstrapping vcpkg..."
        Push-Location $VcpkgDir
        & .\bootstrap-vcpkg.bat
        Pop-Location
    }

    if (Test-Path $VcpkgExe) {
        Write-Host "✓ vcpkg setup complete" -ForegroundColor Green
    } else {
        Write-Host "✗ Failed to setup vcpkg" -ForegroundColor Red
        exit 1
    }
}

Write-Host ""

# Determine build mode and preset
$BuildMode = if ($t) { "with tests" } else { "release" }
$ReloadMode = if ($r) { "hot reload" } else { "clean build" }
Write-Host "→ Step 2/4: Configuring CMake (windows-release preset, $BuildMode, $ReloadMode)..." -ForegroundColor Yellow
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

# Handle hot reload mode - only clean if not in hot reload mode
if (-not $r -and (Test-Path "build")) {
    Write-Host "  Cleaning existing build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force "build"
} elseif ($r -and (Test-Path "build")) {
    Write-Host "  Hot reload mode: Keeping existing build directory" -ForegroundColor Green
} elseif ($r) {
    Write-Host "  Hot reload mode: No existing build directory found, creating new one" -ForegroundColor Yellow
}

# Configure CMake with appropriate options
$CMakeArgs = @("--preset", "windows-release")
if ($t) {
    $CMakeArgs += @("-DBUILD_TESTS=ON", "-DBUILD_GRAPHICAL_TESTS=ON")
    Write-Host "  Building with tests enabled" -ForegroundColor Green
} else {
    Write-Host "  Building without tests (use -t to include tests)" -ForegroundColor Gray
}

cmake @CMakeArgs
if ($LASTEXITCODE -ne 0) {
    Write-Host "✗ CMake configuration failed" -ForegroundColor Red
    exit 1
}
Write-Host "✓ CMake configuration complete" -ForegroundColor Green
Write-Host ""

Write-Host "→ Step 3/4: Building project..." -ForegroundColor Yellow

$NumProcs = (Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors
if (-not $NumProcs) { $NumProcs = 4 }

# Build with appropriate verbosity for hot reload mode
if ($r) {
    Write-Host "  Hot reload: Building only modified files..." -ForegroundColor Green
    cmake --build --preset "windows-release" --parallel $NumProcs
} else {
    Write-Host "  Full build with $NumProcs parallel jobs..." -ForegroundColor Green
    cmake --build --preset "windows-release" --parallel $NumProcs
}
if ($LASTEXITCODE -ne 0) {
    Write-Host "✗ Build failed" -ForegroundColor Red
    exit 1
}
Write-Host "✓ Build complete" -ForegroundColor Green
Write-Host ""

Write-Host "→ Step 4/4: Copying executables to repository root..." -ForegroundColor Yellow

$ClientExe = Join-Path $ProjectRoot "build\src\client\Release\r-type_client.exe"
$ServerExe = Join-Path $ProjectRoot "build\src\server\Release\r-type_server.exe"

if (-not (Test-Path $ClientExe)) {
    $ClientExe = Join-Path $ProjectRoot "build\src\client\r-type_client.exe"
}
if (-not (Test-Path $ServerExe)) {
    $ServerExe = Join-Path $ProjectRoot "build\src\server\r-type_server.exe"
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

# If tests were built, show test information
if ($t) {
    Write-Host ""
    Write-Host "→ Test executables built:" -ForegroundColor Yellow
    $TestExes = Get-ChildItem -Path (Join-Path $ProjectRoot "build") -Filter "test_*.exe" -Recurse -ErrorAction SilentlyContinue
    if ($TestExes) {
        $TestExes | ForEach-Object {
            $RelativePath = $_.FullName.Replace($ProjectRoot, "")
            Write-Host "  $($_.Name) - $RelativePath" -ForegroundColor Gray
        }
        Write-Host "  Run tests with: ctest --test-dir build --output-on-failure" -ForegroundColor Cyan
    } else {
        Write-Host "  No test executables found" -ForegroundColor Yellow
    }
}

Write-Host ""
Write-Host "╔══════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
$Title = if ($t) { "║              Build Complete (with tests)!             ║" } else { "║                   Build Complete!                        ║" }
Write-Host $Title -ForegroundColor Cyan
Write-Host "╚══════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

# Build summary
if ($r) {
    Write-Host "Hot reload build completed - only modified files were rebuilt" -ForegroundColor Green
} else {
    Write-Host "Full clean build completed" -ForegroundColor Green
}

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
Write-Host "  .\build.ps1 -r    # Hot reload (incremental build)" -ForegroundColor Gray  
Write-Host "  .\build.ps1 -t    # Clean build with tests" -ForegroundColor Gray
Write-Host "  .\build.ps1 -r -t # Hot reload with tests" -ForegroundColor Gray
Write-Host ""
