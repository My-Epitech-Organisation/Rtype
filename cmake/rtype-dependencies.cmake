# Centralized dependency helpers for the R-Type project.
# - Prefer vcpkg/toolchain-provided packages when available.
# - Fall back to CPM.cmake to fetch the same dependencies from source.
#
# SECURITY NOTE:
# CPM dependencies are pinned to version tags (e.g., v3.0.2, 1.28.0) from upstream
# GitHub/GitLab repositories. While convenient, tags are mutable and can be
# force-pushed or modified on the upstream repository. For production builds,
# consider:
#   1. Pinning to immutable commit SHAs instead of tags
#   2. Verifying integrity with checksums (e.g., GIT_TAG + GIT_SHALLOW_COMMIT_HASH)
#   3. Mirroring or vendoring critical build-time dependencies
#   4. Regularly auditing upstream repositories for security advisories
# See CPM.cmake documentation for options: https://github.com/cpm-cmake/CPM.cmake

include(${CMAKE_CURRENT_LIST_DIR}/CPM.cmake)

if(NOT DEFINED CPM_SOURCE_CACHE)
    set(CPM_SOURCE_CACHE "${CMAKE_SOURCE_DIR}/.cpm-cache" CACHE PATH "CPM download cache" FORCE)
endif()

option(RTYPE_FORCE_CPM "Force CPM even if a package manager is available" OFF)

if(NOT DEFINED RTYPE_VCPKG_FOUND)
    set(RTYPE_VCPKG_FOUND OFF)
endif()
set(RTYPE_USE_CPM ${RTYPE_FORCE_CPM} CACHE BOOL "Use CPM instead of vcpkg/system packages")
if(NOT RTYPE_USE_CPM AND NOT RTYPE_VCPKG_FOUND)
    set(RTYPE_USE_CPM ON CACHE BOOL "Use CPM instead of vcpkg/system packages" FORCE)
endif()

if(RTYPE_FORCE_CPM)
    set(CPM_USE_LOCAL_PACKAGES OFF CACHE BOOL "Prefer already-available packages" FORCE)
endif()

if(RTYPE_USE_CPM)
    message(STATUS "[deps] CPM fallback enabled (force=${RTYPE_FORCE_CPM}; vcpkg_found=${RTYPE_VCPKG_FOUND})")
else()
    message(STATUS "[deps] Using existing packages (vcpkg/system); CPM available as backup")
endif()

