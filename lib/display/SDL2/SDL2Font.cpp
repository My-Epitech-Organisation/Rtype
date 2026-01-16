/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SDL2Font.cpp
*/

#include "SDL2Font.hpp"
#include <iostream>
#include <filesystem>

namespace rtype::display {
    SDL2Font::SDL2Font() {
    }

    SDL2Font::~SDL2Font() {
    }

    bool SDL2Font::openFromFile(const std::string& path) {
        if (!std::filesystem::exists(path)) {
            std::cerr << "Font file not found: " << path << std::endl;
            return false;
        }

        TTF_Font* testFont = TTF_OpenFont(path.c_str(), 12);
        if (!testFont) {
            std::cerr << "Failed to load font: " << path << " - " << TTF_GetError() << std::endl;
            return false;
        }
        TTF_CloseFont(testFont);

        _path = path;
        return true;
    }

    TTF_Font* SDL2Font::getFont(unsigned int size) {
        if (_path.empty()) {
            return nullptr;
        }
        return TTF_OpenFont(_path.c_str(), static_cast<int>(size));
    }
}
