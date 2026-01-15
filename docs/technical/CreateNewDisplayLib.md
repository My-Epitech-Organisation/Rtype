# Guide to Creating a Display Library

This document explains how to implement a new display library for the R-Type engine.

## Prerequisites

Your library must be compiled as a dynamic library (`.so` on Linux, `.dll` on Windows).

## Interfaces to Implement

The main interface is `rtype::display::IDisplay`. It defines methods for window management, event handling, and rendering.
You can find the definitions in `include/rtype/display/IDisplay.hpp`.

In addition to `IDisplay`, you will likely need to implement interfaces for graphic and audio resources returned by your loading methods:
*   `rtype::display::ITexture`
*   `rtype::display::IFont`
*   `rtype::display::ISoundBuffer`
*   `rtype::display::ISound`
*   `rtype::display::IMusic`

An abstract class `rtype::display::ADisplay` is provided in `lib/display/ADisplay.hpp` and can serve as a common base (it handles some basic attributes like window size), but it is not mandatory.

## Entry Point

Your dynamic library must expose a `createInstanceDisplay` function with C linkage (`extern "C"`) to allow the engine to instantiate it dynamically.

### Signature

```cpp
extern "C" {
    rtype::display::IDisplay* createInstanceDisplay(void);
}
```

### Complete Example (Entrypoint)

```cpp
#include "MyDisplayLib.hpp"

#ifdef _WIN32
    #define RTYPE_EXPORT __declspec(dllexport)
#else
    #define RTYPE_EXPORT
#endif

extern "C" {
    RTYPE_EXPORT rtype::display::IDisplay* createInstanceDisplay(void) {
        return new MyDisplayLib(); // Your class implementing IDisplay
    }
}
```

## Recommended Structure

It is recommended to structure your library as follows:
*   `MyDisplayLib.hpp`/`.cpp`: Your main implementation of `IDisplay`.
*   Classes implementing `ITexture`, `IFont`, etc.
*   `entrypoint.cpp`: Contains the exported function.
*   `CMakeLists.txt`: For compilation into a shared library.

## Key Methods of `IDisplay`

Here are some key responsibilities your class must handle:
*   **open/close**: Creation and destruction of the window.
*   **pollEvent**: Conversion of events from your underlying library (SDL, SFML, Raylib...) to the `rtype::display::Event` structure.
*   **drawSprite/drawText/drawRectangle**: Rendering primitives.
*   **loadTexture/loadFont/...**: Loading resources from disk and internal storage.
*   **getTexture/getFont/...**: Accessing loaded resources.

## Compilation and Installation

To use this library in R-Type, it must be compiled as a shared library and placed in the appropriate directory.

### CMake Example

```cmake
add_library(my_display SHARED
    entrypoint.cpp
    MyDisplayLib.cpp
    # ... other source files
)

# Link necessary libraries (e.g., SDL2, SFML, etc.)
target_link_libraries(my_display PRIVATE ...)

# Set the output name used by the game (display.so)
set_target_properties(my_display PROPERTIES OUTPUT_NAME "display")
```

### Installation Path

The compiled shared library must be placed in:
`plugins/display.so` (or `plugins/display.dll` on Windows)

The game will search for this specific filename to load the display module.
