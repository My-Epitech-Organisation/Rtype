# Examples directory
# Add all example subdirectories here

# Check if qt_network_poc exists
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/qt_network_poc)
    add_subdirectory(qt_network_poc)
endif()

# Check if boost_asio_poc exists
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/boost_asio_poc)
    add_subdirectory(boost_asio_poc)
endif()

# Check if asio_poc exists
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/asio_poc)
    add_subdirectory(asio_poc)
endif()
