
# Why not use SDL "as-is" for the graphics layer

- SDL is primarily a C API: low-level and procedural. The exposed functions require manual resource management (create/destroy), explicit error handling, and lifecycle bookkeeping.
- For a modern C++ project, it's better to encapsulate the C API behind RAII abstractions and typed interfaces. That reduces leaks, lifetime bugs and makes testing and replacement easier.

Observations

- SDL does not provide native C++ objects: it uses pointers/structs and free functions.
- Many repeated create/destroy calls (window, renderer, texture, surface, etc.).
- Error handling is manual (return codes or NULL) and often scattered across the codebase.
- If you don't encapsulate, SDL types (e.g. `SDL_Window *`) will leak into your engine code and create strong coupling to SDL.

Recommendations

- Encapsulate SDL behind well-defined C++ classes/facades: Window, Renderer, Texture, Surface, EventDispatcher, etc.
- Never expose raw SDL pointers in your engine's public interfaces; map them to opaque types or interfaces (e.g. `IWindow`, `IRenderer`).
- Use RAII for lifetimes: constructors do init/alloc, destructors cleanup. This reduces error surface area.
- Centralize initialization/cleanup (for example, an `SDLContext` responsible for `SDL_Init`/`SDL_Quit`).
- Choose an error strategy: use C++ exceptions for critical failures (init), or result types/boolean returns for non-critical operations.

Minimal contract (inputs/outputs/errors)

- Inputs: creation parameters (width, height, flags, title), pixel / texture data, user events.
- Outputs: visible window, basic 2D rendering, events translated into engine-agnostic types.
- Error modes: exceptions for creation failures; return codes or optional results for recoverable errors (resource load failure, etc.).

```cpp
#include <SDL.h>
#include <stdexcept>
#include <iostream>

class SDLContext {
public:
    SDLContext() {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
        }
    }
    ~SDLContext() {
        SDL_Quit();
    }
};

class Window {
public:
    Window(const char* title, int w, int h) {
        window_ = SDL_CreateWindow(title,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            w, h,
            SDL_WINDOW_SHOWN);
        if (!window_) throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());

        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!renderer_) throw std::runtime_error(std::string("SDL_CreateRenderer failed: ") + SDL_GetError());
    }
    ~Window() {
        if (renderer_) SDL_DestroyRenderer(renderer_);
        if (window_) SDL_DestroyWindow(window_);
    }

    SDL_Renderer* renderer() const { return renderer_; }

private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
};

int main(int argc, char** argv) {
    try {
        SDLContext ctx; // centralized init/quit
        Window win("Demo", 800, 600);

        bool running = true;
        SDL_Event ev;
        while (running) {
            while (SDL_PollEvent(&ev)) {
                if (ev.type == SDL_QUIT) running = false;
            }

            SDL_SetRenderDrawColor(win.renderer(), 0, 0, 50, 255);
            SDL_RenderClear(win.renderer());

            // Draw stuff here (textures, shapes...)

            SDL_RenderPresent(win.renderer());
            SDL_Delay(16); // ~60 FPS (simple)
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}
```
