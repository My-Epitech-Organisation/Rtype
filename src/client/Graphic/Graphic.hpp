/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** Graphic.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_GRAPHIC_HPP_
#define SRC_CLIENT_GRAPHIC_GRAPHIC_HPP_

#include <memory>

#include <SFML/Graphics.hpp>

#include "AssetManager/AssetManager.hpp"
#include "KeyboardActions.hpp"
#include "SceneManager/SceneManager.hpp"
#include "ecs/ECS.hpp"

class Graphic {
   public:
    static constexpr int WINDOW_WIDTH = 1920;
    static constexpr int WINDOW_HEIGHT = 1080;

   private:
    std::shared_ptr<ECS::Registry> _registry;
    std::shared_ptr<AssetManager> _assetsManager;
    std::unique_ptr<SceneManager> _sceneManager;
    std::shared_ptr<KeyboardActions> _keybinds;

    std::shared_ptr<sf::RenderWindow> _window;
    sf::View _view;
    sf::Clock _mainClock;

    void _pollEvents();
    void _update();
    void _display();

   public:
    void loop();

    explicit Graphic(const std::shared_ptr<ECS::Registry>& registry);
    ~Graphic() = default;

    Graphic(const Graphic&) = delete;
    Graphic& operator=(const Graphic&) = delete;
    Graphic(Graphic&&) = delete;
    Graphic& operator=(Graphic&&) = delete;
};

#endif  // SRC_CLIENT_GRAPHIC_GRAPHIC_HPP_
