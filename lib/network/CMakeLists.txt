# ============================================================================
# Network Library (Static)
# ============================================================================

set(NETWORK_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/transport/AsioUdpSocket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/reliability/ReliableChannel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/connection/ConnectionStateMachine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/connection/Connection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UdpSocket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Packet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Serializer.cpp
)

add_library(network STATIC ${NETWORK_SOURCES})

target_include_directories(network PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/src/Logger
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(network PUBLIC cxx_std_20)

rtype_find_asio()

target_link_libraries(network PUBLIC
    engine
    common
    asio::asio
)

if(WIN32)
    target_link_libraries(network PUBLIC ws2_32)
endif()

if(MSVC)
    target_compile_options(network PRIVATE /W4)
else()
    target_compile_options(network PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(RType::Network ALIAS network)
