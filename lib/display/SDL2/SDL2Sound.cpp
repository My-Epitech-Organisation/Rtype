/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SDL2Sound.cpp
*/

#include "SDL2Sound.hpp"
#include <algorithm>

namespace rtype::display {

SDL2Sound::SDL2Sound(std::shared_ptr<SDL2SoundBuffer> buffer) : _buffer(std::move(buffer)) {
    if (!_buffer || _buffer->getPath().empty()) return;
    if (!g_sdl2AudioInitialized) return;

    ma_result res = ma_sound_init_from_file(&g_sdl2AudioEngine, _buffer->getPath().c_str(), MA_SOUND_FLAG_DECODE, nullptr, nullptr, &_sound);
    if (res != MA_SUCCESS) {
        _valid = false;
        return;
    }
    _valid = true;
}

void SDL2Sound::setVolume(float volume) {
    if (!_valid) return;
    float v = std::clamp(volume, 0.0f, 100.0f) / 100.0f;
    ma_sound_set_volume(&_sound, v);
}

void SDL2Sound::play() {
    if (!_valid || !g_sdl2AudioInitialized) return;
    ma_sound_seek_to_pcm_frame(&_sound, 0);
    ma_sound_start(&_sound);
}

ISound::Status SDL2Sound::getStatus() const {
    if (!_valid || !g_sdl2AudioInitialized) return Status::Stopped;
    if (ma_sound_is_playing(&_sound) == MA_TRUE) return Status::Playing;
    return Status::Stopped;
}

SDL2Sound::~SDL2Sound() {
    if (_valid && g_sdl2AudioInitialized) {
        ma_sound_uninit(&_sound);
    }
}

} // namespace rtype::display
