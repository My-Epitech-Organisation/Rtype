add_library(rtype-graphic-audio STATIC
    AudioLib.cpp
    AudioLib.hpp
)

target_link_libraries(rtype-graphic-audio
    PUBLIC
    SFML::Audio
)
