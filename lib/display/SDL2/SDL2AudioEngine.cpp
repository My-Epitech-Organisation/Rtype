/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SDL2AudioEngine.cpp - shared miniaudio engine helpers
*/


#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#include "SDL2AudioEngine.hpp"
#include <iostream>

ma_engine g_sdl2AudioEngine;
bool g_sdl2AudioInitialized = false;

bool initSDL2AudioEngine() {
    if (g_sdl2AudioInitialized) return true;
    ma_result result = ma_engine_init(nullptr, &g_sdl2AudioEngine);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to initialize miniaudio engine: " << result << std::endl;
        return false;
    }
    g_sdl2AudioInitialized = true;
    return true;
}

void shutdownSDL2AudioEngine() {
    if (!g_sdl2AudioInitialized) return;
    ma_engine_uninit(&g_sdl2AudioEngine);
    g_sdl2AudioInitialized = false;
}
