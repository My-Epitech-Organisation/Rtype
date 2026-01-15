/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ABackground.hpp
*/

#ifndef R_TYPE_ABACKGROUND_HPP
#define R_TYPE_ABACKGROUND_HPP
#include <utility>
#include <memory>
#include <string>
#include <vector>

#include "IBackground.hpp"
#include "lib/ecs/src/ECS.hpp"
#include "src/client/Graphic/AssetManager/AssetManager.hpp"

class ABackground : public IBackground {
protected:
    std::shared_ptr<ECS::Registry> _registry;
    std::shared_ptr<AssetManager> _assetManager;
    std::string _backgroundName;
    std::vector<ECS::Entity> _listEntities;

    std::string getBackgroundName() override { return _backgroundName; };
    void unloadEntitiesBackground() override {
        for (auto s: this->_listEntities) {
            if (this->_registry && this->_registry->isAlive(s)) {
                this->_registry->killEntity(s);
            }
        }
        this->_listEntities.clear();
    };

    ABackground(std::shared_ptr<ECS::Registry> registry, std::shared_ptr<AssetManager> assetManager, const std::string &backgroundName) : _registry(std::move(registry)), _assetManager(std::move(assetManager)), _backgroundName(backgroundName) {};
    ~ABackground() override {
        for (auto s: this->_listEntities) {
            if (this->_registry && this->_registry->isAlive(s)) {
                this->_registry->killEntity(s);
            }
        }
    };
};

#endif //R_TYPE_ABACKGROUND_HPP
