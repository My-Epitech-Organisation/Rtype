add_library(rtype_game_client_gamescene
    OBJECT
    RtypeGameScene.cpp
    RtypeGameScene.hpp
    RtypeEntityFactory.cpp
    RtypeEntityFactory.hpp
    RtypePauseMenu.cpp
    RtypePauseMenu.hpp
    RtypeInputHandler.cpp
    RtypeInputHandler.hpp
)

target_include_directories(rtype_game_client_gamescene PUBLIC
    ${CMAKE_SOURCE_DIR}/src/client
    ${CMAKE_SOURCE_DIR}/src/client/Graphic
    ${CMAKE_SOURCE_DIR}/src/games/rtype/client
    ${CMAKE_SOURCE_DIR}/lib/rtype_engine/src
    ${CMAKE_SOURCE_DIR}/lib/rtype_ecs/src
    ${CMAKE_SOURCE_DIR}/lib/rtype_common/src
    ${CMAKE_SOURCE_DIR}/lib/rtype_network/src
)

target_compile_features(rtype_game_client_gamescene PUBLIC cxx_std_20)

target_link_libraries(rtype_game_client_gamescene PUBLIC
    SFML::Graphics
    SFML::Window
    SFML::Audio
    SFML::System
    rtype_engine
    rtype_network
    rtype_game_shared
    rtype_game_client_components
)
