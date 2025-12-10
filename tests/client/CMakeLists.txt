if(BUILD_GRAPHICAL_TESTS)
    add_executable(test_client_app test_client_app.cpp ${CMAKE_SOURCE_DIR}/src/client/ClientApp.cpp)

    target_link_libraries(test_client_app PRIVATE
        GTest::gtest_main
        rtype_engine
        rtype_network
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
        rtype_engine
        rtype_network
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
        rtype_engine
        rtype_network
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
        rtype_engine
        rtype_network
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
        rtype_engine
        rtype_network
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
        rtype_engine
        rtype_network
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
        rtype_engine
        rtype_network
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

endif()

add_executable(test_entity_factory test_entity_factory.cpp)

target_link_libraries(test_entity_factory PRIVATE
    GTest::gtest_main
    rtype_engine
    rtype_network
    rtype_game_shared
    rtype_game_client
    rtype-graphic-SFML
    rtype_client_network
)

target_include_directories(test_entity_factory PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(test_entity_factory)

# UI Components tests (Rectangle, Text, Button)
add_executable(test_ui_components test_ui_components.cpp)

target_link_libraries(test_ui_components PRIVATE
    GTest::gtest_main
    rtype_engine
    rtype_network
    rtype_game_shared
    rtype_game_client
    rtype-graphic-SFML
)

target_include_directories(test_ui_components PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(test_ui_components)
