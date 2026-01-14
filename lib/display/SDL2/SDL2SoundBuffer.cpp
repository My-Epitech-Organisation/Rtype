/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SDL2SoundBuffer.cpp
*/

#include "SDL2SoundBuffer.hpp"
#include <iostream>

namespace rtype::display {

bool SDL2SoundBuffer::loadFromFile(const std::string& path) {
    ma_decoder decoder;
    ma_result result = ma_decoder_init_file(path.c_str(), nullptr, &decoder);
    if (result != MA_SUCCESS) {
        std::cerr << "SDL2SoundBuffer failed to load: " << path << " - error " << result << std::endl;
        _valid = false;
        _path.clear();
        return false;
    }
    ma_decoder_uninit(&decoder);
    _path = path;
    _valid = true;
    return true;
}

} // namespace rtype::display
