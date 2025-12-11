/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AudioLib.cpp
*/

#include "AudioLib.hpp"

#include <utility>

void AudioLib::setLoop(const bool& loop) const {
    if (this->_currentMusic) this->_currentMusic->setLooping(loop);
}

void AudioLib::setMusicVolume(const float& volume) {
    this->_volumeMusic = volume;
    if (this->_currentMusic) this->_currentMusic->setVolume(this->_volumeMusic);
}

float AudioLib::getMusicVolume() const { return this->_volumeMusic; }

void AudioLib::setSFXVolume(const float& volume) {
    this->_volumeSFX = volume;
    for (auto& sound : this->_sounds) {
        sound.setVolume(this->_volumeSFX);
    }
}

float AudioLib::getSFXVolume() const { return this->_volumeSFX; }

void AudioLib::pauseMusic() const {
    if (this->_currentMusic) this->_currentMusic->pause();
}

void AudioLib::play() const {
    if (this->_currentMusic) this->_currentMusic->stop();
    this->_currentMusic->play();
}

void AudioLib::playSFX(const sf::SoundBuffer& sfx) {
    this->_sounds.remove_if([](const sf::Sound& s) {
        return s.getStatus() == sf::SoundSource::Status::Stopped;
    });
    this->_sounds.emplace_back(sfx);
    this->_sounds.back().setVolume(this->_volumeSFX);
    this->_sounds.back().play();
}

void AudioLib::loadMusic(std::shared_ptr<sf::Music> music) {
    if (this->_currentMusic) this->_currentMusic->stop();
    this->_currentMusic = music;
    this->_currentMusic->setVolume(this->_volumeMusic);
}

AudioLib::~AudioLib() {
    if (this->_currentMusic) this->_currentMusic->stop();
}
