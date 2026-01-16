/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SDL2SoundBuffer.hpp
*/

#ifndef R_TYPE_SDL2SOUNDBUFFER_HPP
#define R_TYPE_SDL2SOUNDBUFFER_HPP

#include "include/rtype/display/IDisplay.hpp"
#include <miniaudio.h>
#include <string>

namespace rtype::display {
class SDL2SoundBuffer : public ISoundBuffer {
public:
    SDL2SoundBuffer() = default;
    ~SDL2SoundBuffer() override = default;

    bool loadFromFile(const std::string& path) override;

    const std::string& getPath() const { return _path; }

private:
    std::string _path;
    bool _valid = false;
};
}

#endif // R_TYPE_SDL2SOUNDBUFFER_HPP
