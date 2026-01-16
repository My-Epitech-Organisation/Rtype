/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SDL2Sound.hpp
*/

#ifndef R_TYPE_SDL2SOUND_HPP
#define R_TYPE_SDL2SOUND_HPP

#include "include/rtype/display/IDisplay.hpp"
#include "SDL2SoundBuffer.hpp"
#include "SDL2AudioEngine.hpp"
#include <miniaudio.h>
#include <memory>

namespace rtype::display {
class SDL2Sound : public ISound {
public:
    explicit SDL2Sound(std::shared_ptr<SDL2SoundBuffer> buffer);
    ~SDL2Sound() override;

    void setVolume(float volume) override;
    void play() override;
    Status getStatus() const override;

private:
    std::shared_ptr<SDL2SoundBuffer> _buffer;
    ma_sound _sound{};
    bool _valid = false;
};
}

#endif // R_TYPE_SDL2SOUND_HPP
