---
sidebar_position: 1
---

# Graphics Library Selection

## Executive Summary

**Decision:** SFML (Simple and Fast Multimedia Library)  
**Date:** November 2025  
**Status:** ✅ Approved

After evaluating **SFML, SDL3, and Raylib**, SFML was chosen for its native C++ design, object-oriented API, and focus on 2D graphics.

---

## Comparison

| Library | Language | API Style | 2D Focus | Complexity |
|---------|----------|-----------|----------|------------|
| **SFML** | Native C++ | Object-Oriented | ✅ Yes | Low |
| SDL3 | C (C++ wrappers needed) | Procedural | ⚠️ General | Medium |
| Raylib | C (C++ wrappers needed) | Procedural | ⚠️ General | Low |

---

## Why SFML?

### 1. Native C++ Library ✅

**No need for C wrappers:**

```cpp
// SFML (native C++)
sf::RenderWindow window(sf::VideoMode(1920, 1080), "R-Type");
sf::Sprite sprite;
sprite.setTexture(texture);
window.draw(sprite);

// SDL3 (C API, needs wrapping)
SDL_Window* window = SDL_CreateWindow("R-Type", 1920, 1080, 0);
SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
// ... manual resource management
```

**Benefits:**

- ✅ No manual resource management (RAII)
- ✅ Exception safety built-in
- ✅ Modern C++ idioms
- ✅ No boilerplate wrapper code

---

### 2. Object-Oriented Design ✅

**Clean, intuitive API:**

```cpp
// Window management
sf::RenderWindow window(sf::VideoMode(1920, 1080), "R-Type");
window.setFramerateLimit(60);
window.setVerticalSyncEnabled(true);

// Sprite handling
sf::Sprite player;
player.setTexture(playerTexture);
player.setPosition(100.f, 200.f);
player.setScale(2.f, 2.f);
player.rotate(45.f);

// Drawing
window.clear();
window.draw(player);
window.display();
```

**vs SDL (procedural):**

```cpp
SDL_Window* window = SDL_CreateWindow("R-Type", 
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    1920, 1080, SDL_WINDOW_SHOWN);
SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 
    SDL_RENDERER_ACCELERATED);

SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
SDL_Rect dest = {100, 200, 64, 64};
SDL_RenderCopy(renderer, texture, nullptr, &dest);

// Manual cleanup
SDL_DestroyTexture(texture);
SDL_DestroyRenderer(renderer);
SDL_DestroyWindow(window);
```

---

### 3. 2D Graphics Focus ✅

**Designed specifically for 2D games:**

- ✅ **Sprite batching** optimized for 2D
- ✅ **Text rendering** built-in with TrueType fonts
- ✅ **View/Camera system** for 2D scrolling
- ✅ **Shape primitives** (rectangles, circles, polygons)

**Perfect for R-Type's side-scrolling shooter:**

```cpp
// Scrolling background
sf::View camera(sf::FloatRect(0, 0, 1920, 1080));
camera.move(scrollSpeed * dt, 0);  // Scroll right
window.setView(camera);

// Parallax layers
window.draw(backgroundLayer1);  // Slow scroll
camera.move(scrollSpeed * 0.5f * dt, 0);
window.draw(backgroundLayer2);  // Medium scroll
```

---

### 4. Comprehensive Feature Set ✅

**Everything needed for R-Type:**

| Feature | SFML | SDL3 | Raylib |
|---------|------|------|--------|
| **2D Graphics** | ✅ Excellent | ✅ Good | ✅ Good |
| **Audio** | ✅ Built-in | ⚠️ SDL_mixer | ✅ Built-in |
| **Input** | ✅ Keyboard/Mouse/Joystick | ✅ Yes | ✅ Yes |
| **Networking** | ✅ TCP/UDP sockets | ❌ No | ❌ No |
| **Window Management** | ✅ Full control | ✅ Full control | ✅ Full control |

**Audio example:**

```cpp
// SFML audio (built-in)
sf::Music music;
music.openFromFile("music.ogg");
music.setVolume(50);
music.play();

sf::SoundBuffer buffer;
buffer.loadFromFile("shoot.wav");
sf::Sound sound(buffer);
sound.play();
```

