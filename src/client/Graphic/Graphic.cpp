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
#include "Systems/AnimationSystem.hpp"
#include "Systems/ChaserExplosionSystem.hpp"
#include "Systems/ChaserRotationSystem.hpp"
#include "games/rtype/client/AllComponents.hpp"
#include "games/rtype/client/GameScene/RtypeEntityFactory.hpp"
#include "games/rtype/client/GameScene/RtypeInputHandler.hpp"
#include "games/rtype/client/PauseState.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"
#include "games/rtype/shared/Components/VelocityComponent.hpp"
#include "games/rtype/shared/Config/EntityConfig/EntityConfig.hpp"

void Graphic::_pollEvents() {
    this->_systemScheduler->runSystem("reset_triggers");
    rtype::display::Event event;
    while (this->_display->pollEvent(event)) {
        if (event.type == rtype::display::EventType::Closed) {
            this->_display->close();
        }

        this->_eventSystem->setEvent(event);
        this->_eventSystem->update(*this->_registry, 0.f);
        this->_sceneManager->pollEvents(event);
    }
}

void Graphic::_updateDeltaTime() {
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<float> elapsed = now - _lastFrameTime;
    this->_currentDeltaTime = elapsed.count();
    _lastFrameTime = now;
}

void Graphic::_updateViewScrolling() {
    rtype::display::Vector2f center = this->_display->getViewCenter();
    float newX = center.x + (scrollSpeed * this->_currentDeltaTime);
    this->_display->setView(
        {newX, center.y},
        {static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)});
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
            LOG_ERROR_CAT(
                ::rtype::LogCategory::GameEngine,
                "[Graphic] Exception accessing PauseState: " << e.what());
        } catch (...) {
            LOG_ERROR_CAT(::rtype::LogCategory::GameEngine,
                          "[Graphic] Unknown exception accessing PauseState");
        }
    }

    this->_updateViewScrolling();
    this->_systemScheduler->runSystem("parallax");

    if (!isPaused) {
        this->_systemScheduler->runSystem("color_tint");
        this->_systemScheduler->runSystem("player_animation");
        this->_systemScheduler->runSystem("chaser_rotation");
        this->_systemScheduler->runSystem("chaser_explosion");
        this->_systemScheduler->runSystem("animation");
        this->_systemScheduler->runSystem("charged_projectile_animation");
        this->_systemScheduler->runSystem("powerup_visuals");
        this->_systemScheduler->runSystem("projectile");
        this->_systemScheduler->runSystem("charge_input");
        this->_systemScheduler->runSystem("charge_visual");
        this->_systemScheduler->runSystem("forcepod_visual");
        this->_systemScheduler->runSystem("powerup_collection");
        this->_systemScheduler->runSystem("enemy_health_bars");
    }

    this->_systemScheduler->runSystem("lifetime");
    this->_sceneManager->update(this->_currentDeltaTime);
}

void Graphic::_render() {
    if (!this->_display->isOpen()) return;
    this->_display->resetView();

    this->_display->beginRenderToTexture("scene");
    this->_display->clear();

    this->_systemScheduler->runSystem("render");
    this->_systemScheduler->runSystem("boxing");

    this->_display->endRenderToTexture();

    this->_systemScheduler->runSystem("shader_render");

    this->_sceneManager->draw();
    this->_display->display();
}

void Graphic::loop() {
    this->_display->setFramerateLimit(60);

    while (this->_display->isOpen()) {
        this->_pollEvents();
        if (!this->_display->isOpen()) break;
        this->_update();
        if (!this->_display->isOpen()) break;
        this->_render();
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
                LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                              "[Graphic] Local player entity assigned");
            }
        });
}

