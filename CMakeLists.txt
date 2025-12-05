cmake_minimum_required(VERSION 3.15)

# Prefer Clang over GCC for better diagnostics and modern C++ support
# Override with: cmake -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc ..
if(NOT DEFINED CMAKE_CXX_COMPILER AND NOT DEFINED ENV{CXX})
    find_program(CLANG_CXX clang++)
    if(CLANG_CXX)
        set(CMAKE_CXX_COMPILER ${CLANG_CXX})
        set(CMAKE_C_COMPILER clang)
    endif()
endif()

project(r-type LANGUAGES CXX)

# Set C++20 standard globally
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_GRAPHICAL_TESTS "Build graphical tests" ON)
option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)
option(BUILD_DOCS "Build documentation (Doxygen + Docusaurus)" OFF)

# Coverage configuration
if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "Code coverage enabled")
        set(COVERAGE_COMPILE_FLAGS "--coverage -fprofile-arcs -ftest-coverage")
        set(COVERAGE_LINK_FLAGS "--coverage")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COVERAGE_COMPILE_FLAGS}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COVERAGE_COMPILE_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${COVERAGE_LINK_FLAGS}")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${COVERAGE_LINK_FLAGS}")
    else()
        message(WARNING "Code coverage is only supported with GCC or Clang")
    endif()
endif()

# Add libraries first (dependencies)
add_subdirectory(lib)

# Add source directory
add_subdirectory(src)

# Add tests if enabled
if(BUILD_TESTS)
    enable_testing()

    # Fetch Google Test
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_subdirectory(tests)
endif()

# Documentation targets
if(BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        # Doxygen configuration
        set(DOXYGEN_INPUT_DIR ${CMAKE_SOURCE_DIR})
        set(DOXYGEN_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/docs/doxygen)
        set(DOXYGEN_CONFIG_FILE ${CMAKE_SOURCE_DIR}/docs/Doxyfile)

        # Create Doxygen target
        add_custom_target(docs-doxygen
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONFIG_FILE}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )

        message(STATUS "Doxygen found - target 'docs-doxygen' available")
    else()
        message(WARNING "Doxygen not found - API documentation will not be generated")
    endif()

    # Find npm for Docusaurus
    find_program(NPM_EXECUTABLE npm)
    if(NPM_EXECUTABLE)
        # Install Docusaurus dependencies
        add_custom_target(docs-install
                COMMAND ${NPM_EXECUTABLE} install
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/docs/website
                COMMENT "Installing Docusaurus dependencies"
        )

        # Docusaurus development server
        add_custom_target(docs-serve
                COMMAND ${NPM_EXECUTABLE} start
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/docs/website
                COMMENT "Starting Docusaurus development server"
                USES_TERMINAL
        )

        # Docusaurus production build
        add_custom_target(docs-build
                COMMAND ${NPM_EXECUTABLE} run build
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/docs/website
                COMMENT "Building Docusaurus for production"
                DEPENDS docs-install
        )

        message(STATUS "npm found - Docusaurus targets available (docs-install, docs-serve, docs-build)")
    else()
        message(WARNING "npm not found - Docusaurus documentation will not be available")
    endif()

    # Combined documentation target
    if(DOXYGEN_FOUND AND NPM_EXECUTABLE)
        add_custom_target(docs
                COMMENT "Generating complete documentation (Doxygen + Docusaurus)"
                DEPENDS docs-doxygen docs-install
        )

        # Copy Doxygen output to Docusaurus static folder
        add_custom_command(TARGET docs POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${DOXYGEN_OUTPUT_DIR}/html
                ${CMAKE_SOURCE_DIR}/docs/website/static/api
                COMMENT "Copying Doxygen output to Docusaurus static folder"
        )

        message(STATUS "Complete documentation target 'docs' available")
    endif()
endif()

target_include_directories(rtype-graphic-main-menu-scene
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
