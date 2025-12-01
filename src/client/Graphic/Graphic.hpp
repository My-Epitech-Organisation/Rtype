/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** Graphic.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_GRAPHIC_HPP_
#define SRC_CLIENT_GRAPHIC_GRAPHIC_HPP_

#include <SFML/Graphics.hpp>

#include "../../engine/ecs/ECS.hpp"
#include "KeyboardActions.hpp"
#include "SceneManager.hpp"

class Graphic {
   private:
    std::shared_ptr<ECS::Registry> _registry;
    SceneManager _sceneManager;
    KeyboardActions _keybinds;

    sf::RenderWindow _window;
    sf::Clock _mainClock;

    static const int WINDOW_WIDTH = 800;
    static const int WINDOW_HEIGHT = 600;

    void _handleKeyReleasedEvent(const std::optional<sf::Event>& event);

    void _pollEvents();
    void _update();
    void _display();

   public:
    void loop();

    explicit Graphic(std::shared_ptr<ECS::Registry> registry);
    ~Graphic() = default;

    Graphic(const Graphic&) = delete;
    Graphic& operator=(const Graphic&) = delete;
    Graphic(Graphic&&) = delete;
    Graphic& operator=(Graphic&&) = delete;
};

#endif  // SRC_CLIENT_GRAPHIC_GRAPHIC_HPP_
