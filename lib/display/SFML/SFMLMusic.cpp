/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SFMLMusic.cpp
*/

#include <string>
#include "SFMLMusic.hpp"

bool rtype::display::SFMLMusic::openFromFile(const std::string &path) {
    return _music.openFromFile(path);
}

void rtype::display::SFMLMusic::setLooping(bool loop) {
    _music.setLooping(loop);
}

void rtype::display::SFMLMusic::setVolume(float volume) {
    _music.setVolume(volume);
}

void rtype::display::SFMLMusic::play() {
    _music.play();
}

void rtype::display::SFMLMusic::pause() {
    _music.pause();
}

void rtype::display::SFMLMusic::stop() {
    _music.stop();
}
