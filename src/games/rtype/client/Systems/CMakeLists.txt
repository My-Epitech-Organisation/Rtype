add_library(rtype_game_client_systems STATIC
    RenderSystem.cpp
    RenderSystem.hpp
    EventSystem.cpp
    EventSystem.hpp
    ButtonUpdateSystem.cpp
    ButtonUpdateSystem.hpp
    ParallaxScrolling.cpp
    ParallaxScrolling.hpp
    ColorTintSystem.cpp
    ColorTintSystem.hpp
    BoxingSystem.cpp
    BoxingSystem.hpp
    ResetTriggersSystem.cpp
    ResetTriggersSystem.hpp
    TextInputSystem.cpp
    TextInputSystem.hpp
    PlayerAnimationSystem.cpp
    PlayerAnimationSystem.hpp
    PlayerPowerUpVisualSystem.cpp
    PlayerPowerUpVisualSystem.hpp
    PowerUpCollectionSystem.cpp
    PowerUpCollectionSystem.hpp
    AnimationSystem.cpp
    AnimationSystem.hpp
    ClientDestroySystem.cpp
    ClientDestroySystem.hpp
    ShaderRenderSystem.cpp
    ShaderRenderSystem.hpp
)

target_link_libraries(rtype_game_client_systems
    PUBLIC
    engine
    rtype-graphic-scene-manager
    rtype_game_shared
    rtype_game_client_components
)

target_include_directories(rtype_game_client_systems PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)
