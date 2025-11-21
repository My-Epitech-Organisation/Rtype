add_executable(r-type_client main.cpp ClientApp.cpp)

target_compile_features(r-type_client PRIVATE cxx_std_20)

# Client executable depends on all client-side modules
target_link_libraries(r-type_client PRIVATE
    rtype_engine
    rtype_network
    rtype_game_shared
    rtype_game_client
)
