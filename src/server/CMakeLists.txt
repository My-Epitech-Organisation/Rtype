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
    rtype_engine
    rtype_network
    rtype_game_shared
    rtype_game_server
)
