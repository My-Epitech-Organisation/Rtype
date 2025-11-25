# Rtype Graphic lib result Proof-of-Concept

According to tested graphic lib:

Raylib
SDL3
SFML

We decided to choose SFML because it is really simple to understand, it's also a native C++ library, so we don't need to create extensive C++ wrappers for a C API like SDL or raylib.

Additional arguments for SFML:

- **Object-Oriented Design**: SFML provides a clean, object-oriented API that naturally fits with C++ development, reducing the need for manual resource management and boilerplate code.
- **Ease of Use**: Compared to SDL, SFML's API is generally considered more intuitive and higher-level, allowing for faster development of common graphic tasks.
- **Modern C++ Features**: SFML leverages modern C++ features, making the codebase cleaner and more maintainable
- **Good Documentation and Community**: SFML has comprehensive documentation and an active community, which is beneficial for troubleshooting and learning.
- **Cross-Platform**: Like SDL, SFML is cross-platform, ensuring compatibility across various operating systems.
- **Focus on 2D Graphics**: SFML is primarily focused on 2D graphics, which aligns well with the requirements of an R-Type game, without the overhead of a full 3D engine like some other libraries might imply.
