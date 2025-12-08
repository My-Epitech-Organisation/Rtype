/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AudioLib.cpp
*/

#include "AudioLib.hpp"

#include <utility>

void AudioLib::setLoop(const bool& loop) const {
    this->_currentMusic->setLooping(loop);
}

void AudioLib::setMusicVolume(const float& volume) {
    this->_volume = volume;
    this->_currentMusic->setVolume(this->_volume);
}

float AudioLib::getMusicVolume() const {
    return this->_volume;
}

void AudioLib::pauseMusic() const {
    this->_currentMusic->pause();
}

void AudioLib::play() const {
    this->_currentMusic->stop();
    this->_currentMusic->play();
}

void AudioLib::loadMusic(std::shared_ptr<sf::Music> music) {
    if (this->_currentMusic)
        this->_currentMusic->stop();
    this->_currentMusic = music;
}

AudioLib::~AudioLib() {
    this->_currentMusic->stop();
}
