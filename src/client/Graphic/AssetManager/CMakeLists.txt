add_library(rtype-graphic-asset-manager
    STATIC
    IAssetManager.hpp
    AAssetManager.hpp
    TextureManager.cpp
    TextureManager.hpp
    AssetManager.hpp
    AssetManager.cpp
    AudioManager.hpp
    AudioManager.cpp
    FontManager.cpp
    FontManager.hpp
    SoundManager.cpp
    SoundManager.hpp
)

target_link_libraries(rtype-graphic-asset-manager
        PUBLIC
        common
)

target_include_directories(rtype-graphic-asset-manager
    PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/games/rtype/shared
)
