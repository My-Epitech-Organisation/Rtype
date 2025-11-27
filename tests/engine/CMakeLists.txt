add_executable(test_time test_time.cpp)

target_link_libraries(test_time PRIVATE
    rtype_engine
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_time)