---

### 5. Good Documentation & Community ✅

- ✅ **Comprehensive tutorials** and examples
- ✅ **Active community** for support
- ✅ **Well-documented API** with clear examples
- ✅ **Regular updates** and maintenance

**Learning curve:**

```text
SFML:   Low  ████
SDL3:   Med  ████████
Raylib: Low  ████
```

---

### 6. Cross-Platform Support ✅

**Runs on all target platforms:**

- ✅ Windows (Visual C++, MinGW)
- ✅ Linux (GCC, Clang)
- ✅ macOS (Clang)
- ✅ FreeBSD

**Same code, all platforms:**

```cpp
// This code works unchanged on Windows/Linux/macOS
int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "R-Type");
    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        
        window.clear();
        // ... render game
        window.display();
    }
}
```

---

## Why NOT SDL3?

**SDL is excellent, but:**

- ⚠️ **C API**: Requires C++ wrappers for RAII and safety
- ⚠️ **More Boilerplate**: Manual resource management
- ⚠️ **Procedural**: Less intuitive than OOP for game objects
- ⚠️ **General Purpose**: Not optimized specifically for 2D

**SDL is better for:**

- 3D games (OpenGL/Vulkan backends)
- Low-level control needs
- C projects

---

## Why NOT Raylib?

**Raylib is great for beginners, but:**

- ⚠️ **C API**: Same wrapper issues as SDL
- ⚠️ **Less Mature**: Smaller ecosystem than SFML/SDL
- ⚠️ **Simpler**: Less control for complex games
- ⚠️ **Game-focused**: Good for simple games, less flexible

**Raylib is better for:**

- Rapid prototyping
- Game jams
- Educational projects
- Simple 2D/3D games

---

## R-Type Implementation with SFML

### Window Setup

```cpp
class Game {
    sf::RenderWindow window_;
    sf::View camera_;
    
public:
    Game() 
        : window_(sf::VideoMode(1920, 1080), "R-Type",
                  sf::Style::Fullscreen),
          camera_(sf::FloatRect(0, 0, 1920, 1080)) {
        window_.setFramerateLimit(60);
        window_.setVerticalSyncEnabled(true);
    }
    
    void run() {
        while (window_.isOpen()) {
            handleEvents();
            update(deltaTime);
            render();
        }
    }
};
```

### Sprite Management

```cpp
class Entity {
    sf::Sprite sprite_;
    sf::Vector2f velocity_;
    
public:
    void update(float dt) {
        sprite_.move(velocity_ * dt);
    }
    
    void render(sf::RenderWindow& window) {
        window.draw(sprite_);
    }
    
    sf::FloatRect getBounds() const {
        return sprite_.getGlobalBounds();
    }
};
```

### Scrolling Background

```cpp
class Background {
    sf::Sprite layer1_, layer2_;
    float scrollSpeed_ = 100.f;
    
public:
    void update(float dt) {
        layer1_.move(-scrollSpeed_ * dt, 0);
        layer2_.move(-scrollSpeed_ * 0.5f * dt, 0);
        
        // Wrap around
        if (layer1_.getPosition().x < -1920) {
            layer1_.setPosition(0, 0);
        }
    }
    
    void render(sf::RenderWindow& window) {
        window.draw(layer1_);
        window.draw(layer2_);
    }
};
```

---

## Final Recommendation

✅ **Use SFML** for R-Type graphics.

**Rationale:**

1. **Native C++**: No wrapper overhead, RAII safety
2. **Object-Oriented**: Clean API matching our ECS design
3. **2D Optimized**: Perfect for side-scrolling shooter
4. **Comprehensive**: Graphics + Audio + Input + Networking
5. **Well-Documented**: Excellent learning resources
6. **Cross-Platform**: Same code on Windows/Linux/macOS

**Implementation:**

- SFML for rendering, audio, input
- Integrates cleanly with ECS architecture
- Simple build integration with Vcpkg

---

## References

- PoC implementations: `/PoC/PoC_Graphic/`
- Decision document: `/PoC/PoC_Graphic/GraphicChoice.md`
- SFML documentation: [https://www.sfml-dev.org/](https://www.sfml-dev.org/)
