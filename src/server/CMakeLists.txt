
add_library(rtype_server_network STATIC
    network/NetworkServer.cpp
    network/ServerNetworkSystem.cpp
)

target_compile_features(rtype_server_network PRIVATE cxx_std_20)
target_include_directories(rtype_server_network PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(rtype_server_network
    PRIVATE
    network
    common
    rtype_game_server
)

add_executable(r-type_server
    main.cpp
    ServerApp.cpp
    ClientManager.cpp
)

target_compile_features(r-type_server PRIVATE cxx_std_20)

target_include_directories(r-type_server PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Server executable depends on all server-side modules
target_link_libraries(r-type_server PRIVATE
    engine
    network
    rtype_game_shared
    rtype_game_server
    rtype_server_network
)
