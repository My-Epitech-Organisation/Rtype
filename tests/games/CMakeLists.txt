add_subdirectory(rtype)

# Additional branch coverage tests
add_executable(test_branch_coverage_additional test_branch_coverage_additional.cpp)

target_link_libraries(test_branch_coverage_additional PRIVATE
    rtype_game_shared
    rtype_game_server
    engine
    GTest::gtest_main
)

target_include_directories(test_branch_coverage_additional PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

include(GoogleTest)
if(WIN32 OR MSVC)
    gtest_discover_tests(test_branch_coverage_additional WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    gtest_discover_tests(test_branch_coverage_additional)
endif()
