# vcpkg-toolchain.cmake
# Détecte automatiquement vcpkg avec fallback
#
# Priorité :
#   1. VCPKG_ROOT (variable d'environnement - vcpkg personnel du dev)
#   2. external/vcpkg (submodule Git - solution par défaut)
#
# Usage dans CMakePresets.json:
#   "toolchainFile": "${sourceDir}/cmake/vcpkg-toolchain.cmake"

# Option 1: Variable d'environnement VCPKG_ROOT (vcpkg personnel)
if(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "[vcpkg] Using personal vcpkg from VCPKG_ROOT: $ENV{VCPKG_ROOT}")
    include("$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    return()
endif()

# Option 2: Submodule Git (external/vcpkg)
set(VCPKG_SUBMODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../external/vcpkg")
if(EXISTS "${VCPKG_SUBMODULE_PATH}/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "[vcpkg] Using submodule vcpkg from: ${VCPKG_SUBMODULE_PATH}")
    include("${VCPKG_SUBMODULE_PATH}/scripts/buildsystems/vcpkg.cmake")
    return()
endif()

# Aucune option trouvée - erreur explicite
message(FATAL_ERROR 
    "[vcpkg] vcpkg not found!\n"
    "Please use one of these options:\n"
    "  1. Set VCPKG_ROOT environment variable to your vcpkg installation\n"
    "  2. Initialize the submodule: git submodule update --init --recursive\n"
    "     Then run: ./external/vcpkg/bootstrap-vcpkg.sh (Linux/macOS)\n"
    "           or: .\\external\\vcpkg\\bootstrap-vcpkg.bat (Windows)"
)
