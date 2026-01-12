/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ABackground.hpp
*/

#ifndef R_TYPE_ABACKGROUND_HPP
#define R_TYPE_ABACKGROUND_HPP
#include <utility>

#include "IBackground.hpp"

class ABackground : public IBackground {
protected:
    std::shared_ptr<ECS::Registry> _registry;
    std::string _backgroundName;
    std::vector<ECS::Entity> _listEntities;

    std::string getBackgroundName() override { return _backgroundName; };
    void unloadEntitiesBackground() override {
        for (auto s: this->_listEntities) {
            this->_registry->killEntity(s);
        }
        this->_listEntities.clear();
    };

    ABackground(std::shared_ptr<ECS::Registry> registry, const std::string &backgroundName) : _registry(std::move(registry)), _backgroundName(backgroundName) {};
    ~ABackground() override {
        for (auto s: this->_listEntities) {
            this->_registry->killEntity(s);
        }
    };
};

#endif //R_TYPE_ABACKGROUND_HPP
