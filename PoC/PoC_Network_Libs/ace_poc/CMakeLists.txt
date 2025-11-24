# ACE PoC - Simplified Implementation
# This PoC demonstrates ACE-style patterns without requiring full ACE library
# to avoid complex build system issues

# UDP Server example (ACE-style Reactor pattern)
add_executable(ace_udp_server udp_server.cpp)
target_compile_features(ace_udp_server PRIVATE cxx_std_20)

# UDP Client example
add_executable(ace_udp_client udp_client.cpp)
target_compile_features(ace_udp_client PRIVATE cxx_std_20)

# Platform-specific linking
if(UNIX)
    target_link_libraries(ace_udp_server PRIVATE pthread)
    target_link_libraries(ace_udp_client PRIVATE pthread)
endif()

