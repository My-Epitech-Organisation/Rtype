add_library(network STATIC
    # Transport layer (Asio-based async UDP)
    transport/AsioUdpSocket.cpp

    # Reliability layer (RUDP per RFC RTGP v1.1.0)
    reliability/ReliableChannel.cpp

    # Connection layer (State Machine + high-level API)
    connection/ConnectionStateMachine.cpp
    connection/Connection.cpp

    # Compression layer (LZ4)
    compression/Compressor.cpp

    UdpSocket.cpp
    Packet.cpp
    Serializer.cpp
)

# Protocol types are header-only (no .cpp files needed)
# - protocol/OpCode.hpp
# - protocol/Header.hpp
# - protocol/Payloads.hpp
# - protocol/Validator.hpp

# Find Asio (standalone, non-Boost)
rtype_find_asio()

# Find LZ4 for compression
find_package(lz4 CONFIG REQUIRED)

target_include_directories(network PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(network PUBLIC cxx_std_20)

# Network depends on engine, Asio, and LZ4
target_link_libraries(network PUBLIC
    engine
    asio::asio
    lz4::lz4
)

# Windows-specific: link Winsock2 for byte order functions (htons, htonl, etc.)
if(WIN32)
    target_link_libraries(network PUBLIC ws2_32)
endif()
