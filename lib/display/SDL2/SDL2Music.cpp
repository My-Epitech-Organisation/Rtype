/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SDL2Music.cpp
*/

#include "SDL2Music.hpp"
#include <iostream>
#include <algorithm>

namespace rtype::display {

SDL2Music::~SDL2Music() {
    if (_valid && g_sdl2AudioInitialized) {
        ma_sound_uninit(&_sound);
    }
}

bool SDL2Music::openFromFile(const std::string& path) {
    if (!g_sdl2AudioInitialized) return false;
    if (_valid) {
        ma_sound_uninit(&_sound);
        _valid = false;
    }
    ma_result res = ma_sound_init_from_file(&g_sdl2AudioEngine, path.c_str(), MA_SOUND_FLAG_STREAM, nullptr, nullptr, &_sound);
    if (res != MA_SUCCESS) {
        std::cerr << "SDL2Music failed to load: " << path << " - error " << res << std::endl;
        return false;
    }
    ma_sound_set_looping(&_sound, _loop);
    _valid = true;
    return true;
}

void SDL2Music::setLooping(bool loop) {
    _loop = loop;
    if (_valid) {
        ma_sound_set_looping(&_sound, loop);
    }
}

void SDL2Music::setVolume(float volume) {
    if (!_valid) return;
    float v = std::clamp(volume, 0.0f, 100.0f) / 100.0f;
    ma_sound_set_volume(&_sound, v);
}

void SDL2Music::play() {
    if (!_valid) return;
    ma_sound_start(&_sound);
}

void SDL2Music::pause() {
    if (!_valid) return;
    ma_sound_stop(&_sound);
}

void SDL2Music::stop() {
    if (!_valid) return;
    ma_sound_stop(&_sound);
    ma_sound_seek_to_pcm_frame(&_sound, 0);
}

} // namespace rtype::display
