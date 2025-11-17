add_executable(test_serialization test_serialization.cpp)

target_link_libraries(test_serialization PRIVATE
    rtype_network
    GTest::gtest_main
)

target_include_directories(test_serialization PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

include(GoogleTest)
gtest_discover_tests(test_serialization)
