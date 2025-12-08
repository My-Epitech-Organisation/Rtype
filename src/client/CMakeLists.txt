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

add_subdirectory(Graphic)

add_executable(r-type_client
        main.cpp
        ClientApp.cpp
        GameAction.hpp
        network/NetworkClient.cpp
        network/ClientNetworkSystem.cpp
)

target_compile_features(r-type_client PRIVATE cxx_std_20)

target_include_directories(r-type_client PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(r-type_client PUBLIC
    rtype_engine
    rtype_network
    rtype_game_shared
    rtype_game_client
    rtype-graphic-SFML
    rtype_game_client
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
    rtype_engine
    rtype_game_client_systems
)

execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/assets
)