void Graphic::_initializeSystems() {
    this->_colorTintSystem =
        std::make_unique<::rtype::games::rtype::client::ColorTintSystem>();
    this->_playerAnimationSystem = std::make_unique<
        ::rtype::games::rtype::client::PlayerAnimationSystem>();
    this->_animationSystem =
        std::make_unique<::rtype::games::rtype::client::AnimationSystem>();
    this->_chaserRotationSystem =
        std::make_unique<::rtype::games::rtype::client::ChaserRotationSystem>();
    this->_chaserExplosionSystem = std::make_unique<
        ::rtype::games::rtype::client::ChaserExplosionSystem>();
    this->_playerPowerUpVisualSystem = std::make_unique<
        ::rtype::games::rtype::client::PlayerPowerUpVisualSystem>();
    this->_powerUpCollectionSystem = std::make_unique<
        ::rtype::games::rtype::client::PowerUpCollectionSystem>("main_font");
    this->_buttonUpdateSystem =
        std::make_unique<::rtype::games::rtype::client::ButtonUpdateSystem>(
            this->_display);
    this->_parallaxScrolling =
        std::make_unique<::rtype::games::rtype::client::ParallaxScrolling>(
            this->_display);
    this->_renderSystem =
        std::make_unique<::rtype::games::rtype::client::RenderSystem>(
            this->_display);
    this->_boxingSystem =
        std::make_unique<::rtype::games::rtype::client::BoxingSystem>(
            this->_display);
    this->_resetTriggersSystem =
        std::make_unique<::rtype::games::rtype::client::ResetTriggersSystem>();
    this->_eventSystem =
        std::make_unique<::rtype::games::rtype::client::EventSystem>(
            this->_display, this->_audioLib);
    this->_projectileSystem =
        std::make_unique<::rtype::games::rtype::shared::ProjectileSystem>();
    this->_lifetimeSystem =
        std::make_unique<::rtype::games::rtype::shared::LifetimeSystem>();
    this->_clientDestroySystem =
        std::make_unique<::rtype::games::rtype::client::ClientDestroySystem>();
    this->_shaderRenderSystem =
        std::make_unique<::rtype::games::rtype::client::ShaderRenderSystem>(
            this->_display);
    this->_forcePodVisualSystem =
        std::make_unique<::rtype::games::rtype::client::ForcePodVisualSystem>();
    this->_enemyHealthBarSystem =
        std::make_unique<::rtype::games::rtype::client::EnemyHealthBarSystem>(
            this->_registry);
    this->_chargeInputSystem =
        std::make_unique<::rtype::games::rtype::client::ChargeInputSystem>();
    this->_chargeVisualSystem =
        std::make_unique<::rtype::games::rtype::client::ChargeVisualSystem>(
            this->_display, this->_audioLib);
    this->_chargedProjectileAnimationSystem = std::make_unique<
        ::rtype::games::rtype::client::ChargedProjectileAnimationSystem>();

    this->_systemScheduler =
        std::make_unique<ECS::SystemScheduler>(*this->_registry);

    this->_systemScheduler->addSystem(
        "reset_triggers", [this](ECS::Registry& reg) {
            this->_resetTriggersSystem->update(reg, 0.f);
        });

    this->_systemScheduler->addSystem("color_tint",
                                      [this](ECS::Registry& reg) {
                                          _colorTintSystem->update(
                                              reg, _currentDeltaTime);
                                      },
                                      {"sprite_position"});

    this->_systemScheduler->addSystem("player_animation",
                                      [this](ECS::Registry& reg) {
                                          _playerAnimationSystem->update(
                                              reg, _currentDeltaTime);
                                      },
                                      {"color_tint"});

    this->_systemScheduler->addSystem("animation",
                                      [this](ECS::Registry& reg) {
                                          _animationSystem->update(
                                              reg, _currentDeltaTime);
                                      },
                                      {"reset_triggers"});

    this->_systemScheduler->addSystem("chaser_rotation",
                                      [this](ECS::Registry& reg) {
                                          _chaserRotationSystem->update(
                                              reg, _currentDeltaTime);
                                      },
                                      {});

    this->_systemScheduler->addSystem("chaser_explosion",
                                      [this](ECS::Registry& reg) {
                                          _chaserExplosionSystem->update(
                                              reg, _currentDeltaTime);
                                      },
                                      {});

    this->_systemScheduler->addSystem("powerup_visuals",
                                      [this](ECS::Registry& reg) {
                                          _playerPowerUpVisualSystem->update(
                                              reg, _currentDeltaTime);
                                      },
                                      {"reset_triggers"});

    this->_systemScheduler->addSystem("powerup_collection",
                                      [this](ECS::Registry& reg) {
                                          _powerUpCollectionSystem->update(
                                              reg, _currentDeltaTime);
                                      },
                                      {"powerup_visuals"});

    this->_systemScheduler->addSystem("forcepod_visual",
                                      [this](ECS::Registry& reg) {
                                          _forcePodVisualSystem->update(
                                              reg, _currentDeltaTime);
                                      },
                                      {"animation"});

    this->_systemScheduler->addSystem("charge_input",
                                      [this](ECS::Registry& reg) {
                                          _chargeInputSystem->update(
                                              reg, _currentDeltaTime);
                                      },
                                      {});

    this->_systemScheduler->addSystem("charge_visual",
                                      [this](ECS::Registry& reg) {
                                          _chargeVisualSystem->update(
                                              reg, _currentDeltaTime);
                                      },
                                      {"charge_input"});

    this->_systemScheduler->addSystem(
        "charged_projectile_animation",
        [this](ECS::Registry& reg) {
            _chargedProjectileAnimationSystem->update(reg, _currentDeltaTime);
        },
        {"animation"});

    this->_systemScheduler->addSystem("parallax",
                                      [this](ECS::Registry& reg) {
                                          _parallaxScrolling->update(
                                              reg, _currentDeltaTime);
                                      },
                                      {"player_animation", "powerup_visuals"});

    this->_systemScheduler->addSystem(
        "button_update",
        [this](ECS::Registry& reg) { _buttonUpdateSystem->update(reg, 0.f); },
        {"parallax"});

    this->_systemScheduler->addSystem("projectile",
                                      [this](ECS::Registry& reg) {
                                          this->_projectileSystem->update(
                                              reg, this->_currentDeltaTime);
                                      },
                                      {"reset_triggers"});

    this->_systemScheduler->addSystem("lifetime",
                                      [this](ECS::Registry& reg) {
                                          this->_lifetimeSystem->update(
                                              reg, this->_currentDeltaTime);
                                      },
                                      {"projectile"});

    this->_systemScheduler->addSystem("client_destroy",
                                      [this](ECS::Registry& reg) {
                                          this->_clientDestroySystem->update(
                                              reg, 0.f);
                                      },
                                      {"lifetime"});

    this->_systemScheduler->addSystem("enemy_health_bars",
                                      [this](ECS::Registry& reg) {
                                          this->_enemyHealthBarSystem->update(
                                              reg, this->_currentDeltaTime);
                                      },
                                      {"client_destroy"});

    this->_systemScheduler->addSystem(
        "render",
        [this](ECS::Registry& reg) { this->_renderSystem->update(reg, 0.f); },
        {"enemy_health_bars", "chaser_rotation"});

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

    manager->textureManager->load(
        "bg_menu", config.assets.textures.backgroundTexture.background);
    manager->textureManager->load("bg_sun",
                                  config.assets.textures.backgroundTexture.sun);
    manager->textureManager->load(
        "bg_big_asteroids",
        config.assets.textures.backgroundTexture.bigAsteroids);
    manager->textureManager->load(
        "bg_small_asteroids",
        config.assets.textures.backgroundTexture.smallAsteroids);
    manager->textureManager->load(
        "bg_fst_plan_asteroids",
        config.assets.textures.backgroundTexture.fstPlanAsteroids);
    manager->textureManager->load(
        "bg_snd_plan_asteroids",
        config.assets.textures.backgroundTexture.sndPlanAsteroids);
    manager->textureManager->load(
        "bg_planet_1", config.assets.textures.backgroundTexture.planet1);
    manager->textureManager->load(
        "bg_planet_2", config.assets.textures.backgroundTexture.planet2);
    manager->textureManager->load(
        "bg_planet_3", config.assets.textures.backgroundTexture.planet3);
    manager->textureManager->load("astro_vessel",
                                  config.assets.textures.astroVessel);
    manager->textureManager->load("player_vessel",
                                  config.assets.textures.Player);
    manager->textureManager->load("bdos_enemy_normal",
                                  config.assets.textures.EnemyNormal);
    manager->textureManager->load("bdos_enemy_chaser",
                                  config.assets.textures.EnemyChaser);
    manager->textureManager->load("bdos_enemy_shooter",
                                  config.assets.textures.EnemyShooter);
    manager->textureManager->load("bdos_enemy_heavy",
                                  config.assets.textures.EnemyHeavy);
    manager->textureManager->load("bdos_enemy_patrol",
                                  config.assets.textures.EnemyPatrol);
    manager->textureManager->load("bdos_enemy_wave",
                                  config.assets.textures.EnemyWave);
    manager->textureManager->load("projectile_player_laser",
                                  config.assets.textures.missileLaser);
    manager->textureManager->load("charged_shot",
                                  config.assets.textures.chargedShot);
    manager->textureManager->load("force_pod", config.assets.textures.forcePod);

    // Load power-up textures
    manager->textureManager->load("health_small",
                                  config.assets.textures.healthSmall);
    manager->textureManager->load("health_large",
                                  config.assets.textures.healthLarge);
    manager->textureManager->load("speed_boost",
                                  config.assets.textures.speedBoost);
    manager->textureManager->load("weapon_upgrade",
                                  config.assets.textures.weaponUpgrade);
    manager->textureManager->load("shield", config.assets.textures.shield);
    manager->textureManager->load("rapid_fire",
                                  config.assets.textures.rapidFire);
    manager->textureManager->load("double_damage",
                                  config.assets.textures.damageBoost);
    manager->textureManager->load("extra_life",
                                  config.assets.textures.extraLife);

    manager->textureManager->load(
        "projectile1", config.assets.textures.wallTexture.engrenage1);
    manager->textureManager->load(
        "projectile2", config.assets.textures.wallTexture.engrenage2);
    manager->textureManager->load("projectile3",
                                  config.assets.textures.wallTexture.panneau1);
    manager->textureManager->load("projectile4",
                                  config.assets.textures.wallTexture.panneau2);
    manager->textureManager->load("projectile5",
                                  config.assets.textures.wallTexture.panneau3);
    manager->textureManager->load("projectile6",
                                  config.assets.textures.wallTexture.metal1);
    manager->textureManager->load("projectile7",
                                  config.assets.textures.wallTexture.metal2);
    manager->textureManager->load("projectile8",
                                  config.assets.textures.wallTexture.metal3);
    manager->textureManager->load("projectile9",
                                  config.assets.textures.wallTexture.metal4);
    manager->textureManager->load("projectile10",
                                  config.assets.textures.wallTexture.truc);
    manager->textureManager->load("projectile11",
                                  config.assets.textures.wallTexture.tubeMetal);

    manager->soundManager->load("hover_button", config.assets.sfx.hoverButton);
    manager->soundManager->load("click_button", config.assets.sfx.clickButton);
    manager->soundManager->load("player_spawn", config.assets.sfx.playerSpawn);
    manager->soundManager->load("player_death", config.assets.sfx.playerDeath);
    manager->soundManager->load("bydos_spawn", config.assets.sfx.enemySpawn);
    manager->soundManager->load("bydos_death", config.assets.sfx.enemyDeath);
    manager->soundManager->load("laser_sfx", config.assets.sfx.laser);
    manager->soundManager->load("forcepod_launch",
                                config.assets.sfx.forcePodLaunch);
    manager->soundManager->load("forcepod_return",
                                config.assets.sfx.forcePodReturn);
    manager->soundManager->load("charged_shot", config.assets.sfx.chargedShot);
    manager->soundManager->load("charged_shot_max",
                                config.assets.sfx.chargedShotMax);

    manager->textureManager->get("bg_menu")->setRepeated(true);
    manager->textureManager->get("bg_planet_1")->setRepeated(true);
    manager->textureManager->get("bg_planet_2")->setRepeated(true);
    manager->textureManager->get("bg_planet_3")->setRepeated(true);
    manager->textureManager->get("bg_small_asteroids")->setRepeated(true);
    manager->textureManager->get("bg_big_asteroids")->setRepeated(true);
    manager->textureManager->get("bg_fst_plan_asteroids")->setRepeated(true);
    manager->textureManager->get("bg_snd_plan_asteroids")->setRepeated(true);

    if (_display) {
        _display->loadShader("colorShader", "",
                             "assets/shaders/colorblind.frag");
    }
}

