# ============================================================================
# ECS UNIT TESTS
# ============================================================================

include(GoogleTest)

# List of all ECS test files (without .cpp extension)
set(ECS_TESTS
    # Core tests
    core/test_entity
    core/test_registry_entity
    core/test_registry_component
    core/test_registry_singleton
    core/test_registry_view
    core/test_registry_signal
    core/test_registry_relationship
    # Storage tests
    storage/test_isparse_set
    storage/test_sparse_set
    # Traits tests
    traits/test_component_traits
)

# Function to create a test executable with common configuration
function(add_ecs_test TEST_PATH)
    get_filename_component(TEST_NAME ${TEST_PATH} NAME)
    add_executable(${TEST_NAME} ${TEST_PATH}.cpp)
    target_link_libraries(${TEST_NAME} PRIVATE
        rtype_engine
        GTest::gtest_main
    )
    target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    gtest_discover_tests(${TEST_NAME})
endfunction()

# Create all test executables
foreach(TEST_PATH IN LISTS ECS_TESTS)
    add_ecs_test(${TEST_PATH})
endforeach()

