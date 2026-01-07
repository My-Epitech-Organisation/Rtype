/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SFMLSound.cpp
*/

#include "SFMLSound.hpp"

SFMLSound::SFMLSound(std::shared_ptr<SFMLSoundBuffer> buffer) : _buffer(buffer), _sound(buffer->getSFMLSoundBuffer()) {
}

void SFMLSound::setVolume(float volume) {
    _sound.setVolume(volume);
}

void SFMLSound::play() {
    _sound.play();
}

rtype::display::ISound::Status SFMLSound::getStatus() const {
    switch (_sound.getStatus()) {
        case sf::SoundSource::Status::Stopped: return Status::Stopped;
        case sf::SoundSource::Status::Paused: return Status::Paused;
        case sf::SoundSource::Status::Playing: return Status::Playing;
        default: return Status::Stopped;
    }
}