# === Helper functions ===
#
# TARGET NAMES CONTRACT:
# Each rtype_find_*() helper ensures that a well-known CMake target exists after
# the function completes. Callers can assume these targets are present and use them
# without conditional checks. The expected targets are:
#   - asio::asio (asio)
#   - tomlplusplus::tomlplusplus (tomlplusplus)
#   - SFML::Network, SFML::Graphics, SFML::Window, SFML::Audio, SFML::System (SFML)
#   - SDL2::SDL2 (SDL2)
#   - ZLIB::ZLIB (zlib)
#   - PNG::PNG (libpng)
#   - BZip2::BZip2 (bzip2)
#   - unofficial::brotli::brotlidec, unofficial::brotli::brotlicommon (brotli)
#   - lz4::lz4 (lz4)
#
# This works consistently across vcpkg, CPM, and system packages, allowing
# downstream CMakeLists (e.g., src/client) to use TARGET checks without *_FOUND guards.
#
function(rtype_find_asio)
    if(NOT RTYPE_FORCE_CPM)
        find_package(asio CONFIG QUIET)
        if(asio_FOUND)
            if(TARGET asio::asio)
                return()
            endif()

            # asio was found but target not created; try to construct it from headers
            # Asio is header-only, so we just need to find the include directory.
            # The include path may vary:
            #   - Some installs: /usr/include/asio/asio.hpp
            #   - Others: /usr/include/asio.hpp
            find_path(ASIO_INCLUDE_DIR
                NAMES asio.hpp
                PATHS ${asio_INCLUDE_DIRS} /usr/include /usr/local/include
                PATH_SUFFIXES asio
                NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
            )

            if(NOT ASIO_INCLUDE_DIR)
                find_path(ASIO_INCLUDE_DIR
                    NAMES asio.hpp
                    PATHS ${asio_INCLUDE_DIRS} /usr/include /usr/local/include
                    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
                )
            endif()

            if(ASIO_INCLUDE_DIR)
                add_library(asio::asio INTERFACE IMPORTED)
                set_target_properties(asio::asio PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES "${ASIO_INCLUDE_DIR}"
                )
                return()
            else()
                message(WARNING
                    "[deps] asio found but include directory (asio.hpp) could not be located in:\n"
                    "  - ${asio_INCLUDE_DIRS}\n"
                    "  - /usr/include\n"
                    "  - /usr/local/include\n"
                    "Falling back to CPM to fetch asio from source."
                )
                # do not return; proceed to CPM fallback below
            endif()
        endif()
    endif()

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

    if(NOT TARGET asio::asio)
        # Determine the correct include directory for standalone asio.
        # CPM may fetch asio from different repository forks or versions with varying layouts.
        # Try the primary expected layout first, then fall back to alternatives.
        set(ASIO_CANDIDATE_INCLUDE_DIRS
            "${asio_SOURCE_DIR}/asio/include"
            "${asio_SOURCE_DIR}/include"
        )
        set(ASIO_INCLUDE_DIR "")
        foreach(dir IN LISTS ASIO_CANDIDATE_INCLUDE_DIRS)
            if(EXISTS "${dir}/asio.hpp" OR EXISTS "${dir}/asio/asio.hpp")
                set(ASIO_INCLUDE_DIR "${dir}")
                message(STATUS "[deps] Found asio headers in: ${ASIO_INCLUDE_DIR}")
                break()
            endif()
        endforeach()

        if(NOT ASIO_INCLUDE_DIR)
            message(FATAL_ERROR
                "[deps] Unable to locate asio headers in fetched source directory.\n"
                "Checked: ${ASIO_CANDIDATE_INCLUDE_DIRS}\n"
                "asio_SOURCE_DIR: ${asio_SOURCE_DIR}\n"
                "Please verify the asio repository structure or try using vcpkg instead."
            )
        endif()

        add_library(asio::asio INTERFACE IMPORTED)
        target_include_directories(asio::asio INTERFACE "${ASIO_INCLUDE_DIR}")
    endif()
endfunction()

function(rtype_find_tomlplusplus)
    if(NOT RTYPE_FORCE_CPM)
        find_package(tomlplusplus CONFIG QUIET)
        if(tomlplusplus_FOUND)
            return()
        endif()
    endif()

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

    if(NOT TARGET tomlplusplus::tomlplusplus)
        add_library(tomlplusplus::tomlplusplus INTERFACE IMPORTED)
        target_include_directories(tomlplusplus::tomlplusplus INTERFACE ${tomlplusplus_SOURCE_DIR}/include)
    endif()
endfunction()

function(rtype_find_sfml)
    set(required_sfml_targets
        SFML::Network
        SFML::Graphics
        SFML::Window
        SFML::Audio
        SFML::System
    )
    set(all_sfml_targets_present TRUE)
    foreach(_sfml_tgt IN LISTS required_sfml_targets)
        if(NOT TARGET ${_sfml_tgt})
            set(all_sfml_targets_present FALSE)
            break()
        endif()
    endforeach()

    if(all_sfml_targets_present)
        message(STATUS "[deps] SFML already configured, skipping rtype_find_sfml()")
        return()
    endif()

    if(NOT RTYPE_FORCE_CPM)
        find_package(SFML
            COMPONENTS
            Network
            Graphics
            Window
            Audio
            System
            CONFIG
        )
        if(SFML_FOUND)
            return()
        endif()
    endif()

    message(STATUS "[deps] Building SFML from source via CPM...")

    CPMAddPackage(
        NAME SFML
        VERSION 3.0.2
        GITHUB_REPOSITORY SFML/SFML
        GIT_TAG 3.0.2
        # NOTE:
        # - FIND_PACKAGE_ARGUMENTS are only used by CPM when it can satisfy SFML
        #   via an existing/local package and calls find_package(SFML ...) for us.
        # - When CPM actually fetches and builds SFML from source, these arguments
        #   are ignored; the build configuration is instead driven by the OPTIONS below.
        FIND_PACKAGE_ARGUMENTS "COMPONENTS Network Graphics Window Audio System CONFIG"
        OPTIONS
            "BUILD_SHARED_LIBS=OFF"
            "SFML_BUILD_EXAMPLES=OFF"
            "SFML_BUILD_TEST_SUITE=OFF"
            "SFML_BUILD_DOC=OFF"
    )
