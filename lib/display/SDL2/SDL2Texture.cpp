/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SDL2Texture.cpp
*/

#include "SDL2Texture.hpp"
#include <SDL2/SDL_image.h>
#include <iostream>

namespace rtype::display {
    SDL2Texture::SDL2Texture() {}

    SDL2Texture::~SDL2Texture() {
        if (_texture) {
            SDL_DestroyTexture(_texture);
            _texture = nullptr;
        }
    }

    bool SDL2Texture::loadFromFile(const std::string& path) {
        // This will be called from SDL2Display which has the renderer
        // The actual loading happens in SDL2Display::loadTexture
        return true;
    }

    void SDL2Texture::setRepeated(bool repeated) {
        // SDL2 doesn't have per-texture repeat settings like SFML
        // This is handled at render time
    }

    void SDL2Texture::setSmooth(bool smooth) {
        // SDL2 filtering is set at renderer level
    }

    Vector2u SDL2Texture::getSize() const {
        return _cachedSize;
    }

    void SDL2Texture::setSDL2Texture(SDL_Texture* texture) {
        _texture = texture;
        if (_texture) {
            int w = 0, h = 0;
            SDL_QueryTexture(_texture, nullptr, nullptr, &w, &h);
            _cachedSize = {static_cast<unsigned int>(w), static_cast<unsigned int>(h)};
        } else {
            _cachedSize = {0, 0};
        }
    }
}
