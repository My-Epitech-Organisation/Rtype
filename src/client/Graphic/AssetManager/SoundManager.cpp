/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SoundManager.cpp
*/

#include "SoundManager.hpp"

#include "Macros.hpp"
#include "include/rtype/display/IDisplay.hpp"

void SoundManager::load(const std::string& id, const std::string& filePath) {
    if (this->_assets.contains(id)) return;

    _display->loadSoundBuffer(id, filePath);
    auto sound = _display->getSoundBuffer(id);

    if (!sound) {
        LOG_ERROR_CAT(::rtype::LogCategory::Audio,
                      "Error unable to open sound: " + filePath);
        throw std::runtime_error("Error while loading sound: " + filePath);
    }

    this->_assets[id] = sound;
    LOG_INFO_CAT(::rtype::LogCategory::Audio, "Sound saved with ID: " + id);
}

std::shared_ptr<::rtype::display::ISoundBuffer> SoundManager::get(
    const std::string& id) {
    auto it = this->_assets.find(id);

    if (it == this->_assets.end()) {
        LOG_ERROR_CAT(::rtype::LogCategory::Audio,
                      "Error sound not found: " + id);
        throw std::out_of_range("Error sound not found: " + id);
    }

    return it->second;
}
