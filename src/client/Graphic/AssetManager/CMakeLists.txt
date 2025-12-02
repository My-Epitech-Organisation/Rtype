add_library(rtype-graphic-asset-manager
    STATIC
    TextureManager.cpp
    TextureManager.hpp
    AssetManager.hpp
    AssetManager.cpp
    FontManager.cpp
    FontManager.hpp
)

target_link_libraries(rtype-graphic-asset-manager
        PUBLIC
        SFML::Network
        SFML::Graphics
)

target_include_directories(rtype-graphic-asset-manager
    PUBLIC
    ${CMAKE_SOURCE_DIR}
)