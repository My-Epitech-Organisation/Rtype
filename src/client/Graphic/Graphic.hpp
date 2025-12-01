/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** Graphic.hpp
*/

#ifndef R_TYPE_GRAPHIC_HPP
#define R_TYPE_GRAPHIC_HPP

#include <SFML/Graphics.hpp>
#include "KeyboardActions.hpp"
#include "SceneManager.hpp"
#include "ecs/ECS.hpp"

class Graphic {
private:
    std::shared_ptr<ECS::Registry> _registry;
    SceneManager _sceneManager;
    KeyboardActions _keybinds;

    sf::RenderWindow _window;
    sf::Clock _mainClock;


    void _handleKeyReleasedEvent(const std::optional<sf::Event> &event);

    void _poolEvents();
    void _update();
    void _display();
public:
    void loop();

    explicit Graphic(std::shared_ptr<ECS::Registry> registry);
    ~Graphic() = default;
};



#endif //R_TYPE_GRAPHIC_HPP