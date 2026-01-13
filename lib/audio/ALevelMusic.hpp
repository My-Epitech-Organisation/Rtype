/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ABackground.hpp
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

class ALevelMusic : public ILevelMusic {
protected:
    std::shared_ptr<ECS::Registry> _registry;
    std::shared_ptr<AssetManager> _assetManager;
    std::string _levelMusicName;
    std::vector<ECS::Entity> _listEntities;

    std::string getLevelMusicName() override { return _levelMusicName; };
    void unloadLevelMusic() override {
        for (auto s: this->_listEntities) {
            this->_registry->killEntity(s);
        }
        this->_listEntities.clear();
    };

    ALevelMusic(std::shared_ptr<ECS::Registry> registry, std::shared_ptr<AssetManager> assetManager, const std::string &levelMusicName) : _registry(std::move(registry)), _assetManager(std::move(assetManager)), _levelMusicName(levelMusicName) {};
    ~ALevelMusic() override {
        for (auto s: this->_listEntities) {
            this->_registry->killEntity(s);
        }
    };
};

#endif //R_TYPE_ALEVELMUSIC_HPP_
