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
    for (auto& instance : this->_sounds) {
        instance.sound->setVolume(this->_volumeSFX);
    }
}

float AudioLib::getSFXVolume() const { return this->_volumeSFX; }

void AudioLib::pauseMusic() const {
    if (this->_currentMusic) this->_currentMusic->pause();
}

void AudioLib::play() const {
    if (this->_currentMusic) {
        this->_currentMusic->stop();
    }
    this->_currentMusic->play();
}

void AudioLib::cleanupStoppedSounds() {
    this->_sounds.remove_if([](const SoundInstance& instance) {
        return instance.sound->getStatus() ==
               rtype::display::ISound::Status::Stopped;
    });
}

void AudioLib::playSFX(std::shared_ptr<rtype::display::ISoundBuffer> sfx) {
    this->playSFX(sfx, "");
}

void AudioLib::playSFX(std::shared_ptr<rtype::display::ISoundBuffer> sfx,
                       const std::string& soundId) {
    if (!sfx) {
        return;
    }

    cleanupStoppedSounds();

    if (this->_sounds.size() >= MAX_CONCURRENT_SOUNDS) {
        return;
    }

    auto sound = _display->createSound(sfx);
    if (sound) {
        sound->setVolume(this->_volumeSFX);
        sound->play();
        this->_sounds.push_back({sound, soundId});
    }
}

void AudioLib::loadMusic(std::shared_ptr<::rtype::display::IMusic> music) {
    if (this->_currentMusic) this->_currentMusic->stop();
    this->_currentMusic = music;
    this->_currentMusic->setVolume(this->_volumeMusic);
}

void AudioLib::update() { cleanupStoppedSounds(); }

AudioLib::~AudioLib() {
    if (this->_currentMusic) this->_currentMusic->stop();
}
