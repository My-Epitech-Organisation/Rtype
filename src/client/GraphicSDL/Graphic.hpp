/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Graphic.hpp
*/

#pragma once

#include "SDL3/SDL.h"
#include <iostream>

namespace RTypeClient {
    class Graphic {
        private:
            SDL_Window *_window;
            SDL_Renderer *_renderer;
            bool _isRunning;
            SDL_Texture *_playerTexture;
            SDL_Texture* loadTextureFromMemory(const unsigned char* data, size_t size);

        public:
            Graphic();
            ~Graphic();
            void loop();
    };
}