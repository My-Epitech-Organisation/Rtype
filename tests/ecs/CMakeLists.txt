add_executable(test_registry test_registry.cpp)

target_link_libraries(test_registry PRIVATE
    rtype_engine
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_registry)
