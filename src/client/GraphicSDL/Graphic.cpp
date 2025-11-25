/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Graphic.cpp
*/

#include "Graphic.hpp"
#include "../assets/r-typesheet30a.h"

RTypeClient::Graphic::Graphic() : _window(nullptr), _renderer(nullptr), _isRunning(true)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return;
    }

    if (SDL_CreateWindowAndRenderer("R-Type Client", 800, 600, 0, &_window, &_renderer) < 0) {
        std::cerr << "Window/Renderer Creation Error: " << SDL_GetError() << std::endl;
        return;
    }
    this->_playerTexture = loadTextureFromMemory(r_typesheet30a_bmp, r_typesheet30a_bmp_len);

    if (!_playerTexture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load player texture: %s", SDL_GetError());
    }
}

SDL_Texture* RTypeClient::Graphic::loadTextureFromMemory(const unsigned char* data, size_t size)
{
    SDL_IOStream* io = SDL_IOFromConstMem(data, size);
    if (!io) {
        std::cerr << "Erreur IOStream: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    SDL_Surface* surface = SDL_LoadBMP_IO(io, true);

    if (!surface) {
        std::cerr << "Erreur chargement Surface: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(_renderer, surface);
    SDL_DestroySurface(surface);

    return texture;
}

RTypeClient::Graphic::~Graphic()
{
    if (_playerTexture)
        SDL_DestroyTexture(_playerTexture);
    if (_renderer)
        SDL_DestroyRenderer(_renderer);
    if (_window)
        SDL_DestroyWindow(_window);
    SDL_Quit();
}

void RTypeClient::Graphic::loop()
{
    SDL_Event event;

    while (_isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                _isRunning = false;
            }
        }
        SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
        SDL_RenderClear(_renderer);
        SDL_FRect destRect = { 100.0f, 100.0f, 32.0f, 32.0f }; // x, y, w, h
        SDL_RenderTexture(_renderer, _playerTexture, nullptr, &destRect);
        SDL_RenderPresent(_renderer);
    }
}