/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** chillMusic
*/

#include "ChillMusic.hpp"


void ChillMusic::loadLevelMusic(std::shared_ptr<AudioLib> audioLib)
{
    if (!audioLib || !this->_assetManager || !this->_assetManager->audioManager) {
        return;
    }

    this->_waveMusicId = "chill_level_music";
    const std::string musicPath = "assets/audio/gameMusic.mp3";

    this->_assetManager->audioManager->load(this->_waveMusicId, musicPath);
    auto music = this->_assetManager->audioManager->get(this->_waveMusicId);

    if (music) {
        audioLib->loadMusic(music);
        audioLib->setLoop(true);
        audioLib->play();
    }
}
