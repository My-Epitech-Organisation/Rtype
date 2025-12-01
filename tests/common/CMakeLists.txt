add_executable(test_argparser test_argparser.cpp)

target_link_libraries(test_argparser PRIVATE
    GTest::gtest_main
)

target_include_directories(test_argparser PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

include(GoogleTest)
gtest_discover_tests(test_argparser)
