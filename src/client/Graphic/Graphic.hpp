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
#include <rtype/ecs.hpp>

#include "../../games/rtype/client/Systems/BoxingSystem.hpp"
#include "../../games/rtype/client/Systems/ButtonUpdateSystem.hpp"
#include "../../games/rtype/client/Systems/MovementSystem.hpp"
#include "../../games/rtype/client/Systems/RenderSystem.hpp"
#include "../../games/rtype/client/Systems/ResetTriggersSystem.hpp"
#include "AssetManager/AssetManager.hpp"
#include "KeyboardActions.hpp"
#include "SceneManager/SceneManager.hpp"
#include "ecs/ECS.hpp"
#include "src/games/rtype/client/Systems/ParallaxScrolling.hpp"

class Graphic {
   private:
    std::shared_ptr<ECS::Registry> _registry;
    std::shared_ptr<AssetManager> _assetsManager;
    std::unique_ptr<SceneManager> _sceneManager;
    std::shared_ptr<KeyboardActions> _keybinds;

    std::shared_ptr<sf::RenderWindow> _window;
    std::shared_ptr<sf::View> _view;
    sf::Clock _mainClock;

    // Systems
    std::unique_ptr<::rtype::games::rtype::client::MovementSystem> _movementSystem;
    std::unique_ptr<::rtype::games::rtype::client::ButtonUpdateSystem> _buttonUpdateSystem;
    std::unique_ptr<::rtype::games::rtype::client::ParallaxScrolling> _parallaxScrolling;
    std::unique_ptr<::rtype::games::rtype::client::RenderSystem> _renderSystem;
    std::unique_ptr<::rtype::games::rtype::client::BoxingSystem> _boxingSystem;
    std::unique_ptr<::rtype::games::rtype::client::ResetTriggersSystem> _resetTriggersSystem;

    static constexpr int WINDOW_WIDTH = 1920;
    static constexpr int WINDOW_HEIGHT = 1080;

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
