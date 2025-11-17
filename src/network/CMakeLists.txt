add_library(rtype_network STATIC
    UdpSocket.cpp
    Packet.cpp
    Serializer.cpp
)

target_include_directories(rtype_network PUBLIC ${CMAKE_SOURCE_DIR}/include)
