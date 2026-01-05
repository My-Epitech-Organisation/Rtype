# vcpkg-toolchain.cmake
# Automatically detects vcpkg with fallback
#
# Priority:
#   1. VCPKG_ROOT (environment variable - developer's personal vcpkg)
#   2. external/vcpkg (Git submodule - project default)
#
# Usage in CMakePresets.json:
#   "toolchainFile": "${sourceDir}/cmake/vcpkg-toolchain.cmake"

# Track availability so the top-level CMake logic can fall back to CPM when needed
set(RTYPE_VCPKG_FOUND OFF CACHE BOOL "True when vcpkg toolchain is available")

# Option 1: VCPKG_ROOT environment variable (personal vcpkg)
if(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "[vcpkg] Using personal vcpkg from VCPKG_ROOT: $ENV{VCPKG_ROOT}")
    set(RTYPE_VCPKG_FOUND ON CACHE BOOL "True when vcpkg toolchain is available" FORCE)
    include("$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    return()
endif()

# Option 2: Git submodule (external/vcpkg)
set(VCPKG_SUBMODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../external/vcpkg")
if(EXISTS "${VCPKG_SUBMODULE_PATH}/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "[vcpkg] Using submodule vcpkg from: ${VCPKG_SUBMODULE_PATH}")
    set(RTYPE_VCPKG_FOUND ON CACHE BOOL "True when vcpkg toolchain is available" FORCE)
    include("${VCPKG_SUBMODULE_PATH}/scripts/buildsystems/vcpkg.cmake")
    return()
endif()

# No option found - record that vcpkg is unavailable and let top-level CMake logic decide how to fall back
# (for example, to CPM-based dependency management if enabled in this project)
message(WARNING
    "[vcpkg] vcpkg toolchain not found during configuration. Continuing without vcpkg; "
    "the top-level build logic will automatically fall back to CPM-based dependency management if enabled.\n"
    "If you prefer vcpkg, set VCPKG_ROOT or initialize external/vcpkg before configuring the project."
)
set(RTYPE_VCPKG_FOUND OFF CACHE BOOL "True when vcpkg toolchain is available" FORCE)
