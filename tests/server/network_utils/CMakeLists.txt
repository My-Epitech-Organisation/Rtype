if(NOT WIN32)
    add_executable(test_network_utils
        test_network_utils.cpp
    )

    target_link_libraries(test_network_utils
        PRIVATE
        gtest_main
        rtype_server_core
    )

    add_test(NAME test_network_utils COMMAND test_network_utils)
else()
    message(STATUS "Skipping test_network_utils on Windows")
endif()
