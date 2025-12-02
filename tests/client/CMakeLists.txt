add_executable(test_client_app test_client_app.cpp ${CMAKE_SOURCE_DIR}/src/client/ClientApp.cpp)

target_link_libraries(test_client_app PRIVATE
    GTest::gtest_main
    rtype_engine
    rtype_network
    rtype_game_shared
    rtype_game_client
    rtype-graphic-SFML
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
)

target_include_directories(test_graphic PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(test_graphic)

add_executable(test_systems test_systems.cpp)

target_link_libraries(test_systems PRIVATE
    GTest::gtest_main
    rtype_engine
    rtype_network
    rtype_game_shared
    rtype_game_client
    rtype-graphic-SFML
    rtype-client-system
)

target_include_directories(test_systems PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(test_systems)

add_executable(test_scene_manager test_scene_manager.cpp)

target_link_libraries(test_scene_manager PRIVATE
    GTest::gtest_main
    rtype_engine
    rtype_network
    rtype_game_shared
    rtype_game_client
    rtype-graphic-SFML
)

target_include_directories(test_scene_manager PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(test_scene_manager)

add_executable(test_entity_factory test_entity_factory.cpp)

target_link_libraries(test_entity_factory PRIVATE
    GTest::gtest_main
    rtype_engine
    rtype_network
    rtype_game_shared
    rtype_game_client
    rtype-graphic-SFML
)

target_include_directories(test_entity_factory PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(test_entity_factory)

add_executable(test_components test_components.cpp)

target_link_libraries(test_components PRIVATE
    GTest::gtest_main
)

target_include_directories(test_components PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(test_components)

