add_executable(test_registry test_registry.cpp)

target_link_libraries(test_registry PRIVATE
    rtype_engine
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_registry)

add_executable(test_entity test_entity.cpp)

target_link_libraries(test_entity PRIVATE
    rtype_engine
    GTest::gtest_main
)

gtest_discover_tests(test_entity)
