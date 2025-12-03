add_library(rtype_network STATIC
    # Transport layer (Asio-based async UDP)
    transport/AsioUdpSocket.cpp

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
find_package(asio CONFIG REQUIRED)

target_include_directories(rtype_network PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol
)

target_compile_features(rtype_network PUBLIC cxx_std_20)

# Network depends on engine and Asio
target_link_libraries(rtype_network PUBLIC
    rtype_engine
    asio::asio
)

# Windows-specific: link Winsock2 for byte order functions (htons, htonl, etc.)
if(WIN32)
    target_link_libraries(rtype_network PUBLIC ws2_32)
endif()
