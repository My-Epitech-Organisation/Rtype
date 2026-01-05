# Dependency Management: vcpkg + CPM Fallback

## Overview

The R-Type project now supports **dual dependency management**:

1. **Primary (default)**: vcpkg with manifest mode (`vcpkg.json`)
2. **Fallback**: CPM.cmake (C++ Package Manager) for automatic source-based builds

This provides resilience against vcpkg unavailability while maintaining optimal performance and consistency with vcpkg binaries when available.

---

## How It Works

### Strategy Selection

The build system automatically decides which approach to use:

| Scenario | Behavior |
|----------|----------|
| vcpkg found (VCPKG_ROOT or external/vcpkg) | Use vcpkg (build dir: `build`) |
| vcpkg not found | Fall back to CPM (build dir: `build-cpm`) |
| `./build.sh -c` flag | Force CPM regardless |

### Automatic Detection

The vcpkg toolchain (`cmake/vcpkg-toolchain.cmake`) now:

- Sets `RTYPE_VCPKG_FOUND` flag when vcpkg is available
- **Warns** instead of **failing** if vcpkg is missing
- Allows CPM to take over seamlessly

The dependency helpers (`cmake/rtype-dependencies.cmake`) expose:

- `RTYPE_USE_CPM` flag (ON if CPM is active)
- Helper functions: `rtype_find_*()` (e.g., `rtype_find_sfml()`, `rtype_find_asio()`)

---

## Usage

### Standard Build (Prefer vcpkg, fall back to CPM)

```bash
./build.sh              # Full rebuild
./build.sh -r           # Incremental
./build.sh -t           # With tests
./build.sh -r -t        # Incremental + tests
```

### Force CPM (Skip vcpkg entirely)

```bash
./build.sh -c           # Force CPM, full rebuild
./build.sh -c -r        # Force CPM, incremental
```

### Manual CMake

```bash
# vcpkg-based (if available)
cmake --preset linux-release
cmake --build --preset linux-release

# CPM-based
cmake --preset linux-release-cpm
cmake --build --preset linux-release-cpm

# Or manually
cmake -B build -DRTYPE_FORCE_CPM=ON
cmake --build build
```

---

## Supported Dependencies

CPM fallback includes configurations for:

| Package | Version | Source |
|---------|---------|--------|
| asio | 1.28.0+ | GitHub (chriskohlhoff/asio) |
| tomlplusplus | 3.4.0+ | GitHub (marzer/tomlplusplus) |
| SFML | 3.0.2+ | GitHub (SFML/SFML) |
| SDL2 | 2.32.4 | GitHub (libsdl-org/SDL) |
| zlib | 1.3.1+ | GitHub (madler/zlib) |
| libpng | 1.6.51+ | GitHub (glennrp/libpng) |
| bzip2 | 1.0.8+ | GitLab (bzip2/bzip2) |
| brotli | 1.2.0+ | GitHub (google/brotli) |

All other dependencies (threads, etc.) are system-provided or optional.

---

## File Structure

```text
cmake/
├── vcpkg-toolchain.cmake     # Modified: now warns instead of fails
├── rtype-dependencies.cmake  # NEW: CPM helpers and strategy logic
└── CPM.cmake                 # NEW: CPM.cmake downloader (fetches on demand)
```

### Modified CMakeLists Files

The following now use `rtype_find_*()` helpers instead of `find_package()`:

- `lib/common/CMakeLists.txt` (tomlplusplus)
- `lib/network/CMakeLists.txt` (asio)
- `src/client/CMakeLists.txt` (SFML, PNG, ZLIB, BZip2, brotli)
- `src/client/Graphic/CMakeLists.txt` (SFML, SDL2)

---

## CMake Presets

