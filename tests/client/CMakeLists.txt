if(BUILD_GRAPHICAL_TESTS)
    add_executable(test_client_app test_client_app.cpp ${CMAKE_SOURCE_DIR}/src/client/ClientApp.cpp)

    target_link_libraries(test_client_app PRIVATE
        GTest::gtest_main
        engine
        network
        rtype_game_shared
        rtype_game_client
        rtype-graphic-SFML
        rtype_client_network
    )

    target_include_directories(test_client_app PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    include(GoogleTest)
    gtest_discover_tests(test_client_app)

    add_executable(test_graphic test_graphic.cpp)

    target_link_libraries(test_graphic PRIVATE
        GTest::gtest_main
        engine
        network
        rtype_game_shared
        rtype_game_client
        rtype-graphic-SFML
        rtype_client_network
    )

    target_include_directories(test_graphic PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    gtest_discover_tests(test_graphic)

    add_executable(test_graphic_systems test_systems.cpp)

    target_link_libraries(test_graphic_systems PRIVATE
        GTest::gtest_main
        engine
        network
        rtype_game_shared
        rtype_game_client
        rtype-graphic-SFML
        rtype_game_client_systems
        rtype_client_network
    )

    target_include_directories(test_graphic_systems PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    gtest_discover_tests(test_graphic_systems)

    add_executable(test_scene_manager test_scene_manager.cpp)

    target_link_libraries(test_scene_manager PRIVATE
        GTest::gtest_main
        engine
        network
        rtype_game_shared
        rtype_game_client
        rtype-graphic-SFML
        rtype_client_network
    )

    target_include_directories(test_scene_manager PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    gtest_discover_tests(test_scene_manager)

    # ==========================================================================
    # Stress Tests - Performance testing under heavy load
    # ==========================================================================

    # Rendering stress tests
    add_executable(test_stress_rendering test_stress_rendering.cpp)

    target_link_libraries(test_stress_rendering PRIVATE
        GTest::gtest_main
        engine
        network
        rtype_game_shared
        rtype_game_client
        rtype-graphic-SFML
        rtype_game_client_systems
        rtype_client_network
    )

    target_include_directories(test_stress_rendering PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    gtest_discover_tests(test_stress_rendering)

    # ECS component stress tests
    add_executable(test_stress_ecs test_stress_ecs.cpp)

    target_link_libraries(test_stress_ecs PRIVATE
        GTest::gtest_main
        engine
        network
        rtype_game_shared
        rtype_game_client
        rtype-graphic-SFML
        rtype_client_network
    )

    target_include_directories(test_stress_ecs PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    gtest_discover_tests(test_stress_ecs)

    # Event system stress tests
    add_executable(test_stress_events test_stress_events.cpp)

    target_link_libraries(test_stress_events PRIVATE
        GTest::gtest_main
        engine
        network
        rtype_game_shared
        rtype_game_client
        rtype-graphic-SFML
        rtype_game_client_systems
        rtype_client_network
    )

    target_include_directories(test_stress_events PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    gtest_discover_tests(test_stress_events)

    add_executable(test_entity_factory test_entity_factory.cpp)

    target_link_libraries(test_entity_factory PRIVATE
        GTest::gtest_main
        engine
        network
        rtype_game_shared
        rtype_game_client
        rtype-graphic-SFML
        rtype-graphic-entity-factory
        rtype_client_network
    )

    target_include_directories(test_entity_factory PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    gtest_discover_tests(test_entity_factory)

    add_executable(test_asset_managers test_asset_managers.cpp)

    target_link_libraries(test_asset_managers PRIVATE
        GTest::gtest_main
        engine
        network
        rtype_game_shared
        rtype_game_client
        rtype-graphic-SFML
        rtype-graphic-audio
    )

    add_executable(test_audio test_audio.cpp)
    target_link_libraries(test_audio PRIVATE
        rtype-graphic-audio
        rtype-graphic-asset-manager
        GTest::gtest_main
    )
    target_include_directories(test_audio PRIVATE
        ${CMAKE_SOURCE_DIR}
    )
    gtest_discover_tests(test_audio)

    add_executable(test_audio_more test_audio_more.cpp)
    target_link_libraries(test_audio_more PRIVATE
        rtype-graphic-audio
        rtype-graphic-asset-manager
        GTest::gtest_main
    )
    target_include_directories(test_audio_more PRIVATE
        ${CMAKE_SOURCE_DIR}
    )
    gtest_discover_tests(test_audio_more)

    add_executable(test_audio_branches test_audio_branches.cpp)
    target_link_libraries(test_audio_branches PRIVATE
        rtype-graphic-audio
        rtype-graphic-asset-manager
        GTest::gtest_main
    )
    target_include_directories(test_audio_branches PRIVATE
        ${CMAKE_SOURCE_DIR}
    )
    gtest_discover_tests(test_audio_branches)

    add_executable(test_controller_rumble test_controller_rumble.cpp)
    target_link_libraries(test_controller_rumble PRIVATE
        GTest::gtest_main
        rtype-graphic-SFML
    )
    target_include_directories(test_controller_rumble PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )
    gtest_discover_tests(test_controller_rumble)

    target_include_directories(test_asset_managers PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    gtest_discover_tests(test_asset_managers)

    # UI Components tests (Rectangle, Text, Button)
    add_executable(test_ui_components test_ui_components.cpp)

    target_link_libraries(test_ui_components PRIVATE
        GTest::gtest_main
        engine
        network
        rtype_game_shared
        rtype_game_client
        rtype-graphic-SFML
    )

    target_include_directories(test_ui_components PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    gtest_discover_tests(test_ui_components)

    add_executable(test_accessibility_and_input test_accessibility_and_input.cpp)

    target_link_libraries(test_accessibility_and_input PRIVATE
        GTest::gtest_main
        engine
        network
        rtype_game_shared
        rtype_game_client
        rtype-graphic-SFML
        rtype_game_client_systems
    )

    target_include_directories(test_accessibility_and_input PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    gtest_discover_tests(test_accessibility_and_input)

    add_executable(test_input_handler test_input_handler.cpp)

    target_link_libraries(test_input_handler PRIVATE
        GTest::gtest_main
        engine
        network
        rtype_game_shared
        rtype_game_client
        rtype-graphic-SFML
    )

    target_include_directories(test_input_handler PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    gtest_discover_tests(test_input_handler)

    add_executable(test_pause_menu test_pause_menu.cpp)

    target_link_libraries(test_pause_menu PRIVATE
        GTest::gtest_main
        engine
        network
        rtype_game_shared
        rtype_game_client
        rtype-graphic-SFML
    )

    target_include_directories(test_pause_menu PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    gtest_discover_tests(test_pause_menu)

endif()

add_executable(test_rtype_entity_factory test_rtype_entity_factory.cpp)

target_link_libraries(test_rtype_entity_factory PRIVATE
    GTest::gtest_main
    engine
    network
    rtype_game_shared
    rtype_game_client
    rtype_client_network
)

target_include_directories(test_rtype_entity_factory PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(test_rtype_entity_factory)
