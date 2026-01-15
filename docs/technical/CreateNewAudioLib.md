# Guide to Creating an Audio Library (LevelMusic)

This document explains how to create an audio library to manage the music for a specific level in R-Type.

## Prerequisites

Your library must be compiled as a dynamic library (`.so` on Linux, `.dll` on Windows).

## Interfaces to Implement

The main interface is `ILevelMusic`.
You can find its definition in `lib/audio/ILevelMusic.hpp`.

It is **strongly recommended** to inherit from the abstract class `ALevelMusic` (`lib/audio/ALevelMusic.hpp`). It already provides members to store the ECS registry and AssetManager, as well as fields to store the IDs of loaded music tracks (`_waveMusicId`, `_bossMusicId`).

### Base Class: `ALevelMusic`

If you inherit from `ALevelMusic`, you mainly need to implement:

1.  **Constructor**: Call the parent constructor.
2.  **`loadLevelMusic(std::shared_ptr<AudioLib> audioLib)`**: Method called to load and start the music. You will use the `audioLib` object or `_assetManager` to load audio files.

The `unloadLevelMusic()` method is already implemented in `ALevelMusic` and takes care of unloading the music tracks identified by `_waveMusicId` and `_bossMusicId`.

## Entry Point

Your dynamic library must expose a `createLevelMusic` function with C linkage (`extern "C"`).

### Signature

```cpp
extern "C" {
    ILevelMusic* createLevelMusic(std::shared_ptr<ECS::Registry> registry, std::shared_ptr<AssetManager> assetManager);
}
```

### Complete Example (Entrypoint)

```cpp
#include "MyLevelMusic.hpp"

#ifdef _WIN32
    #define RTYPE_EXPORT __declspec(dllexport)
#else
    #define RTYPE_EXPORT
#endif

extern "C" {
    RTYPE_EXPORT ILevelMusic *createLevelMusic(std::shared_ptr<ECS::Registry> registry, std::shared_ptr<AssetManager> assetManager) {
        return new MyLevelMusic(std::move(registry), std::move(assetManager));
    }
}
```

### Class Implementation Example

```cpp
#include "ALevelMusic.hpp"

class MyLevelMusic : public ALevelMusic {
public:
    MyLevelMusic(std::shared_ptr<ECS::Registry> registry, std::shared_ptr<AssetManager> assetManager)
        : ALevelMusic(std::move(registry), std::move(assetManager), "Level1Music") {}

    void loadLevelMusic(std::shared_ptr<AudioLib> audioLib) override {
        // Loading the music
        // Make sure you have the files in assets/audio/
        
        // Hypothetical usage example via assetManager or audioLib
        // audioLib->playMusic("my_music_theme"); 
        
        // If you load resources dynamically, store their IDs
        // in _waveMusicId or _bossMusicId so that unloadLevelMusic() can clean them up.
         _waveMusicId = "assets/audio/my_theme.ogg";
         _assetManager->audioManager->loadMusic(_waveMusicId, _waveMusicId);
         _assetManager->audioManager->playMusic(_waveMusicId);
    }
};

## Compilation and Installation

To use this library in R-Type, it must be compiled as a shared library and placed in the `plugins/music/` directory.

### CMake Example

```cmake
add_library(MyLevelMusic SHARED
    entrypoint.cpp
    MyLevelMusic.cpp
    # ... other source files
)

# Link necessary libraries
target_link_libraries(MyLevelMusic PRIVATE ...)
```

### Installation Path

The compiled shared library must be placed in:
`plugins/music/<YourLibName>.so` (or `.dll` on Windows)

Example: `plugins/music/Level1Music.so`

The game will automatically detect and list available level musics from this directory.
```
