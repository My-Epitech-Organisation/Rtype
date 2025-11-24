# Boost.Asio PoC

# Add Boost via CPM
CPMAddPackage(
    NAME Boost
    VERSION 1.84.0
    URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
    OPTIONS
        "BOOST_ENABLE_CMAKE ON"
        "BOOST_INCLUDE_LIBRARIES asio\;system\;date_time"
)

# UDP Server example
add_executable(boost_udp_server udp_server.cpp)
target_link_libraries(boost_udp_server PRIVATE Boost::asio Boost::system)
target_compile_features(boost_udp_server PRIVATE cxx_std_20)

# UDP Client example
add_executable(boost_udp_client udp_client.cpp)
target_link_libraries(boost_udp_client PRIVATE Boost::asio Boost::system)
target_compile_features(boost_udp_client PRIVATE cxx_std_20)

# Platform-specific linking
if(WIN32)
    target_link_libraries(boost_udp_server PRIVATE ws2_32 wsock32)
    target_link_libraries(boost_udp_client PRIVATE ws2_32 wsock32)
else()
    target_link_libraries(boost_udp_server PRIVATE pthread)
    target_link_libraries(boost_udp_client PRIVATE pthread)
endif()
