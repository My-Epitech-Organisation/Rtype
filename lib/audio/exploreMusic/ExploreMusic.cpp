/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** ExploreMusic.cpp
*/

#include "ExploreMusic.hpp"

void ExploreMusic::loadLevelMusic(std::shared_ptr<AudioLib> audioLib)
{
    if (!audioLib || !this->_assetManager || !this->_assetManager->audioManager) {
        return;
    }

    this->_waveMusicId = "explore_level_music";
    const std::string musicPath = "assets/audio/Engi(Explore).mp3";

    this->_assetManager->audioManager->load(this->_waveMusicId, musicPath);
    auto music = this->_assetManager->audioManager->get(this->_waveMusicId);

    if (music) {
        audioLib->loadMusic(music);
        audioLib->setLoop(true);
        audioLib->play();
    }
}
