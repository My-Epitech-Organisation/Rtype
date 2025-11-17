add_executable(test_registry test_registry.cpp)

target_link_libraries(test_registry PRIVATE
    rtype_engine
    GTest::gtest_main
)

target_include_directories(test_registry PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

include(GoogleTest)
gtest_discover_tests(test_registry)
