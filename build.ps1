
param(
    [switch]$Help,
    [switch]$Debug,
    [switch]$Release,
    [switch]$Test,
    [switch]$BuildTests,
    [switch]$Verbose,
    [switch]$Clean,
    [int]$Jobs = 0,
    [switch]$Install,
    [string]$Prefix = "C:\Program Files\R-Type",
    [switch]$SetupVcpkg
)

$Colors = @{
    Info    = "Cyan"
    Success = "Green"
    Warn    = "Yellow"
    Error   = "Red"
}

$BuildType = if ($Debug) { "Debug" } else { "Release" }
$BuildTestsFlag = if ($Test -or $BuildTests) { "ON" } else { "OFF" }
$RunTests = $Test

$ScriptDir = $PSScriptRoot
$BuildDir = Join-Path $ScriptDir "build"
$VcpkgDir = Join-Path $ScriptDir "vcpkg"

if ($Jobs -eq 0) {
    $Jobs = (Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors
    if (-not $Jobs) { $Jobs = 4 }
}

function Log-Info { param($Message) Write-Host "[INFO] $Message" -ForegroundColor $Colors.Info }
function Log-Success { param($Message) Write-Host "[SUCCESS] $Message" -ForegroundColor $Colors.Success }
function Log-Warn { param($Message) Write-Host "[WARN] $Message" -ForegroundColor $Colors.Warn }
function Log-Error { param($Message) Write-Host "[ERROR] $Message" -ForegroundColor $Colors.Error }

function Show-Help {
    @"
R-Type Build Script (Windows)

Usage: .\build.ps1 [options]

Options:
  -Help              Show this help message
  -Debug             Build in Debug mode (default: Release)
  -Release           Build in Release mode
  -Test              Build and run tests
  -BuildTests        Build tests without running them
  -Verbose           Enable verbose output
  -Clean             Clean build directory before building
  -Jobs N            Number of parallel jobs (default: auto-detect)
  -Install           Install after building
  -Prefix PATH       Installation prefix (default: C:\Program Files\R-Type)
  -SetupVcpkg        Setup vcpkg if not present

Examples:
  .\build.ps1                    # Release build
  .\build.ps1 -Debug             # Debug build
  .\build.ps1 -Test              # Build and run tests
  .\build.ps1 -Debug -Test -Verbose  # Debug build with tests, verbose
  .\build.ps1 -Clean -Debug      # Clean debug build
  .\build.ps1 -SetupVcpkg        # Setup vcpkg first, then build

"@
}

function Setup-Vcpkg {
    $VcpkgExe = Join-Path $VcpkgDir "vcpkg.exe"

    if (Test-Path $VcpkgExe) {
        Log-Info "vcpkg already installed at $VcpkgDir"
        return
    }

    Log-Info "Setting up vcpkg..."

    if (Test-Path $VcpkgDir) {
        Log-Warn "Removing incomplete vcpkg installation..."
        Remove-Item -Recurse -Force $VcpkgDir
    }

    git clone https://github.com/microsoft/vcpkg.git $VcpkgDir
    & "$VcpkgDir\bootstrap-vcpkg.bat" -disableMetrics

    Log-Success "vcpkg setup complete"
}

function Check-Dependencies {
    $Missing = @()

    if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
        $Missing += "cmake"
    }

    if (-not (Get-Command ninja -ErrorAction SilentlyContinue)) {
        Log-Warn "Ninja not found. Install with: choco install ninja"
    }

    $VsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $VsWhere)) {
        Log-Warn "Visual Studio not found. MSVC compiler may not be available."
    }

    if ($Missing.Count -gt 0) {
        Log-Error "Missing dependencies: $($Missing -join ', ')"
        Log-Info "Install with: choco install $($Missing -join ' ') ninja"
        exit 1
    }

    $VcpkgExe = Join-Path $VcpkgDir "vcpkg.exe"
    if (-not (Test-Path $VcpkgExe)) {
        Log-Warn "vcpkg not found at $VcpkgDir"
        Log-Info "Run with -SetupVcpkg to install vcpkg"
        Log-Info "Or manually: git clone https://github.com/microsoft/vcpkg.git; .\vcpkg\bootstrap-vcpkg.bat"
        exit 1
    }
}

function Clean-Build {
    if (Test-Path $BuildDir) {
        Log-Info "Cleaning build directory..."
        Remove-Item -Recurse -Force $BuildDir
        Log-Success "Build directory cleaned"
    }
}

