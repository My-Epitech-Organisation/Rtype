# Examples directory
# Add all example subdirectories here

# Check if qt_network_poc exists
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/qt_network_poc)
    add_subdirectory(qt_network_poc)
endif()


if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/asio_poc)
    add_subdirectory(asio_poc)
endif()