endfunction()

function(rtype_find_sdl2)
    if(NOT RTYPE_FORCE_CPM)
        find_package(SDL2 CONFIG QUIET)
        if(SDL2_FOUND)
            return()
        endif()
    endif()

    CPMAddPackage(
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
    if(NOT RTYPE_FORCE_CPM)
        find_package(ZLIB QUIET)
        if(ZLIB_FOUND)
            return()
        endif()
    endif()

    CPMAddPackage(
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
    if(NOT RTYPE_FORCE_CPM)
        find_package(PNG QUIET)
        if(PNG_FOUND)
            return()
        endif()
    endif()

    CPMAddPackage(
        NAME PNG
        VERSION 1.6.51
        GITHUB_REPOSITORY glennrp/libpng
        GIT_TAG v1.6.51
        OPTIONS
            "PNG_SHARED=OFF"
            "PNG_TESTS=OFF"
            "PNG_BUILD_ZLIB=OFF"
    )
endfunction()

function(rtype_find_bzip2)
    if(NOT RTYPE_FORCE_CPM)
        find_package(BZip2 QUIET)
        if(BZip2_FOUND)
            return()
        endif()
    endif()

    CPMAddPackage(
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
    if(NOT RTYPE_FORCE_CPM)
        find_package(brotli CONFIG QUIET)
        if(brotli_FOUND)
            return()
        endif()
    endif()

    CPMAddPackage(
        NAME brotli
        VERSION 1.2.0
        GITHUB_REPOSITORY google/brotli
        GIT_TAG v1.2.0
        OPTIONS
            "BROTLI_BUNDLED_MODE=OFF"
            "BUILD_SHARED_LIBS=OFF"
    )

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

function(rtype_find_lz4)
    if(NOT RTYPE_FORCE_CPM)
        find_package(lz4 CONFIG QUIET)
        if(lz4_FOUND)
            return()
        endif()
    endif()

    CPMAddPackage(
        NAME lz4
        VERSION 1.10.0
        GITHUB_REPOSITORY lz4/lz4
        GIT_TAG v1.10.0
        SOURCE_SUBDIR build/cmake
        OPTIONS
            "LZ4_BUILD_CLI=OFF"
            "LZ4_BUILD_LEGACY_LZ4C=OFF"
            "BUILD_SHARED_LIBS=OFF"
            "BUILD_STATIC_LIBS=ON"
    )

    # Create alias target for consistent usage across vcpkg/CPM
    if(TARGET lz4_static AND NOT TARGET lz4::lz4)
        add_library(lz4::lz4 ALIAS lz4_static)
    elseif(TARGET LZ4::lz4_static AND NOT TARGET lz4::lz4)
        add_library(lz4::lz4 ALIAS LZ4::lz4_static)
    endif()
endfunction()

# Add cpp-httplib for admin server
function(rtype_find_httplib)
    if(TARGET httplib::httplib)
        message(STATUS "[deps] cpp-httplib already configured")
        return()
    endif()

    # Try vcpkg first
    find_package(httplib CONFIG QUIET)
    if(httplib_FOUND AND TARGET httplib::httplib)
        message(STATUS "[deps] Using cpp-httplib from vcpkg")
        return()
    endif()

    # Fallback to CPM
    message(STATUS "[deps] cpp-httplib not found in vcpkg, using CPM")
    CPMAddPackage(
        NAME httplib
        GITHUB_REPOSITORY yhirose/cpp-httplib
        VERSION 0.28.0
        OPTIONS
            "HTTPLIB_REQUIRE_OPENSSL OFF"
            "HTTPLIB_REQUIRE_ZLIB OFF"
            "HTTPLIB_REQUIRE_BROTLI ON"
    )
    if(httplib_ADDED)
        add_library(httplib::httplib ALIAS httplib)
    endif()
endfunction()
