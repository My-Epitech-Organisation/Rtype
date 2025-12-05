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
    rtype_common
)

set(SERVER_APP_TEST_SRCS test_server_app.cpp)
if(NOT WIN32)
    list(APPEND SERVER_APP_TEST_SRCS test_server_app_private.cpp)
endif()

add_executable(test_server_app ${SERVER_APP_TEST_SRCS}
    ${CMAKE_SOURCE_DIR}/src/server/ServerApp.cpp
    ${CMAKE_SOURCE_DIR}/src/server/ClientManager.cpp
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
    rtype_common
    rtype_network
    rtype_engine
)

include(GoogleTest)
gtest_discover_tests(test_client_manager)
gtest_discover_tests(test_server_app)
