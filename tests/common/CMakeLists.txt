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
if(WIN32 OR MSVC)
    gtest_discover_tests(test_argparser WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_argparser)
endif()

add_executable(test_logger test_logger.cpp)

target_link_libraries(test_logger PRIVATE
    GTest::gtest_main
    common
)

target_include_directories(test_logger PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/lib
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_logger WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_logger)
endif()

add_executable(test_safe_queue SafeQueue_test.cpp)

target_link_libraries(test_safe_queue PRIVATE
    GTest::gtest_main
)

target_include_directories(test_safe_queue PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/lib/common/src
    ${CMAKE_SOURCE_DIR}/lib
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_safe_queue WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_safe_queue)
endif()

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

if(WIN32 OR MSVC)
    gtest_discover_tests(test_config WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_config)
endif()

add_executable(test_savemanager test_savemanager.cpp)

target_link_libraries(test_savemanager PRIVATE
    GTest::gtest_main
    rtype_game_config
)

target_include_directories(test_savemanager PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/games/rtype/shared
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_savemanager WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_savemanager)
endif()

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

if(WIN32 OR MSVC)
    gtest_discover_tests(test_config_branches WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_config_branches)
endif()

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

if(WIN32 OR MSVC)
    gtest_discover_tests(test_tomlparser_branches WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_tomlparser_branches)
endif()

add_executable(test_scene_exception test_scene_exception.cpp)

target_link_libraries(test_scene_exception PRIVATE
    GTest::gtest_main
    common
)

target_include_directories(test_scene_exception PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/lib
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_scene_exception WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_scene_exception)
endif()

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

if(WIN32 OR MSVC)
    gtest_discover_tests(test_savemanager_branches WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_savemanager_branches)
endif()

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

if(WIN32 OR MSVC)
    gtest_discover_tests(test_fileoperations_branches WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_fileoperations_branches)
endif()

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

if(WIN32 OR MSVC)
    gtest_discover_tests(test_configparser_branches WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_configparser_branches)
endif()

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

if(WIN32 OR MSVC)
    gtest_discover_tests(test_binaryserializer_branches WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_binaryserializer_branches)
endif()
# TomlParser tests (from config subdirectory)
add_executable(test_toml_parser config/test_toml_parser.cpp)

target_link_libraries(test_toml_parser PRIVATE
    GTest::gtest_main
    rtype_game_config
)

target_include_directories(test_toml_parser PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/games/rtype/shared
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_toml_parser WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_toml_parser)
endif()

# LogCategory tests (from logger subdirectory)
add_executable(test_log_category logger/test_log_category.cpp)

target_link_libraries(test_log_category PRIVATE
    GTest::gtest_main
    common
)

target_include_directories(test_log_category PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/lib
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_log_category WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_log_category)
endif()

# Timestamp tests (from logger subdirectory)
add_executable(test_timestamp logger/test_timestamp.cpp)

target_link_libraries(test_timestamp PRIVATE
    GTest::gtest_main
    common
)

target_include_directories(test_timestamp PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/lib
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_timestamp WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_timestamp)
endif()

# Types tests (from types subdirectory)
add_executable(test_types types/test_types.cpp)

target_link_libraries(test_types PRIVATE
    GTest::gtest_main
    common
)

target_include_directories(test_types PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/lib
)

if(WIN32 OR MSVC)
    gtest_discover_tests(test_types WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_types)
endif()