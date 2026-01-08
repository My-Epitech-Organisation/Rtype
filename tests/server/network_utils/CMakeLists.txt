add_executable(test_network_utils
    test_network_utils.cpp
)

target_link_libraries(test_network_utils
    PRIVATE
    gtest_main
    rtype_server_core
)

if(WIN32)
    target_link_libraries(test_network_utils PRIVATE ws2_32)
endif()

add_test(NAME test_network_utils COMMAND test_network_utils)
