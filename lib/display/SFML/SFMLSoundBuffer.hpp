/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SFMLSoundBuffer.hpp
*/

#ifndef SFMLSOUNDBUFFER_HPP_
#define SFMLSOUNDBUFFER_HPP_

#include <SFML/Audio.hpp>
#include "include/rtype/display/IDisplay.hpp"

class SFMLSoundBuffer : public rtype::display::ISoundBuffer {
public:
    SFMLSoundBuffer() = default;
    ~SFMLSoundBuffer() override = default;

    bool loadFromFile(const std::string& path) override;

    const sf::SoundBuffer& getSFMLSoundBuffer() const;

private:
    sf::SoundBuffer _buffer;
};

#endif // SFMLSOUNDBUFFER_HPP_
