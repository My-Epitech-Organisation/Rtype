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
add_subdirectory(Components)
add_subdirectory(System)

add_executable(r-type_client
        main.cpp
        ClientApp.cpp
        GameAction.hpp
)

target_compile_features(r-type_client PRIVATE cxx_std_20)

target_link_libraries(r-type_client PUBLIC
    rtype_engine
    rtype_network
    rtype_game_shared
    rtype_game_client
    rtype-graphic-SFML
)

target_include_directories(rtype-graphic-main-menu-scene
        PUBLIC
        ./
)

target_link_libraries(rtype-graphic-SFML
    PRIVATE
    rtype_engine
    rtype-client-system
)

execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/assets
)
