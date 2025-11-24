# Network Library Selection - R-Type Project

## Decision: ASIO Standalone

**Selected Library:** ASIO Standalone 1.30.2
**Date:** 24 novembre 2025
**Status:** ✅ APPROVED

## Executive Summary

ASIO Standalone has been chosen as the networking library for the R-Type project based on comprehensive benchmarking of four candidates. It offers the best balance between performance, binary size, and zero external dependencies.

## Benchmark Results Comparison

| Library | Config Time | Build Time | Server Size | Client Size | Dependencies | Notes |
|---------|-------------|------------|-------------|-------------|--------------|-------|
| **ASIO Standalone** | **17.7s** | **1.66s** | **347 KB** | **355 KB** | **None** | ✅ **SELECTED** |
| Boost.Asio | 26.7s | 1.30s | 391 KB | 455 KB | Full Boost | ⚠️ Slower config, larger binaries |
| Qt Network | 17.2s | 1.30s | 47 KB | 75 KB | Qt6 Framework | ❌ Requires QCoreApplication |
| ACE | N/A | N/A | N/A | N/A | ACE Framework | ❌ Too old, config failed |

## Key Decision Factors

### ✅ Why ASIO Standalone?

1. **Header-only library** - No linking required, simplifies deployment
2. **Zero dependencies** - No external frameworks needed (vs Boost/Qt/ACE)
3. **Reasonable binary size** - Smaller than Boost.Asio (347KB vs 391KB server)
4. **Fast build times** - 1.66s compilation, acceptable for development
5. **Modern C++20 compatible** - Fully supports project requirements
6. **Cross-platform** - Works on Linux/Windows without changes
7. **Active maintenance** - Latest stable version (1.30.2)

### ❌ Why NOT the alternatives?

- **Boost.Asio**: 50% slower configuration (26.7s vs 17.7s), 13% larger binaries, requires entire Boost framework
- **Qt Network**: Requires `QCoreApplication::exec()` event loop - incompatible with game engine architecture
- **ACE**: Deprecated, failed to configure, not suitable for modern C++20 projects

## Technical Justification

The R-Type project requires:

- UDP networking for real-time multiplayer gameplay
- Asynchronous I/O for non-blocking server operations
- Minimal overhead for game performance
- Cross-platform compatibility (Linux/Windows)

ASIO Standalone meets all requirements without the framework overhead of Boost or Qt, and without the maintenance burden of legacy libraries like ACE.

## Implementation

```cmake
# CMakeLists.txt
CPMAddPackage(
    NAME asio
    GITHUB_REPOSITORY chriskohlhoff/asio
    GIT_TAG asio-1-30-2
    OPTIONS "ASIO_STANDALONE ON"
)
```

**Integration path:** `examples/asio_poc/` contains functional proof-of-concept with UDP server/client examples.

## Conclusion

ASIO Standalone provides production-ready async networking with zero dependencies and reasonable overhead - the optimal choice for R-Type's networked game engine.
