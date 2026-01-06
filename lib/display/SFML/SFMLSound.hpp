/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SFMLSound.hpp
*/

#ifndef SFMLSOUND_HPP_
#define SFMLSOUND_HPP_

#include <SFML/Audio.hpp>
#include "include/rtype/display/IDisplay.hpp"
#include "SFMLSoundBuffer.hpp"

class SFMLSound : public rtype::display::ISound {
public:
    explicit SFMLSound(std::shared_ptr<SFMLSoundBuffer> buffer);
    ~SFMLSound() override = default;

    void setVolume(float volume) override;
    void play() override;
    Status getStatus() const override;

private:
    std::shared_ptr<SFMLSoundBuffer> _buffer;
    sf::Sound _sound;
};

#endif // SFMLSOUND_HPP_