| Preset | Description |
|--------|-------------|
| `linux-release` | vcpkg-based, release build (build) |
| `linux-debug` | vcpkg-based, debug build (build) |
| `linux-release-cpm` | CPM-based, release build (build-cpm) |
| `linux-debug-cpm` | CPM-based, debug build (build-cpm) |
| `windows-release`, `windows-debug` | MSVC builds (vcpkg-based) |

Note: CPM presets intentionally do not load the vcpkg toolchain. They rely solely on `RTYPE_FORCE_CPM=ON` and the CPM downloader to fetch and build dependencies from source.

### Linking Behavior

**vcpkg-based builds** (all presets inheriting from `vcpkg-base`) now use **static library linkage** (`VCPKG_LIBRARY_LINKAGE=static`). This ensures that executables are standalone and do not require runtime dependencies (e.g., SFML DLL, libpng DLL) to be installed on the target system. This is a change from the previous default (dynamic linking).

If dynamic linking is required, override with `-DVCPKG_LIBRARY_LINKAGE=dynamic` in your CMake invocation.

---

### Link-Time Optimization (LTO)

The `linux-release` preset enables **Link-Time Optimization** (`CMAKE_INTERPROCEDURAL_OPTIMIZATION=ON`), which optimizes the entire program at link time for better runtime performance and smaller executable size.

**Trade-offs**:

- ✓ Faster executables and reduced size
- ✗ Significantly longer build time (2–3× slower than non-LTO builds)
- ✗ Higher memory usage during linking (may cause issues on memory-constrained systems)

If you need faster builds (e.g., during development), use `linux-debug` or disable LTO:

```bash
cmake --preset linux-release -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=OFF
```

---

## Cache Management

CPM downloads sources to: **`.cpm-cache/`** (project root)

This cache persists across builds, avoiding redundant downloads:

```bash
# Clean CPM cache (will re-download sources on next build)
rm -rf .cpm-cache/

# Clean CPM build directory
rm -rf build-cpm/

# Full clean (vcpkg + CPM)
rm -rf build/ build-cpm/ .cpm-cache/
```

---

## Environment Variables

### vcpkg-specific

- `VCPKG_ROOT`: Path to a personal vcpkg installation
  - If set and valid, used instead of `external/vcpkg`

### CMake Options

- `RTYPE_FORCE_CPM=ON`: Force CPM even if vcpkg is available
- `RTYPE_USE_CPM`: Read-only flag showing which strategy is active

---

## Troubleshooting

### CPM download fails

**Symptom**: "Failed to download CPM.cmake" or package download errors

**Solutions**:

1. Check internet connectivity
2. Manually bootstrap CPM:

   ```bash
   mkdir -p build-cpm/_deps && cd build-cpm
   curl -fSL https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.38.7/CPM.cmake \
       -o _deps/CPM.cmake
   cd ..
   ```

3. Use `VCPKG_ROOT` to point to a working vcpkg instead

### Package configuration not found

**Symptom**: "Could not find package X" during cmake configure

**Solutions**:

1. Ensure you're using a supported CMake version (3.19+)
2. Check `.cpm-cache/` for partial downloads; delete and retry
3. Use vcpkg instead: `cmake --preset linux-release`

### Different build directories interfere

**Symptom**: CMake errors when switching presets

**Solutions**:

1. Always rebuild when switching between vcpkg and CPM:

   ```bash
   rm -rf build/ build-cpm/
   ./build.sh
   ```

2. Or use incremental within the same strategy:

   ```bash
   ./build.sh -c -r  # Stay in CPM, incremental
   ```

---

## Performance Notes

- **vcpkg builds** (default): ~1-5 min (binary downloads)
- **CPM builds** (fallback): ~3-10 min (source compilation)
- **Incremental builds** (`-r` flag): ~10-30 sec (both)

CPM downloads are cached, so subsequent builds are faster.

---

## Security Considerations

### Supply Chain Hardening