function Configure-Project {
    Log-Info "Configuring project ($BuildType)..."

    $ToolchainFile = Join-Path $VcpkgDir "scripts\buildsystems\vcpkg.cmake"

    $CmakeArgs = @(
        "-S", $ScriptDir,
        "-B", $BuildDir,
        "-DCMAKE_BUILD_TYPE=$BuildType",
        "-DCMAKE_TOOLCHAIN_FILE=$ToolchainFile",
        "-DVCPKG_TARGET_TRIPLET=x64-windows",
        "-DBUILD_TESTS=$BuildTestsFlag",
        "-DBUILD_EXAMPLES=OFF"
    )

    if (Get-Command ninja -ErrorAction SilentlyContinue) {
        $CmakeArgs += @("-G", "Ninja")
    }

    if ($Install) {
        $CmakeArgs += "-DCMAKE_INSTALL_PREFIX=$Prefix"
    }

    if ($Verbose) {
        $CmakeArgs += "-DCMAKE_VERBOSE_MAKEFILE=ON"
    }

    & cmake @CmakeArgs

    if ($LASTEXITCODE -ne 0) {
        Log-Error "Configuration failed"
        exit 1
    }

    Log-Success "Configuration complete"
}

function Build-Project {
    Log-Info "Building project with $Jobs parallel jobs..."

    $BuildArgs = @(
        "--build", $BuildDir,
        "--parallel", $Jobs
    )

    if ($Verbose) {
        $BuildArgs += "--verbose"
    }

    & cmake @BuildArgs

    if ($LASTEXITCODE -ne 0) {
        Log-Error "Build failed"
        exit 1
    }

    Log-Success "Build complete"
}

function Run-Tests {
    Log-Info "Running tests..."

    Push-Location $BuildDir

    $CtestArgs = @(
        "--output-on-failure",
        "--timeout", "120"
    )

    if ($Verbose) {
        $CtestArgs += "--verbose"
    }

    & ctest @CtestArgs

    $TestResult = $LASTEXITCODE
    Pop-Location

    if ($TestResult -ne 0) {
        Log-Error "Tests failed"
        exit 1
    }

    Log-Success "All tests passed"
}

function Install-Project {
    Log-Info "Installing to $Prefix..."
    & cmake --install $BuildDir

    if ($LASTEXITCODE -ne 0) {
        Log-Error "Installation failed"
        exit 1
    }

    Log-Success "Installation complete"
}

function Print-Summary {
    Write-Host ""
    Write-Host "===========================================================" -ForegroundColor White
    Write-Host "                    BUILD SUMMARY"
    Write-Host "===========================================================" -ForegroundColor White
    Write-Host "  Build Type:    $BuildType"
    Write-Host "  Build Dir:     $BuildDir"
    Write-Host "  Tests:         $BuildTestsFlag"
    Write-Host "  Jobs:          $Jobs"
    Write-Host "===========================================================" -ForegroundColor White
    Write-Host ""

    $ServerExe = Join-Path $BuildDir "src\server\r-type_server.exe"
    $ClientExe = Join-Path $BuildDir "src\client\r-type_client.exe"

    $ServerExeRelease = Join-Path $BuildDir "src\server\Release\r-type_server.exe"
    $ClientExeRelease = Join-Path $BuildDir "src\client\Release\r-type_client.exe"

    if (Test-Path $ServerExe) {
        Log-Success "Server binary: $ServerExe"
    } elseif (Test-Path $ServerExeRelease) {
        Log-Success "Server binary: $ServerExeRelease"
    }

    if (Test-Path $ClientExe) {
        Log-Success "Client binary: $ClientExe"
    } elseif (Test-Path $ClientExeRelease) {
        Log-Success "Client binary: $ClientExeRelease"
    }

    Write-Host ""
}

function Main {
    Write-Host ""
    Write-Host "===========================================================" -ForegroundColor Cyan
    Write-Host "                   R-TYPE BUILD SYSTEM                     " -ForegroundColor Cyan
    Write-Host "===========================================================" -ForegroundColor Cyan
    Write-Host ""

    if ($Help) {
        Show-Help
        return
    }

    Set-Location $ScriptDir

    if ($SetupVcpkg) {
        Setup-Vcpkg
    }

    Check-Dependencies

    if ($Clean) {
        Clean-Build
    }

    Configure-Project
    Build-Project

    if ($RunTests) {
        Run-Tests
    }

    if ($Install) {
        Install-Project
    }

    Print-Summary
}

Main
