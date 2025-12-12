add_executable(test_engine_factory
    test_GameEngineFactory.cpp
)

target_compile_features(test_engine_factory PRIVATE cxx_std_20)

target_include_directories(test_engine_factory PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_engine_factory PRIVATE
    GTest::gtest_main
    engine
    ecs
    rtype_game_server
)

include(GoogleTest)
gtest_discover_tests(test_engine_factory)

