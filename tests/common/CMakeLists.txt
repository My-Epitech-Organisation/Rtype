add_executable(test_argparser test_argparser.cpp)

target_link_libraries(test_argparser PRIVATE
    GTest::gtest_main
    common
)

target_include_directories(test_argparser PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/lib/common/src
    ${CMAKE_SOURCE_DIR}/lib
)

include(GoogleTest)
gtest_discover_tests(test_argparser)

add_executable(test_logger test_logger.cpp)

target_link_libraries(test_logger PRIVATE
    GTest::gtest_main
    common
)

target_include_directories(test_logger PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/lib
)

gtest_discover_tests(test_logger)

add_executable(test_safe_queue SafeQueue_test.cpp)

target_link_libraries(test_safe_queue PRIVATE
    GTest::gtest_main
)

target_include_directories(test_safe_queue PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/lib/common/src
    ${CMAKE_SOURCE_DIR}/lib
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

# Branch coverage tests for Config
add_executable(test_config_branches test_config_branches.cpp)

target_link_libraries(test_config_branches PRIVATE
    GTest::gtest_main
    rtype_game_config
)

target_include_directories(test_config_branches PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/games/rtype/shared
)

gtest_discover_tests(test_config_branches)

# Branch coverage tests for TomlParser
add_executable(test_tomlparser_branches test_tomlparser_branches.cpp)

target_link_libraries(test_tomlparser_branches PRIVATE
    GTest::gtest_main
    rtype_game_config
)

target_include_directories(test_tomlparser_branches PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/games/rtype/shared
)

gtest_discover_tests(test_tomlparser_branches)

add_executable(test_scene_exception test_scene_exception.cpp)

target_link_libraries(test_scene_exception PRIVATE
    GTest::gtest_main
    common
)

target_include_directories(test_scene_exception PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/lib
)

gtest_discover_tests(test_scene_exception)

# Branch coverage tests for SaveManager
add_executable(test_savemanager_branches test_savemanager_branches.cpp)

target_link_libraries(test_savemanager_branches PRIVATE
    GTest::gtest_main
    rtype_game_config
)

target_include_directories(test_savemanager_branches PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/games/rtype/shared
)

gtest_discover_tests(test_savemanager_branches)

# Branch coverage tests for FileOperations
add_executable(test_fileoperations_branches test_fileoperations_branches.cpp)

target_link_libraries(test_fileoperations_branches PRIVATE
    GTest::gtest_main
    rtype_game_config
)

target_include_directories(test_fileoperations_branches PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/games/rtype/shared
)

gtest_discover_tests(test_fileoperations_branches)

# Branch coverage tests for RTypeConfigParser
add_executable(test_configparser_branches test_configparser_branches.cpp)

target_link_libraries(test_configparser_branches PRIVATE
    GTest::gtest_main
    rtype_game_config
)

target_include_directories(test_configparser_branches PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/games/rtype/shared
)

gtest_discover_tests(test_configparser_branches)

# Branch coverage tests for BinarySerializer
add_executable(test_binaryserializer_branches test_binaryserializer_branches.cpp)

target_link_libraries(test_binaryserializer_branches PRIVATE
    GTest::gtest_main
    rtype_game_config
)

target_include_directories(test_binaryserializer_branches PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/games/rtype/shared
)

gtest_discover_tests(test_binaryserializer_branches)
