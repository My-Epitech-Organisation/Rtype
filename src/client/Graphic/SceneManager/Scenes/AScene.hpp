/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AScene.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_ASCENE_HPP_
#define SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_ASCENE_HPP_

#include <SFML/Graphics/RenderWindow.hpp>

#include "Graphic/AssetManager/AssetManager.hpp"
#include "IScene.hpp"
#include "ecs/ECS.hpp"

class AScene : public IScene {
   protected:
    std::shared_ptr<ECS::Registry> _registry;
    std::shared_ptr<AssetManager> _assetsManager;

   public:
    void pollEvents(const sf::Event& e) override = 0;
    void update() override = 0;
    void render(sf::RenderWindow& window) override = 0;

    explicit AScene(const std::shared_ptr<ECS::Registry>& registry,
                    const std::shared_ptr<AssetManager>& assetsManager)
        : _registry(registry), _assetsManager(assetsManager) {}
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_ASCENE_HPP_
