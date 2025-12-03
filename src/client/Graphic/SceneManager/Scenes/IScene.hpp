/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** IScene.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_ISCENE_HPP_
#define SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_ISCENE_HPP_
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

class IScene {
   public:
    virtual void pollEvents(const sf::Event& e) = 0;
    virtual void update() = 0;
    virtual void render(const std::shared_ptr<sf::RenderWindow>& window) = 0;

    virtual ~IScene() = default;
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_ISCENE_HPP_
