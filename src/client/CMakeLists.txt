rtype_find_sfml()
rtype_find_png()
rtype_find_zlib()
rtype_find_bzip2()
rtype_find_brotli()

add_library(rtype_client_network STATIC
    network/NetworkClient.cpp
    network/ClientNetworkSystem.cpp
)

target_compile_features(rtype_client_network PRIVATE cxx_std_20)
target_include_directories(rtype_client_network PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(rtype_client_network
    PRIVATE
    network
    common
    rtype_game_client
)

add_subdirectory(Graphic)

add_executable(r-type_client
    main.cpp
    ClientApp.cpp
    GameAction.hpp
)

target_compile_features(r-type_client PRIVATE cxx_std_20)

target_include_directories(r-type_client PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

set_property(TARGET r-type_client PROPERTY ENABLE_EXPORTS ON)

target_link_libraries(r-type_client PUBLIC
    engine
    network
    rtype_game_shared
    rtype_game_client
    rtype-graphic
    rtype_game_client
    rtype_client_network
)

# Conditionally link optional compression/image libraries.
# This uses TARGET checks instead of *_FOUND variables for robustness:
# - Ensures the library was not only found, but also had its targets created
# - Works consistently across vcpkg, CPM, and system packages
# - Targets must exist with these exact names (PNG::PNG, ZLIB::ZLIB, etc.)
if(TARGET PNG::PNG)
    target_link_libraries(r-type_client PRIVATE PNG::PNG)
endif()
if(TARGET ZLIB::ZLIB)
    target_link_libraries(r-type_client PRIVATE ZLIB::ZLIB)
endif()
if(TARGET BZip2::BZip2)
    target_link_libraries(r-type_client PRIVATE BZip2::BZip2)
endif()
if(TARGET unofficial::brotli::brotlidec)
    target_link_libraries(r-type_client PRIVATE unofficial::brotli::brotlidec unofficial::brotli::brotlicommon)
endif()

target_include_directories(rtype-graphic-main-menu-scene
        PUBLIC
        ./
)

target_link_libraries(rtype-graphic
    PRIVATE
    engine
    rtype_game_client_systems
)
add_dependencies(r-type_client
    rtype-display-sfml
    rtype-graphic-background-asteroidsSpace
    rtype-graphic-background-lobby
    rtype-graphic-background-labs
    rtype-graphic-background-spatialStation
    rtype-graphic-background-meteorInside
    rtype-graphic-background-blackHole
    rtype-graphic-music-chillMusic
    rtype-graphic-music-battleMusic
    rtype-graphic-music-exploreMusic
)

if(NOT EXISTS ${CMAKE_BINARY_DIR}/assets)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/assets
    )
endif()
