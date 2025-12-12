add_executable(test_client_manager test_ClientManager.cpp
    ${CMAKE_SOURCE_DIR}/src/server/ClientManager.cpp
)

target_include_directories(test_client_manager PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(test_client_manager PRIVATE cxx_std_20)

target_link_libraries(test_client_manager PRIVATE
    GTest::gtest_main
    common
    rtype_toml_parser
)

set(SERVER_APP_TEST_SRCS test_server_app.cpp)
list(APPEND SERVER_APP_TEST_SRCS test_server_app_extra.cpp)
if(NOT WIN32)
    list(APPEND SERVER_APP_TEST_SRCS test_server_app_private.cpp)
endif()

add_executable(test_server_app ${SERVER_APP_TEST_SRCS}
    ${CMAKE_SOURCE_DIR}/src/server/ServerApp.cpp
    ${CMAKE_SOURCE_DIR}/src/server/ClientManager.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/NetworkServer.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/ServerNetworkSystem.cpp
)

target_include_directories(test_server_app PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(test_server_app PRIVATE cxx_std_20)

target_link_libraries(test_server_app PRIVATE
    GTest::gtest_main
    common
    rtype_toml_parser
    network
    engine
    rtype_game_server
    rtype_game_shared
)

# ServerApp GameConfig integration tests
add_executable(test_server_app_gameconfig test_server_app_gameconfig.cpp
    ${CMAKE_SOURCE_DIR}/src/server/ServerApp.cpp
    ${CMAKE_SOURCE_DIR}/src/server/ClientManager.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/NetworkServer.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/ServerNetworkSystem.cpp
)

target_include_directories(test_server_app_gameconfig PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/common
)

target_compile_features(test_server_app_gameconfig PRIVATE cxx_std_20)

target_link_libraries(test_server_app_gameconfig PRIVATE
    GTest::gtest_main
    common
    rtype_toml_parser
    network
    engine
    rtype_game_server
    rtype_game_shared
)

# ServerNetworkSystem unit tests
add_executable(test_server_network_system test_ServerNetworkSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/ServerNetworkSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/NetworkServer.cpp
)

target_include_directories(test_server_network_system PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(test_server_network_system PRIVATE cxx_std_20)

target_link_libraries(test_server_network_system PRIVATE
    GTest::gtest_main
    common
    rtype_toml_parser
    network
    ecs
    rtype_game_shared
)

# Unit tests for ServerApp
add_executable(test_server_app_unit
    ${CMAKE_CURRENT_SOURCE_DIR}/test_server_app_unit.cpp
    ${CMAKE_SOURCE_DIR}/src/server/ServerApp.cpp
    ${CMAKE_SOURCE_DIR}/src/server/ClientManager.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/NetworkServer.cpp
    ${CMAKE_SOURCE_DIR}/src/server/network/ServerNetworkSystem.cpp
)

target_include_directories(test_server_app_unit PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(test_server_app_unit PRIVATE cxx_std_20)

target_link_libraries(test_server_app_unit PRIVATE
    GTest::gtest_main
    common
    rtype_toml_parser
    network
    engine
    rtype_game_server
    rtype_game_shared
)

add_executable(test_server_network_messages test_server_network_messages.cpp)

target_include_directories(test_server_network_messages PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/common
)

target_compile_features(test_server_network_messages PRIVATE cxx_std_20)

target_link_libraries(test_server_network_messages PRIVATE
    GTest::gtest_main
    common
    rtype_toml_parser
    network
    rtype_client_network
    rtype_server_network
)

include(GoogleTest)
gtest_discover_tests(test_server_network_messages)
gtest_discover_tests(test_client_manager)
gtest_discover_tests(test_server_app)
gtest_discover_tests(test_server_app_gameconfig)
gtest_discover_tests(test_server_network_system)
gtest_discover_tests(test_server_app_unit)
