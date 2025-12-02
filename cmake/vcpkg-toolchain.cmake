# vcpkg-toolchain.cmake
# Automatically detects vcpkg with fallback
#
# Priority:
#   1. VCPKG_ROOT (environment variable - developer's personal vcpkg)
#   2. external/vcpkg (Git submodule - project default)
#
# Usage in CMakePresets.json:
#   "toolchainFile": "${sourceDir}/cmake/vcpkg-toolchain.cmake"

# Option 1: VCPKG_ROOT environment variable (personal vcpkg)
if(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "[vcpkg] Using personal vcpkg from VCPKG_ROOT: $ENV{VCPKG_ROOT}")
    include("$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    return()
endif()

# Option 2: Git submodule (external/vcpkg)
set(VCPKG_SUBMODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../external/vcpkg")
if(EXISTS "${VCPKG_SUBMODULE_PATH}/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "[vcpkg] Using submodule vcpkg from: ${VCPKG_SUBMODULE_PATH}")
    include("${VCPKG_SUBMODULE_PATH}/scripts/buildsystems/vcpkg.cmake")
    return()
endif()

# No option found - explicit error
message(FATAL_ERROR
    "[vcpkg] vcpkg not found!\n"
    "Please use one of these options:\n"
    "  1. Set VCPKG_ROOT environment variable to your vcpkg installation\n"
    "  2. Initialize the submodule: git submodule update --init --recursive\n"
    "     Then run: ./external/vcpkg/bootstrap-vcpkg.sh (Linux/macOS)\n"
    "           or: .\\external\\vcpkg\\bootstrap-vcpkg.bat (Windows)"
)
