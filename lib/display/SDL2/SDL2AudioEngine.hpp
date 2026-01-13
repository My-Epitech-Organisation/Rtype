/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SDL2AudioEngine.hpp - shared miniaudio engine helpers
*/

#ifndef R_TYPE_SDL2AUDIOENGINE_HPP
#define R_TYPE_SDL2AUDIOENGINE_HPP

#include <miniaudio.h>

extern ma_engine g_sdl2AudioEngine;
extern bool g_sdl2AudioInitialized;

bool initSDL2AudioEngine();
void shutdownSDL2AudioEngine();

#endif // R_TYPE_SDL2AUDIOENGINE_HPP
