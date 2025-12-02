add_executable(test_safe_queue SafeQueue_test.cpp)
target_link_libraries(test_safe_queue gtest_main)
add_test(NAME SafeQueueTest COMMAND test_safe_queue)
add_executable(test_client_manager test_ClientManager.cpp
    ${CMAKE_SOURCE_DIR}/src/server/ClientManager.cpp
)

target_include_directories(test_client_manager PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/common
)

target_compile_features(test_client_manager PRIVATE cxx_std_20)

target_link_libraries(test_client_manager PRIVATE
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_client_manager)
