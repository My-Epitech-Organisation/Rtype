add_subdirectory(Graphic)

add_executable(r-type_client
        main.cpp
        ClientApp.cpp
        GameAction.hpp
)

target_compile_features(r-type_client PRIVATE cxx_std_20)

target_include_directories(r-type_client PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(r-type_client PUBLIC
    rtype_engine
    rtype_network
    rtype_game_shared
    rtype_game_client
    rtype-graphic-SFML
)
