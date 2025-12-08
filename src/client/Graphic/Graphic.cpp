/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** Graphic.cpp
*/

#include "Graphic.hpp"

#include <optional>
#include <utility>

#include "AssetManager/AssetManager.hpp"
#include "SceneManager/SceneException.hpp"

void Graphic::_pollEvents() {
    while (const std::optional event = this->_window->pollEvent()) {
        if (!event) return;
        if (event->is<sf::Event::Closed>()) {
            this->_window->close();
        }
        this->_eventSystem->setEvent(*event);
        this->_eventSystem->update(*this->_registry, 0.f);
        this->_sceneManager->pollEvents(*event);
    }
}

void Graphic::_updateDeltaTime() {
    this->_currentDeltaTime = this->_mainClock.getElapsedTime().asSeconds();
    this->_mainClock.restart();
}

void Graphic::_updateViewScrolling() {
    sf::Vector2f center = this->_view->getCenter();
    float newX = center.x + (scrollSpeed * this->_currentDeltaTime);
    this->_view->setCenter({newX, center.y});
}

void Graphic::_update() {
    this->_updateDeltaTime();
    this->_updateViewScrolling();

    this->_systemScheduler->runSystem("button_update");
    this->_systemScheduler->runSystem("parallax");
    this->_systemScheduler->runSystem("movement");
    this->_sceneManager->update(this->_currentDeltaTime);
}

void Graphic::_display() {
    this->_window->clear();

    this->_systemScheduler->runSystem("render");
    this->_systemScheduler->runSystem("boxing");

    this->_sceneManager->draw();
    this->_window->display();
}

void Graphic::loop() {
    while (this->_window->isOpen()) {
        this->_systemScheduler->runSystem("reset_triggers");
        this->_pollEvents();
        this->_update();
        this->_display();
    }
}

void Graphic::_initializeSystems() {
    _movementSystem =
        std::make_unique<::rtype::games::rtype::client::MovementSystem>();
    _buttonUpdateSystem =
        std::make_unique<::rtype::games::rtype::client::ButtonUpdateSystem>(
            this->_window);
    _parallaxScrolling =
        std::make_unique<::rtype::games::rtype::client::ParallaxScrolling>(
            this->_view);
    _renderSystem =
        std::make_unique<::rtype::games::rtype::client::RenderSystem>(
            this->_window);
    _boxingSystem =
        std::make_unique<::rtype::games::rtype::client::BoxingSystem>(
            this->_window);
    _resetTriggersSystem =
        std::make_unique<::rtype::games::rtype::client::ResetTriggersSystem>();
    _eventSystem = std::make_unique<::rtype::games::rtype::client::EventSystem>(
        this->_window);
    _projectileSystem =
        std::make_unique<::rtype::games::rtype::shared::ProjectileSystem>();
    _lifetimeSystem =
        std::make_unique<::rtype::games::rtype::shared::LifetimeSystem>();

    _systemScheduler = std::make_unique<ECS::SystemScheduler>(*this->_registry);

    _systemScheduler->addSystem("reset_triggers", [this](ECS::Registry& reg) {
        _resetTriggersSystem->update(reg, 0.f);
    });

    _systemScheduler->addSystem(
        "button_update",
        [this](ECS::Registry& reg) { _buttonUpdateSystem->update(reg, 0.f); },
        {"reset_triggers"});

    _systemScheduler->addSystem("parallax",
                                [this](ECS::Registry& reg) {
                                    _parallaxScrolling->update(
                                        reg, _currentDeltaTime);
                                },
                                {"button_update"});

    _systemScheduler->addSystem("movement",
                                [this](ECS::Registry& reg) {
                                    _movementSystem->update(reg,
                                                            _currentDeltaTime);
                                },
                                {"parallax"});

    _systemScheduler->addSystem("projectile",
                                [this](ECS::Registry& reg) {
                                    _projectileSystem->update(
                                        reg, _currentDeltaTime);
                                },
                                {"movement"});

    _systemScheduler->addSystem("lifetime",
                                [this](ECS::Registry& reg) {
                                    _lifetimeSystem->update(reg,
                                                            _currentDeltaTime);
                                },
                                {"projectile"});

    _systemScheduler->addSystem(
        "render",
        [this](ECS::Registry& reg) { _renderSystem->update(reg, 0.f); },
        {"lifetime"});

    _systemScheduler->addSystem(
        "boxing",
        [this](ECS::Registry& reg) { _boxingSystem->update(reg, 0.f); },
        {"render"});
}

Graphic::Graphic(std::shared_ptr<ECS::Registry> registry)
    : _registry(registry),
      _view(std::make_shared<sf::View>(
          sf::FloatRect({0, 0}, {WINDOW_WIDTH, WINDOW_HEIGHT}))) {
    this->_keybinds = std::make_shared<KeyboardActions>();
    this->_window = std::make_shared<sf::RenderWindow>(
        sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "R-Type - Epitech 2025");
    this->_window->setView(*this->_view);
    this->_assetsManager = std::make_shared<AssetManager>();
    this->_sceneManager = std::make_unique<SceneManager>(
        registry, this->_assetsManager, this->_window, this->_keybinds);
    _initializeSystems();
    this->_mainClock.start();
}