CPM-based builds fetch dependencies directly from upstream GitHub/GitLab repositories using version tags (e.g., `v3.0.2`, `1.28.0`). While tags are convenient, they are **mutable** and can be force-pushed or modified on upstream repositories, creating a supply chain risk.

#### For Development Builds (Current Approach)

- Tags provide convenience and flexibility
- Acceptable for development and CI builds with trusted upstream sources
- Assumes well-maintained projects (SFML, asio, libpng, etc.) with low compromise risk

#### For Production / Hardened Builds

Consider these additional measures:

1. **Pin to commit SHAs** instead of tags:

   ```cmake
   # Instead of: GIT_TAG v3.0.2
   GIT_TAG a1b2c3d4e5f6g7h8i9j0...  # Full commit SHA
   ```

2. **Verify checksums** (if CPM version supports it):

   ```cmake
   GIT_TAG 3.0.2
   GIT_SHALLOW_COMMIT_HASH <sha256 hash>
   ```

3. **Vendor or mirror** critical dependencies:
   - Maintain internal copies of trusted versions
   - Use `CPM_LOCAL_PACKAGES_ONLY=ON` with local mirrors

4. **Audit upstream regularly**:
   - Monitor security advisories for dependencies
   - Review release notes before updating versions
   - Use `RTYPE_FORCE_CPM=OFF` (default) to prefer pre-built vcpkg binaries when available

See [CPM.cmake Security](https://github.com/cpm-cmake/CPM.cmake/wiki/Advantages#supply-chain-integrity) for more details.

---

## Contributing

When adding new dependencies:

1. Add to `vcpkg.json` (for vcpkg users)
2. Create `rtype_find_*()` helper in `cmake/rtype-dependencies.cmake`
3. Replace `find_package()` calls with `rtype_find_*()`
4. Test both presets:

   ```bash
   rm -rf build build-cpm .cpm-cache
   ./build.sh            # Tests vcpkg
   ./build.sh -c         # Tests CPM
   ```

---

## Build System Security

### CPM.cmake Bootstrapper

The CPM bootstrapper (`cmake/CPM.cmake`) downloads the CPM package manager directly from GitHub at configure time. While this provides convenience and flexibility, it introduces a supply chain risk: **remote code execution during the build** if the upstream repository or release asset is compromised.

The current implementation includes:

- **TLS verification** (HTTPS only)
- **Network error handling** (fatal error on download failure)
- **No cryptographic integrity check** (beyond TLS)

#### Risk Level

- **Development builds**: Low to moderate (acceptable with trusted upstream)
- **CI/CD pipelines**: Moderate to high (more exposure, potential secret access)
- **Production releases**: High (reaches end users)

#### Mitigation Strategies (Recommended Order)

1. **Vendor CPM.cmake into the repository** (BEST)
   - Download a known-good copy to `cmake/cpm/CPM.cmake`
   - Update this script to use the local copy
   - Eliminate external dependency during builds
   - Requires monitoring upstream for security updates

2. **Pin to commit + verify hash**
   - Use a specific commit SHA instead of release tag
   - Add SHA256 verification after download
   - Requires hash update when upgrading CPM

3. **Mirror to internal repository**
   - Host a copy on an internal artifact server
   - Use local URLs instead of GitHub
   - Reduces external dependency risk

4. **Use GitHub release signatures**
   - Download and verify GPG signatures of releases
   - More complex but provides cryptographic proof

#### Current Recommendation

For production builds, vendor CPM.cmake locally:

```bash
# Download a known-good version
mkdir -p cmake/cpm
curl -fSL https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.38.7/CPM.cmake \
    -o cmake/cpm/CPM.cmake

# Then update cmake/CPM.cmake to include the local copy instead
```

Verify the file contents and commit it to version control for reproducible builds.

---

## See Also

- [CPM.cmake Documentation](https://github.com/cpm-cmake/CPM.cmake)
- [vcpkg Documentation](https://vcpkg.io/)
- [R-Type Build Guide](../README.md)