Graphic::Graphic(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<rtype::client::NetworkClient> networkClient,
    std::shared_ptr<rtype::client::ClientNetworkSystem> networkSystem)
    : _registry(std::move(registry)),
      _networkClient(std::move(networkClient)),
      _networkSystem(std::move(networkSystem)) {
    rtype::game::config::RTypeConfigParser parser;
    auto assetsConfig = parser.loadFromFile("./assets/config.toml");
    if (!assetsConfig.has_value()) throw std::exception();
    this->_keybinds = std::make_shared<KeyboardActions>();

#ifdef _WIN32
    this->_displayLoader =
        std::make_unique<rtype::common::DLLoader<rtype::display::IDisplay>>(
            "./display.dll");
#else
    this->_displayLoader =
        std::make_unique<rtype::common::DLLoader<rtype::display::IDisplay>>(
            "./display.so");
#endif
    this->_display = std::shared_ptr<rtype::display::IDisplay>(
        this->_displayLoader->getInstance("createInstanceDisplay"));
    this->_display->open(WINDOW_WIDTH, WINDOW_HEIGHT, "R-Type - Epitech 2025",
                         false);

    this->_keybinds->initialize(*this->_display);

    this->_assetsManager =
        std::make_shared<AssetManager>(assetsConfig.value(), this->_display);

    LOG_DEBUG("[Graphic] Loading entity configurations");
    auto& entityConfigRegistry =
        rtype::games::rtype::shared::EntityConfigRegistry::getInstance();
    entityConfigRegistry.loadEnemiesWithSearch("config/game/enemies.toml");
    entityConfigRegistry.loadPlayersWithSearch("config/game/players.toml");
    entityConfigRegistry.loadProjectilesWithSearch(
        "config/game/projectiles.toml");
    entityConfigRegistry.loadPowerUpsWithSearch("config/game/powerups.toml");
    LOG_DEBUG("[Graphic] Entity configurations loaded");

    this->_initializeCommonAssets();

    if (_networkSystem) {
        _setupNetworkEntityFactory();
    }
    this->_audioLib = std::make_shared<AudioLib>(this->_display);
    this->_registry->setSingleton<std::shared_ptr<AudioLib>>(this->_audioLib);
    this->_registry->setSingleton<AccessibilitySettings>(
        AccessibilitySettings{ColorBlindMode::None, 1.0f, false, false});
    this->_registry->setSingleton<rtype::games::rtype::client::PauseState>(
        rtype::games::rtype::client::PauseState{false});
    this->_initializeCommonAssets();
    this->_sceneManager = std::make_unique<SceneManager>(
        _registry, this->_assetsManager, this->_display, this->_keybinds,
        _networkClient, _networkSystem, this->_audioLib);
    _initializeSystems();
    _lastFrameTime = std::chrono::steady_clock::now();
}

Graphic::~Graphic() {
    if (_display) {
        _display->close();
    }
    _sceneManager.reset();
    _systemScheduler.reset();
    _chargedProjectileAnimationSystem.reset();
    _chargeVisualSystem.reset();
    _chargeInputSystem.reset();
    _enemyHealthBarSystem.reset();
    _forcePodVisualSystem.reset();
    _shaderRenderSystem.reset();
    _clientDestroySystem.reset();
    _lifetimeSystem.reset();
    _projectileSystem.reset();
    _eventSystem.reset();
    _resetTriggersSystem.reset();
    _boxingSystem.reset();
    _renderSystem.reset();
    _parallaxScrolling.reset();
    _buttonUpdateSystem.reset();
    _powerUpCollectionSystem.reset();
    _playerPowerUpVisualSystem.reset();
    _chaserExplosionSystem.reset();
    _chaserRotationSystem.reset();
    _animationSystem.reset();
    _playerAnimationSystem.reset();
    _colorTintSystem.reset();
    if (_registry) {
        _registry->clear();
    }
    _assetsManager.reset();
    _audioLib.reset();
    _display.reset();
    _displayLoader.reset();
}
