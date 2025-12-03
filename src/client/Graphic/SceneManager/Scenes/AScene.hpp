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
    const std::shared_ptr<sf::RenderWindow>& _window;
    std::vector<ECS::Entity> _listEntity;

   public:
    void pollEvents(const sf::Event& e) override = 0;
    void update() override = 0;
    void render(const std::shared_ptr<sf::RenderWindow>& window) override = 0;

    explicit AScene(const std::shared_ptr<ECS::Registry>& registry,
                    const std::shared_ptr<AssetManager>& assetsManager,
                    const std::shared_ptr<sf::RenderWindow>& window)
        : _registry(registry), _assetsManager(assetsManager), _window(window) {}
    ~AScene() override {
        for (auto& entities : this->_listEntity) {
            this->_registry->killEntity(entities);
        }
    };
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_ASCENE_HPP_
