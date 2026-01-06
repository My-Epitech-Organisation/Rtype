/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SFMLSoundBuffer.cpp
*/

#include "SFMLSoundBuffer.hpp"

bool SFMLSoundBuffer::loadFromFile(const std::string& path) {
    return _buffer.loadFromFile(path);
}

const sf::SoundBuffer& SFMLSoundBuffer::getSFMLSoundBuffer() const {
    return _buffer;
}
