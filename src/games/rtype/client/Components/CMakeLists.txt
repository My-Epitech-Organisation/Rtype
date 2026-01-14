add_library(rtype_game_client_components STATIC
    ChargeShotVisualComponent.cpp
)

target_include_directories(rtype_game_client_components PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(rtype_game_client_components
    PUBLIC
    rtype_game_shared_components
)
