add_executable(test_argparser test_argparser.cpp)

target_link_libraries(test_argparser PRIVATE
    GTest::gtest_main
)

target_include_directories(test_argparser PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

include(GoogleTest)
gtest_discover_tests(test_argparser)

add_executable(test_logger test_logger.cpp)

target_link_libraries(test_logger PRIVATE
    GTest::gtest_main
    rtype_common
)

target_include_directories(test_logger PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(test_logger)

add_executable(test_safe_queue SafeQueue_test.cpp)

target_link_libraries(test_safe_queue PRIVATE
    GTest::gtest_main
)

target_include_directories(test_safe_queue PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(test_safe_queue)

# Config and SaveManager tests
add_executable(test_config test_config.cpp)

target_link_libraries(test_config PRIVATE
    GTest::gtest_main
    rtype_game_config
)

target_include_directories(test_config PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/games/rtype/shared
)

gtest_discover_tests(test_config)

add_executable(test_savemanager test_savemanager.cpp)

target_link_libraries(test_savemanager PRIVATE
    GTest::gtest_main
    rtype_game_config
)

target_include_directories(test_savemanager PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/games/rtype/shared
)

gtest_discover_tests(test_savemanager)
