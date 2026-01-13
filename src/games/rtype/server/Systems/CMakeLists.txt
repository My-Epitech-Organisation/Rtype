add_library(rtype_game_server_systems OBJECT
    Collision/CollisionSystem.cpp
    Spawner/SpawnerSystem.cpp
    Spawner/DataDrivenSpawnerSystem.cpp
    WaveManager/WaveManager.cpp
    Cleanup/CleanupSystem.cpp
    Destroy/DestroySystem.cpp
    Projectile/ProjectileSpawnerSystem.cpp
    EnemyShooting/EnemyShootingSystem.cpp
    ForcePod/ForcePodAttachmentSystem.cpp
    ForcePod/ForcePodLaunchSystem.cpp
    ForcePod/ForcePodShootingSystem.cpp
)

target_include_directories(rtype_game_server_systems PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(rtype_game_server_systems PUBLIC
    engine
    rtype_toml_parser
    network
)
