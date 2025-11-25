# SFML Proof-of-Concept Documentation

## Overview

This document describes a small proof-of-concept (POC) using SFML (Simple and Fast Multimedia Library). SFML is a popular C++ library for multimedia applications, including games and interactive demos.

## What is Easy to Do with SFML?

SFML provides a simple API for:

- **Window Creation:** Quickly open a window with customizable size and title.
- **Drawing Graphics:** Render shapes (circles, rectangles), sprites, and text with minimal code.
- **Handling Input:** Capture keyboard, mouse, and joystick events easily.
- **Playing Sounds:** Load and play audio files with a few lines of code.
- **Timing:** Manage frame rates and time intervals for smooth animations.

It's simple to use the SFML in a project

Example (C++):
```cpp
#include <SFML/Graphics.hpp>

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML POC");
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        window.clear();
        // Draw your objects here
        window.display();
    }
    return 0;
}
```

## Installation
Using sources:

https://github.com/SFML/SFML

Using application package manager
    -> Apt
    -> Dnf
    -> Vcpkg
    -> Conan
    -> and more