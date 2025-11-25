add_library(rtype_network STATIC
    UdpSocket.cpp
    Packet.cpp
    Serializer.cpp
)

target_include_directories(rtype_network PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(rtype_network PUBLIC cxx_std_20)

# Network depends on engine
target_link_libraries(rtype_network PUBLIC
    rtype_engine
)
