/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AudioManager.cpp
*/

#include "AudioManager.hpp"

#include "Macros.hpp"

void AudioManager::load(const std::string& id, const std::string& filePath) {
    if (this->_assets.contains(id)) return;

    _display->loadMusic(id, filePath);
    auto music = _display->getMusic(id);

    if (!music) {
        LOG_ERROR_CAT(::rtype::LogCategory::Audio,
                      "Error unable to open music: " + filePath);
        throw std::runtime_error("Error while loading music: " + filePath);
    }

    this->_assets[id] = music;
    LOG_INFO_CAT(::rtype::LogCategory::Audio, "Audio saved with ID: " + id);
}

std::shared_ptr<::rtype::display::IMusic> AudioManager::get(
    const std::string& id) {
    auto it = this->_assets.find(id);

    if (it == this->_assets.end()) {
        LOG_ERROR_CAT(::rtype::LogCategory::Audio,
                      "Error music not found: " + id);
        throw std::out_of_range("Error music not found: " + id);
    }

    return it->second;
}
