/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ALevelMusic.hpp
*/

#ifndef R_TYPE_ALEVELMUSIC_HPP_
#define R_TYPE_ALEVELMUSIC_HPP_
#include <utility>
#include <memory>
#include <string>
#include <vector>

#include "ILevelMusic.hpp"
#include "lib/ecs/src/ECS.hpp"
#include "src/client/Graphic/AssetManager/AssetManager.hpp"
#include "src/client/Graphic/AudioLib/AudioLib.hpp"

class ALevelMusic : public ILevelMusic {
protected:
    std::shared_ptr<ECS::Registry> _registry;
    std::shared_ptr<AssetManager> _assetManager;
    std::string _levelMusicName;
    std::string _waveMusicId;
    std::string _bossMusicId;

    std::string getLevelMusicName() override { return _levelMusicName; };
    void unloadLevelMusic() override {
        if (!_waveMusicId.empty()) {
            this->_assetManager->audioManager->unload(this->_waveMusicId);
            this->_waveMusicId.clear();
        }
        if (!_bossMusicId.empty()) {
            this->_assetManager->audioManager->unload(this->_bossMusicId);
            this->_bossMusicId.clear();
        }
    };

    ALevelMusic(std::shared_ptr<ECS::Registry> registry, 
                std::shared_ptr<AssetManager> assetManager, 
                const std::string& levelMusicName) 
        : _registry(std::move(registry)), 
          _assetManager(std::move(assetManager)), 
          _levelMusicName(levelMusicName) {};
    ~ALevelMusic() override {
        if (_assetManager && _assetManager->audioManager) {
            if (!_waveMusicId.empty()) {
                this->_assetManager->audioManager->unload(this->_waveMusicId);
            }
            if (!_bossMusicId.empty()) {
                this->_assetManager->audioManager->unload(this->_bossMusicId);
            }
        }
    };
};

#endif //R_TYPE_ALEVELMUSIC_HPP_
