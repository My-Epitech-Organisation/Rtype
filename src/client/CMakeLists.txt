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

find_package(PNG QUIET)
find_package(ZLIB QUIET)
find_package(BZip2 QUIET)
find_package(unofficial-brotli CONFIG QUIET)

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

target_link_libraries(r-type_client PUBLIC
    engine
    network
    rtype_game_shared
    rtype_game_client
    rtype-graphic-SFML
    rtype_game_client
    rtype_client_network
)

if(PNG_FOUND)
    target_link_libraries(r-type_client PRIVATE PNG::PNG)
endif()
if(ZLIB_FOUND)
    target_link_libraries(r-type_client PRIVATE ZLIB::ZLIB)
endif()
if(BZip2_FOUND)
    target_link_libraries(r-type_client PRIVATE BZip2::BZip2)
endif()
if(TARGET unofficial::brotli::brotlidec)
    target_link_libraries(r-type_client PRIVATE unofficial::brotli::brotlidec unofficial::brotli::brotlicommon)
endif()

target_include_directories(rtype-graphic-main-menu-scene
        PUBLIC
        ./
)

target_link_libraries(rtype-graphic-SFML
    PRIVATE
    engine
    rtype_game_client_systems
)

if(NOT EXISTS ${CMAKE_BINARY_DIR}/assets)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/assets
    )
endif()
