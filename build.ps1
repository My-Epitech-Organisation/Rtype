# build.ps1 - Build R-Type project for release on Windows
#
# This script:
#   1. Checks/sets up vcpkg
#   2. Configures CMake with release preset
#   3. Builds the project
#   4. Copies executables to repository root
#
# Usage:
#   .\build.ps1

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

Write-Host "→ Step 2/4: Configuring CMake (windows-release preset)..." -ForegroundColor Yellow
Set-Location $ProjectRoot

if (Test-Path "build") {
    Write-Host "  Cleaning existing build directory..."
    Remove-Item -Recurse -Force "build"
}

cmake --preset "windows-release"
if ($LASTEXITCODE -ne 0) {
    Write-Host "✗ CMake configuration failed" -ForegroundColor Red
    exit 1
}
Write-Host "✓ CMake configuration complete" -ForegroundColor Green
Write-Host ""

Write-Host "→ Step 3/4: Building project..." -ForegroundColor Yellow

$NumProcs = (Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors
if (-not $NumProcs) { $NumProcs = 4 }

cmake --build --preset "windows-release" -- /maxcpucount:$NumProcs
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
    Copy-Item $ClientExe $ProjectRoot
    Write-Host "  ✓ Copied r-type_client.exe" -ForegroundColor Green
} else {
    Write-Host "  ⚠ Warning: Client executable not found" -ForegroundColor Yellow
}

if (Test-Path $ServerExe) {
    Copy-Item $ServerExe $ProjectRoot
    Write-Host "  ✓ Copied r-type_server.exe" -ForegroundColor Green
} else {
    Write-Host "  ⚠ Warning: Server executable not found" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "╔══════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║                   Build Complete!                         ║" -ForegroundColor Cyan
Write-Host "╚══════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
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
