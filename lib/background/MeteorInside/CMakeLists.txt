add_library(rtype-graphic-background-meteorInside SHARED
        entrypoint.cpp
        MeteorInside.cpp
        MeteorInside.hpp
)

target_include_directories(rtype-graphic-background-meteorInside PUBLIC
        ${CMAKE_SOURCE_DIR}/src/games/
        ${CMAKE_SOURCE_DIR}/src/games/rtype/shared/
        ${CMAKE_SOURCE_DIR}
)

target_link_libraries(rtype-graphic-background-meteorInside PRIVATE ecs)
