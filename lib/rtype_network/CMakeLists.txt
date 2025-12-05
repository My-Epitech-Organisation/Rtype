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

add_library(rtype_network STATIC ${NETWORK_SOURCES})

target_include_directories(rtype_network PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../rtype_engine/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../rtype_engine_core/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../rtype_common/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../rtype_common/src/Logger
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(rtype_network PUBLIC cxx_std_20)

find_package(asio CONFIG REQUIRED)

target_link_libraries(rtype_network PUBLIC
    rtype_engine
    rtype_common
    asio::asio
)

if(WIN32)
    target_link_libraries(rtype_network PUBLIC ws2_32)
endif()

if(MSVC)
    target_compile_options(rtype_network PRIVATE /W4)
else()
    target_compile_options(rtype_network PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(RType::Network ALIAS rtype_network)
