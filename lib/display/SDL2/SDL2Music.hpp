/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SDL2Music.hpp
*/

#ifndef R_TYPE_SDL2MUSIC_HPP
#define R_TYPE_SDL2MUSIC_HPP

#include "include/rtype/display/IDisplay.hpp"
#include "SDL2AudioEngine.hpp"
#include <miniaudio.h>
#include <string>

namespace rtype::display {
class SDL2Music : public IMusic {
public:
    SDL2Music() = default;
    ~SDL2Music() override;

    bool openFromFile(const std::string& path) override;
    void setLooping(bool loop) override;
    void setVolume(float volume) override;
    void play() override;
    void pause() override;
    void stop() override;

private:
    ma_sound _sound{};
    bool _loop = false;
    bool _valid = false;
};
}

#endif // R_TYPE_SDL2MUSIC_HPP
