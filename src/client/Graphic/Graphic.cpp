/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** Graphic.cpp
*/

#include "Graphic.hpp"

#include <algorithm>
#include <array>
#include <optional>
#include <utility>

#include "Accessibility.hpp"
#include "AssetManager/AssetManager.hpp"
#include "Config/Parser/RTypeConfigParser.hpp"
#include "Logger/Macros.hpp"
#include "SceneManager/SceneException.hpp"
#include "games/rtype/client/AllComponents.hpp"
#include "games/rtype/client/GameScene/RtypeEntityFactory.hpp"
#include "games/rtype/client/PauseState.hpp"
#include "games/rtype/shared/Components/PositionComponent.hpp"
#include "games/rtype/shared/Components/VelocityComponent.hpp"

void Graphic::_pollEvents() {
    this->_systemScheduler->runSystem("reset_triggers");
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

void Graphic::_updateNetwork() {
    if (_networkSystem) {
        _networkSystem->update();
    }
}

void Graphic::_update() {
    _updateDeltaTime();
    _updateNetwork();
    this->_systemScheduler->runSystem("button_update");

    bool isPaused = false;
    if (this->_registry &&
        this->_registry
            ->hasSingleton<rtype::games::rtype::client::PauseState>()) {
        try {
            isPaused =
                this->_registry
                    ->getSingleton<rtype::games::rtype::client::PauseState>()
                    .isPaused;
        } catch (const std::exception& e) {
            LOG_ERROR("[Graphic] Exception accessing PauseState: " << e.what());
        } catch (...) {
            LOG_ERROR("[Graphic] Unknown exception accessing PauseState");
        }
    }

    if (!isPaused) {
        _updateViewScrolling();
        this->_systemScheduler->runSystem("movement");
        this->_systemScheduler->runSystem("player_animation");
        this->_systemScheduler->runSystem("parallax");
        this->_systemScheduler->runSystem("projectile");
    }

    this->_systemScheduler->runSystem("lifetime");
    this->_sceneManager->update(this->_currentDeltaTime);
}

void Graphic::_display() {
    this->_sceneTexture->clear();

    this->_systemScheduler->runSystem("render");
    this->_systemScheduler->runSystem("boxing");

    this->_sceneTexture->display();

    this->_systemScheduler->runSystem("shader_render");

    this->_sceneManager->draw();
    this->_window->display();
}

void Graphic::loop() {
    this->_window->setFramerateLimit(60);

    while (this->_window->isOpen()) {
        this->_pollEvents();
        this->_update();
        this->_display();
    }
}

void Graphic::_setupNetworkEntityFactory() {
    namespace rc = rtype::games::rtype::client;

    auto assetsManager = this->_assetsManager;
    auto registry = this->_registry;

    _networkSystem->setEntityFactory(
        rc::RtypeEntityFactory::createNetworkEntityFactory(registry,
                                                           assetsManager));
    _networkSystem->onLocalPlayerAssigned(
        [registry](std::uint32_t /*userId*/, ECS::Entity entity) {
            if (registry->isAlive(entity)) {
                registry->emplaceComponent<rc::ControllableTag>(entity);
                LOG_DEBUG("[Graphic] Local player entity assigned");
            }
        });
}

void Graphic::_initializeSystems() {
    this->_movementSystem =
        std::make_unique<::rtype::games::rtype::client::MovementSystem>();
    this->_playerAnimationSystem =
        std::make_unique<::rtype::games::rtype::client::PlayerAnimationSystem>();
    this->_buttonUpdateSystem =
        std::make_unique<::rtype::games::rtype::client::ButtonUpdateSystem>(
            this->_window);
    this->_parallaxScrolling =
        std::make_unique<::rtype::games::rtype::client::ParallaxScrolling>(
            this->_view);
    std::shared_ptr<sf::RenderTarget> targetPtr = this->_sceneTexture;
    this->_renderSystem =
        std::make_unique<::rtype::games::rtype::client::RenderSystem>(
            targetPtr);
    this->_boxingSystem =
        std::make_unique<::rtype::games::rtype::client::BoxingSystem>(
            targetPtr);
    this->_resetTriggersSystem =
        std::make_unique<::rtype::games::rtype::client::ResetTriggersSystem>();
    this->_eventSystem =
        std::make_unique<::rtype::games::rtype::client::EventSystem>(
            this->_window, this->_audioLib);
    this->_projectileSystem =
        std::make_unique<::rtype::games::rtype::shared::ProjectileSystem>();
    this->_lifetimeSystem =
        std::make_unique<::rtype::games::rtype::shared::LifetimeSystem>();
    this->_shaderRenderSystem =
        std::make_unique<::rtype::games::rtype::client::ShaderRenderSystem>(
            this->_window, this->_sceneTexture, this->_colorShader);

    this->_systemScheduler =
        std::make_unique<ECS::SystemScheduler>(*this->_registry);

    this->_systemScheduler->addSystem(
        "reset_triggers", [this](ECS::Registry& reg) {
            this->_resetTriggersSystem->update(reg, 0.f);
        });

    this->_systemScheduler->addSystem("movement",
                                      [this](ECS::Registry& reg) {
                                          _movementSystem->update(
                                              reg, _currentDeltaTime);
                                      },
                                      {"reset_triggers"});

    this->_systemScheduler->addSystem(
        "player_animation",
        [this](ECS::Registry& reg) {
            _playerAnimationSystem->update(reg, _currentDeltaTime);
        },
        {"movement"});

    this->_systemScheduler->addSystem("parallax",
                                      [this](ECS::Registry& reg) {
                                          _parallaxScrolling->update(
                                              reg, _currentDeltaTime);
                                      },
                                      {"player_animation"});

    this->_systemScheduler->addSystem(
        "button_update",
        [this](ECS::Registry& reg) { _buttonUpdateSystem->update(reg, 0.f); },
        {"parallax"});

    this->_systemScheduler->addSystem("projectile",
                                      [this](ECS::Registry& reg) {
                                          this->_projectileSystem->update(
                                              reg, this->_currentDeltaTime);
                                      },
                                      {"movement"});

    this->_systemScheduler->addSystem("lifetime",
                                      [this](ECS::Registry& reg) {
                                          this->_lifetimeSystem->update(
                                              reg, this->_currentDeltaTime);
                                      },
                                      {"projectile"});

    this->_systemScheduler->addSystem(
        "render",
        [this](ECS::Registry& reg) { this->_renderSystem->update(reg, 0.f); },
        {"lifetime"});

    this->_systemScheduler->addSystem(
        "boxing",
        [this](ECS::Registry& reg) { this->_boxingSystem->update(reg, 0.f); },
        {"render"});

    this->_systemScheduler->addSystem("shader_render",
                                      [this](ECS::Registry& reg) {
                                          this->_shaderRenderSystem->update(
                                              reg, 0.f);
                                      },
                                      {"boxing"});
}

void Graphic::_initializeCommonAssets() {
    auto manager = this->_assetsManager;
    auto config = manager->configGameAssets;

    manager->fontManager->load("title_font", config.assets.fonts.TitleFont);
    manager->fontManager->load("main_font", config.assets.fonts.MainFont);

    manager->textureManager->load("bg_menu", config.assets.textures.background);
    manager->textureManager->load("bg_planet_1",
                                  config.assets.textures.planet1);
    manager->textureManager->load("bg_planet_2",
                                  config.assets.textures.planet2);
    manager->textureManager->load("bg_planet_3",
                                  config.assets.textures.planet3);
    manager->textureManager->load("astro_vessel",
                                  config.assets.textures.astroVessel);
    manager->textureManager->load("player_vessel",
                                  config.assets.textures.Player);
    manager->textureManager->load("projectile_player_laser",
                                  config.assets.textures.missileLaser);

    manager->soundManager->load("hover_button", config.assets.sfx.hoverButton);
    manager->soundManager->load("click_button", config.assets.sfx.clickButton);
    manager->soundManager->load("player_spawn", config.assets.sfx.playerSpawn);
    manager->soundManager->load("player_death", config.assets.sfx.playerDeath);
    manager->soundManager->load("bydos_spawn", config.assets.sfx.enemySpawn);
    manager->soundManager->load("bydos_death", config.assets.sfx.enemyDeath);
    manager->soundManager->load("laser_sfx", config.assets.sfx.laser);

    manager->textureManager->get("bg_menu").setRepeated(true);
    manager->textureManager->get("bg_planet_1").setRepeated(true);
    manager->textureManager->get("bg_planet_2").setRepeated(true);
    manager->textureManager->get("bg_planet_3").setRepeated(true);
}

Graphic::Graphic(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<rtype::client::NetworkClient> networkClient,
    std::shared_ptr<rtype::client::ClientNetworkSystem> networkSystem)
    : _registry(std::move(registry)),
      _networkClient(std::move(networkClient)),
      _networkSystem(std::move(networkSystem)),
      _view(std::make_shared<sf::View>(
          sf::FloatRect({0, 0}, {WINDOW_WIDTH, WINDOW_HEIGHT}))) {
    rtype::game::config::RTypeConfigParser parser;
    auto assetsConfig = parser.loadFromFile("./assets/config.toml");
    if (!assetsConfig.has_value()) throw std::exception();
    this->_keybinds = std::make_shared<KeyboardActions>();
    this->_window = std::make_shared<sf::RenderWindow>(
        sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "R-Type - Epitech 2025");
    this->_window->setView(*this->_view);

    this->_sceneTexture = std::make_shared<sf::RenderTexture>(
        sf::Vector2u{WINDOW_WIDTH, WINDOW_HEIGHT});

    if (sf::Shader::isAvailable()) {
        auto shader = std::make_shared<sf::Shader>();
        if (shader->loadFromFile("assets/shaders/colorblind.frag",
                                 sf::Shader::Type::Fragment)) {
            this->_colorShader = shader;
        } else {
            LOG_ERROR("Failed to load shader: assets/shaders/colorblind.frag");
        }
    }
    this->_assetsManager = std::make_shared<AssetManager>(assetsConfig.value());

    this->_assetsManager->textureManager->load(
        "projectile_player_laser",
        this->_assetsManager->configGameAssets.assets.textures.missileLaser);

    if (_networkSystem) {
        _setupNetworkEntityFactory();
    }
    this->_audioLib = std::make_shared<AudioLib>();
    this->_registry->setSingleton<std::shared_ptr<AudioLib>>(this->_audioLib);
    this->_registry->setSingleton<AccessibilitySettings>(
        AccessibilitySettings{ColorBlindMode::None});
    this->_registry->setSingleton<rtype::games::rtype::client::PauseState>(
        rtype::games::rtype::client::PauseState{false});
    this->_initializeCommonAssets();
    this->_sceneManager = std::make_unique<SceneManager>(
        _registry, this->_assetsManager, this->_window, this->_keybinds,
        _networkClient, _networkSystem, this->_audioLib);
    _initializeSystems();
    this->_mainClock.start();
}
