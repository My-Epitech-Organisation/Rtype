# Centralized dependency helpers for the R-Type project.
# - Prefer vcpkg/toolchain-provided packages when available.
# - Fall back to CPM.cmake to fetch the same dependencies from source.

include(${CMAKE_CURRENT_LIST_DIR}/CPM.cmake)

# Cache the CPM source tree to avoid repeated downloads across builds
if(NOT DEFINED CPM_SOURCE_CACHE)
    set(CPM_SOURCE_CACHE "${CMAKE_SOURCE_DIR}/.cpm-cache" CACHE PATH "CPM download cache" FORCE)
endif()

option(RTYPE_FORCE_CPM "Force CPM even if a package manager is available" OFF)

# Decide whether we rely on CPM (no vcpkg/toolchain) or use existing packages
if(NOT DEFINED RTYPE_VCPKG_FOUND)
    set(RTYPE_VCPKG_FOUND OFF)
endif()
set(RTYPE_USE_CPM ${RTYPE_FORCE_CPM} CACHE BOOL "Use CPM instead of vcpkg/system packages")
if(NOT RTYPE_USE_CPM AND NOT RTYPE_VCPKG_FOUND)
    set(RTYPE_USE_CPM ON CACHE BOOL "Use CPM instead of vcpkg/system packages" FORCE)
endif()

# When forcing CPM, skip probing local packages to guarantee a CPM download path
if(RTYPE_FORCE_CPM)
    set(CPM_USE_LOCAL_PACKAGES OFF CACHE BOOL "Prefer already-available packages" FORCE)
endif()

if(RTYPE_USE_CPM)
    message(STATUS "[deps] CPM fallback enabled (force=${RTYPE_FORCE_CPM}; vcpkg_found=${RTYPE_VCPKG_FOUND})")
else()
    message(STATUS "[deps] Using existing packages (vcpkg/system); CPM available as backup")
endif()

