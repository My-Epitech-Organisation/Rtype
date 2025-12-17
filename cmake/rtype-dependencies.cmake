# Centralized dependency helpers for the R-Type project.
# - Prefer vcpkg/toolchain-provided packages when available.
# - Fall back to CPM.cmake to fetch the same dependencies from source.

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
function(rtype_find_asio)
    if(NOT RTYPE_FORCE_CPM)
        find_package(asio CONFIG QUIET)
        if(asio_FOUND)
            if(TARGET asio::asio)
                return()
            endif()

            find_path(ASIO_INCLUDE_DIR
                NAMES asio.hpp
                PATHS ${asio_INCLUDE_DIRS} /usr/include /usr/local/include
                PATH_SUFFIXES asio
            )

            if(ASIO_INCLUDE_DIR)
                add_library(asio::asio INTERFACE IMPORTED)
                set_target_properties(asio::asio PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES "${ASIO_INCLUDE_DIR}"
                )
                return()
            else()
                message(WARNING "[deps] asio found but include directory could not be determined; falling back to CPM")
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
        add_library(asio::asio INTERFACE IMPORTED)
        target_include_directories(asio::asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
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
