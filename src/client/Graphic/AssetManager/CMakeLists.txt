add_library(rtype-graphic-asset-manager
    STATIC
    TextureManager.cpp
    TextureManager.hpp
    AssetManager.hpp
    FontManager.cpp
    FontManager.hpp
)

target_link_libraries(rtype-graphic-asset-manager
        PUBLIC
        SFML::Network
        SFML::Graphics
)