# === Helper functions ===
function(rtype_find_asio)
    # First try system/vcpkg package (non-CPM path)
    if(NOT RTYPE_FORCE_CPM)
        find_package(asio CONFIG QUIET)
        if(asio_FOUND)
            # Ensure the asio::asio target exists
            if(NOT TARGET asio::asio)
                add_library(asio::asio INTERFACE IMPORTED)
                set_target_properties(asio::asio PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "/usr/include")
            endif()
            return()
        endif()
    endif()

    # Fall back to CPM
    CPMAddPackage(
        NAME asio
        VERSION 1.28.0
        GITHUB_REPOSITORY chriskohlhoff/asio
        GIT_TAG asio-1-28-0
        OPTIONS
            "ASIO_BUILD_EXAMPLES=OFF"
            "ASIO_BUILD_TESTS=OFF"
            "ASIO_BUILD_DOC=OFF"
            "ASIO_USE_BOOST=OFF"
    )

    # Ensure asio::asio target exists for linking
    if(NOT TARGET asio::asio)
        add_library(asio::asio INTERFACE IMPORTED)
        target_include_directories(asio::asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
    endif()
endfunction()

function(rtype_find_tomlplusplus)
    # First try system/vcpkg package (non-CPM path)
    if(NOT RTYPE_FORCE_CPM)
        find_package(tomlplusplus CONFIG QUIET)
        if(tomlplusplus_FOUND)
            return()
        endif()
    endif()

    # Fall back to CPM
    CPMAddPackage(
        NAME tomlplusplus
        VERSION 3.4.0
        GITHUB_REPOSITORY marzer/tomlplusplus
        GIT_TAG v3.4.0
        OPTIONS
            "TOMLPP_BUILD_EXAMPLES=OFF"
            "TOMLPP_BUILD_TESTS=OFF"
            "TOMLPP_BUILD_BENCHMARKS=OFF"
    )

    # Ensure tomlplusplus::tomlplusplus target exists for linking (header-only)
    if(NOT TARGET tomlplusplus::tomlplusplus)
        add_library(tomlplusplus::tomlplusplus INTERFACE IMPORTED)
        target_include_directories(tomlplusplus::tomlplusplus INTERFACE ${tomlplusplus_SOURCE_DIR}/include)
    endif()
endfunction()

function(rtype_find_sfml)
    # Avoid duplicate target creation by checking if already found
    if(TARGET SFML::Network)
        message(STATUS "[deps] SFML already configured, skipping retype_find_sfml()")
        return()
    endif()

    # When NOT forcing CPM, use vcpkg/system packages with CONFIG mode
    if(NOT RTYPE_FORCE_CPM)
        # Try VCPKG_FIND_PACKAGE_CONFIGURATION mode to prioritize vcpkg
        find_package(SFML
            COMPONENTS
            Network
            Graphics
            Window
            Audio
            System
            CONFIG
            REQUIRED
        )
        # If successful, return (vcpkg found it)
        if(SFML_FOUND)
            return()
        endif()
    endif()

    # If forcing CPM or vcpkg SFML not found, use CPM
    message(STATUS "[deps] Building SFML from source via CPM...")

    # Tell CPM to skip system package search when using CPM
    set(CPM_USE_FIND_PACKAGE_FALLBACK OFF)

    CPMAddPackage(
        NAME SFML
        VERSION 3.0.2
        GITHUB_REPOSITORY SFML/SFML
        GIT_TAG 3.0.2
        FIND_PACKAGE_ARGUMENTS "COMPONENTS Network Graphics Window Audio System CONFIG REQUIRED"
        OPTIONS
            "BUILD_SHARED_LIBS=OFF"
            "SFML_BUILD_EXAMPLES=OFF"
            "SFML_BUILD_TEST_SUITE=OFF"
            "SFML_BUILD_DOC=OFF"
    )
endfunction()

function(rtype_find_sdl2)
    # First try system/vcpkg package (non-CPM path)
    if(NOT RTYPE_FORCE_CPM)
        find_package(SDL2 CONFIG QUIET)
        if(SDL2_FOUND)
            return()
        endif()
    endif()

    # Fall back to CPM
    CPMFindPackage(
        NAME SDL2
        VERSION 2.32.4
        GITHUB_REPOSITORY libsdl-org/SDL
        GIT_TAG release-2.32.4
        OPTIONS
            "SDL_TEST=OFF"
            "SDL_INSTALL=OFF"
            "SDL_SHARED=OFF"
            "SDL_STATIC=ON"
            "SDL_STATIC_PIC=ON"
    )
endfunction()

function(rtype_find_zlib)
    # First try system/vcpkg package (non-CPM path)
    if(NOT RTYPE_FORCE_CPM)
        find_package(ZLIB QUIET)
        if(ZLIB_FOUND)
            return()
        endif()
    endif()

    # Fall back to CPM
    CPMFindPackage(
        NAME ZLIB
        VERSION 1.3.1
        GITHUB_REPOSITORY madler/zlib
        GIT_TAG v1.3.1
        OPTIONS
            "BUILD_SHARED_LIBS=OFF"
            "SKIP_INSTALL_ALL=ON"
            "ZLIB_ENABLE_TESTS=OFF"
    )
endfunction()

function(rtype_find_png)
    # First try system/vcpkg package (non-CPM path)
    if(NOT RTYPE_FORCE_CPM)
        find_package(PNG QUIET)
        if(PNG_FOUND)
            return()
        endif()
    endif()

    # Fall back to CPM
    CPMFindPackage(
        NAME PNG
        VERSION 1.6.44
        GITHUB_REPOSITORY glennrp/libpng
        GIT_TAG v1.6.44
        OPTIONS
            "PNG_SHARED=OFF"
            "PNG_TESTS=OFF"
            "PNG_BUILD_ZLIB=OFF"
    )
endfunction()

function(rtype_find_bzip2)
    # First try system/vcpkg package (non-CPM path)
    if(NOT RTYPE_FORCE_CPM)
        find_package(BZip2 QUIET)
        if(BZip2_FOUND)
            return()
        endif()
    endif()

    # Fall back to CPM
    CPMFindPackage(
        NAME BZip2
        VERSION 1.0.8
        GIT_REPOSITORY https://gitlab.com/bzip2/bzip2.git
        GIT_TAG bzip2-1.0.8
        OPTIONS
            "BUILD_SHARED_LIBS=OFF"
            "BZIP2_BUILD_EXAMPLES=OFF"
            "CMAKE_POSITION_INDEPENDENT_CODE=ON"
    )
endfunction()

function(rtype_find_brotli)
    # First try system/vcpkg package (non-CPM path)
    if(NOT RTYPE_FORCE_CPM)
        find_package(brotli CONFIG QUIET)
        if(brotli_FOUND)
            return()
        endif()
    endif()

    # Fall back to CPM
    CPMFindPackage(
        NAME brotli
        VERSION 1.1.0
        GITHUB_REPOSITORY google/brotli
        GIT_TAG v1.1.0
        OPTIONS
            "BROTLI_BUNDLED_MODE=OFF"
            "BUILD_SHARED_LIBS=OFF"
    )

    # Align target names with the ones provided by the vcpkg port for seamless linking
    if(TARGET brotlidec-static AND NOT TARGET unofficial::brotli::brotlidec)
        add_library(unofficial::brotli::brotlidec ALIAS brotlidec-static)
    endif()
    if(TARGET brotlicommon-static AND NOT TARGET unofficial::brotli::brotlicommon)
        add_library(unofficial::brotli::brotlicommon ALIAS brotlicommon-static)
    endif()
    if(TARGET brotlienc-static AND NOT TARGET unofficial::brotli::brotlienc)
        add_library(unofficial::brotli::brotlienc ALIAS brotlienc-static)
    endif()
endfunction()
