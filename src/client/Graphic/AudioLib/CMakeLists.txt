add_library(rtype-graphic-audio STATIC
    AudioLib.cpp
    AudioLib.hpp
)

target_link_libraries(rtype-graphic-audio
    PUBLIC
)

target_include_directories(rtype-graphic-audio
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)
