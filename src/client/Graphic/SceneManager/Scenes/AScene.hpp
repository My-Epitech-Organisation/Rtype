/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AScene.hpp
*/

#ifndef R_TYPE_ASCENE_HPP
#define R_TYPE_ASCENE_HPP

#include "IScene.hpp"
#include "ecs/ECS.hpp"
#include "Graphic/AssetManager/AssetManager.hpp"
#include <SFML/Graphics/RenderWindow.hpp>

class AScene : public IScene {
protected:
    std::shared_ptr<ECS::Registry> _registry;
    std::shared_ptr<AssetManager> _assetsManager;
public:
    void pollEvents(const sf::Event &e) override = 0;
    void update() override = 0;
    void render(sf::RenderWindow &window) override = 0;

    explicit AScene(const std::shared_ptr<ECS::Registry> &registry, const std::shared_ptr<AssetManager> &assetsManager) : _registry(registry), _assetsManager(assetsManager) {}
};


#endif //R_TYPE_ASCENE_HPP