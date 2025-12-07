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
    rtype_engine
    rtype_network
    rtype_game_shared
    rtype_game_client
    rtype-graphic-SFML
    rtype_game_client
)

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
