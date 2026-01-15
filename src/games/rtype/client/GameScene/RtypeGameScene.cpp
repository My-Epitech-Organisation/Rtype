/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RtypeGameScene.cpp
*/

#include "RtypeGameScene.hpp"

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "AllComponents.hpp"
#include "Components/ChargeShotVisualComponent.hpp"
#include "Components/CountdownComponent.hpp"
#include "Components/TagComponent.hpp"
#include "Components/TextComponent.hpp"
#include "Components/ZIndexComponent.hpp"
#include "Graphic/EntityFactory/EntityFactory.hpp"
#include "GraphicsConstants.hpp"
#include "Logger/Macros.hpp"
#include "RtypeEntityFactory.hpp"
#include "RtypeInputHandler.hpp"
#include "RtypePauseMenu.hpp"
#include "VisualCueFactory.hpp"
#include "client/network/NetworkClient.hpp"
#include "games/rtype/client/GameOverState.hpp"
#include "games/rtype/client/PauseState.hpp"
#include "games/rtype/shared/Components/HealthComponent.hpp"

namespace rs = ::rtype::games::rtype::shared;

namespace rtype::games::rtype::client {

RtypeGameScene::RtypeGameScene(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> assetsManager,
    std::shared_ptr<::rtype::display::IDisplay> display,
    std::shared_ptr<KeyboardActions> keybinds,
    std::function<void(const SceneManager::Scene&)> switchToScene,
    std::function<void(const std::string&)> setBackground,
    std::function<void(const std::string&)> setLevelMusic,
    std::shared_ptr<::rtype::client::NetworkClient> networkClient,
    std::shared_ptr<::rtype::client::ClientNetworkSystem> networkSystem,
    std::shared_ptr<AudioLib> audioLib)
    : AGameScene(std::move(registry), std::move(assetsManager), display,
                 std::move(keybinds), std::move(switchToScene),
                 std::move(setBackground), std::move(setLevelMusic),
                 std::move(networkClient), std::move(networkSystem)),
      _movementSystem(
          std::make_unique<::rtype::games::rtype::shared::MovementSystem>()),
      _laserBeamAnimationSystem(std::make_unique<LaserBeamAnimationSystem>()) {
    if (_networkClient) {
        auto registry = _registry;
        auto switchToScene = _switchToScene;
        _networkClient->onGameOver(
            [registry, switchToScene](
                const ::rtype::client::GameOverEvent& event) mutable {
                if (registry) {
                    if (!registry->hasSingleton<
                            ::rtype::games::rtype::client::GameOverState>()) {
                        registry->setSingleton<
                            ::rtype::games::rtype::client::GameOverState>(
                            ::rtype::games::rtype::client::GameOverState{
                                event.finalScore, event.isVictory});
                    } else {
                        auto& state = registry->getSingleton<
                            ::rtype::games::rtype::client::GameOverState>();
                        state.finalScore = event.finalScore;
                        state.isVictory = event.isVictory;
                    }
                }
                if (switchToScene) {
                    switchToScene(SceneManager::GAME_OVER);
                }
            });
    }
}

RtypeGameScene::~RtypeGameScene() {
    LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                  "[RtypeGameScene] Destructor called");

    _isDisconnected = true;

    if (_networkClient) {
        _networkClient->clearPendingCallbacks();
        _networkClient->onGameOver(nullptr);
        _networkClient->onEntityMove(nullptr);
        _networkClient->onEntityMoveBatch(nullptr);
        _networkClient->onEntityHealth(nullptr);
        _networkClient->clearDisconnectedCallbacks();
        _networkClient->onGameStateChange(nullptr);
    }
    if (_bandwidthIndicatorEntity.has_value()) {
        _registry->killEntity(*this->_bandwidthIndicatorEntity);
    }

    clearDamageVignette();
    _vignetteEntities.clear();
    _vignetteAlpha = 0.0F;
    LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                  "[RtypeGameScene] Destructor completed");
}

std::vector<ECS::Entity> RtypeGameScene::initialize() {
    LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                  "[RtypeGameScene] Initialize called");
    std::vector<ECS::Entity> entities;

    auto bgEntities = EntityFactory::createBackground(
        this->_registry, this->_assetsManager, "", nullptr);
    entities.insert(entities.end(), bgEntities.begin(), bgEntities.end());
    LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                  "[RtypeGameScene] Background created with "
                      << bgEntities.size() << " entities");
    if (_networkSystem) {
        _networkSystem->registerCallbacks();
        LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                      "[RtypeGameScene] Setting up local player callback");
        setupLocalPlayerCallback();
        LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                      "[RtypeGameScene] Setting up health update callback");
        _networkSystem->onHealthUpdate(
            [this](const ::rtype::client::EntityHealthEvent& event) {
                handleHealthUpdate(event);
            });
        LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                      "[RtypeGameScene] Setting up disconnect callback");
        setupDisconnectCallback();
        LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                      "[RtypeGameScene] Network callbacks configured");
    }

    LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                  "[RtypeGameScene] Constructor started");
    LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                  "[RtypeGameScene] _networkClient is "
                      << (_networkClient ? "valid" : "NULL"));
    LOG_DEBUG_CAT(
        ::rtype::LogCategory::UI,
        "[RtypeGameScene] _registry is " << (_registry ? "valid" : "NULL"));
    LOG_DEBUG_CAT(::rtype::LogCategory::UI, "[RtypeGameScene] Setting up HUD");
    setupHud();
    setupLevelAnnounceCallback();
    setupDamageVignette();
    updateBandwidthIndicator();
    setupBandwidthModeCallback();
    LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                  "[RtypeGameScene] HUD setup complete");
    auto pauseEntities = RtypePauseMenu::createPauseMenu(
        _registry, _assetsManager, _switchToScene);
    entities.insert(entities.end(), pauseEntities.begin(), pauseEntities.end());
    LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                  "[RtypeGameScene] Pause menu created with "
                      << pauseEntities.size() << " entities");

    if (!_registry->hasSingleton<PauseState>()) {
        _registry->setSingleton<PauseState>(PauseState{false});
        LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                      "[RtypeGameScene] PauseState singleton created");
    } else {
        _registry->getSingleton<PauseState>().isPaused = false;
        LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                      "[RtypeGameScene] PauseState reset to unpaused");
    }

    LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                  "[RtypeGameScene] Initialize completed, total entities: "
                      << entities.size());
    return entities;
}

void RtypeGameScene::update() {
    const float dt = 0.016f;
    _uiTimer += dt;

    updateDamageVignette(dt);
    updatePingDisplay();
    updateLevelAnnounce(dt);

    if (_registry->hasSingleton<std::shared_ptr<AudioLib>>()) {
        auto audioLib = _registry->getSingleton<std::shared_ptr<AudioLib>>();
        if (audioLib) {
            audioLib->update();
        }
    }

    if (_isDisconnected) {
        if (_networkSystem) {
            _networkSystem->update();
        }
        return;
    }

    if (_damageFlashTimer > 0.0F) {
        _damageFlashTimer = std::max(0.0F, _damageFlashTimer - dt);
        if (_damageFlashTimer == 0.0F) {
            resetHudColors();
        }
    }

    if (_bandwidthNotificationTimer > 0.0F) {
        _bandwidthNotificationTimer =
            std::max(0.0F, _bandwidthNotificationTimer - dt);
        if (_bandwidthNotificationTimer == 0.0F) {
            if (_bandwidthNotificationEntity.has_value() &&
                _registry->isAlive(*_bandwidthNotificationEntity)) {
                _registry->killEntity(*_bandwidthNotificationEntity);
                _bandwidthNotificationEntity.reset();
            }
        }
    }

    if (_movementSystem && _registry) {
        _movementSystem->update(*_registry, dt);
    }

    if (_laserBeamAnimationSystem && _registry) {
        _laserBeamAnimationSystem->update(*_registry, dt);
    }

    bool isPaused = false;
    if (_registry->hasSingleton<PauseState>()) {
        isPaused = _registry->getSingleton<PauseState>().isPaused;
    }

    if (isPaused) {
        return;
    }

    std::uint16_t inputMask = getInputMask();

    if (_networkSystem && _networkSystem->isConnected()) {
        bool shouldSend = false;

        constexpr std::uint16_t kMovementMask =
            ::rtype::network::InputMask::kUp |
            ::rtype::network::InputMask::kDown |
            ::rtype::network::InputMask::kLeft |
            ::rtype::network::InputMask::kRight;

        std::uint16_t currentMovement = inputMask & kMovementMask;
        std::uint16_t lastMovement = _lastInputMask & kMovementMask;

        if (currentMovement != lastMovement) {
            shouldSend = true;
        }

        bool isShootingNow =
            (inputMask & ::rtype::network::InputMask::kShoot) != 0;
        bool wasShootingLast =
            (_lastInputMask & ::rtype::network::InputMask::kShoot) != 0;
        bool isChargedShotNow =
            (inputMask & ::rtype::network::InputMask::kChargeLevelMask) != 0;
        bool wasChargedShotLast =
            (_lastInputMask & ::rtype::network::InputMask::kChargeLevelMask) !=
            0;

        if (isShootingNow) {
            if (!wasShootingLast) {
                shouldSend = true;
                _shootInputClock.restart();
            } else if (_shootInputClock.getElapsedTime().asSeconds() >=
                       kShootSendInterval) {
                shouldSend = true;
                _shootInputClock.restart();
            }
        } else if (wasShootingLast) {
            shouldSend = true;
        }
        if (isChargedShotNow && !wasChargedShotLast) {
            LOG_INFO("[RtypeGameScene] *** SENDING CHARGED SHOT *** mask=0x"
                     << std::hex << static_cast<int>(inputMask));
            shouldSend = true;
        }

        bool isWeaponSwitchNow =
            (inputMask & ::rtype::network::InputMask::kWeaponSwitch) != 0;
        bool wasWeaponSwitchLast =
            (_lastInputMask & ::rtype::network::InputMask::kWeaponSwitch) != 0;
        if (isWeaponSwitchNow != wasWeaponSwitchLast) {
            shouldSend = true;
        }

        bool isForcePodNow =
            (inputMask & ::rtype::network::InputMask::kForcePod) != 0;
        bool wasForcePodLast =
            (_lastInputMask & ::rtype::network::InputMask::kForcePod) != 0;
        if (isForcePodNow != wasForcePodLast) {
            shouldSend = true;
        }

        if (shouldSend) {
            _networkSystem->sendInput(inputMask);
            if (_registry && _registry->hasSingleton<ChargeShotInputState>()) {
                auto& chargeState =
                    _registry->getSingleton<ChargeShotInputState>();
                if (chargeState.shouldFireShot) {
                    chargeState.shouldFireShot = false;
                    chargeState.releasedChargeLevel =
                        ::rtype::games::rtype::shared::ChargeLevel::None;
                    LOG_DEBUG_CAT(::rtype::LogCategory::Input,
                                  "[RtypeGameScene] Reset shouldFireShot flag "
                                  "after sending input");
                }
            }
        }
        _lastInputMask = inputMask;
    }
}

void RtypeGameScene::render(::rtype::display::IDisplay& display) {
    // R-Type specific rendering if needed
}

void RtypeGameScene::pollEvents(const ::rtype::display::Event& event) {
    if (event.type == ::rtype::display::EventType::KeyPressed) {
        LOG_INFO("[RtypeGameScene] Key pressed: "
                 << static_cast<int>(event.key.code));
    }
    if (event.type == ::rtype::display::EventType::KeyPressed ||
        event.type == ::rtype::display::EventType::KeyReleased) {
        RtypeInputHandler::updateKeyState(event);
    }
    if (event.type == ::rtype::display::EventType::KeyPressed) {
        auto toggleKey =
            _keybinds->getKeyBinding(GameAction::TOGGLE_LOW_BANDWIDTH);
        if (toggleKey.has_value() && event.key.code == *toggleKey) {
            toggleLowBandwidthMode();
        }
    }
    if (event.type == ::rtype::display::EventType::KeyReleased ||
        event.type == ::rtype::display::EventType::JoystickButtonReleased) {
        RtypeInputHandler::handleKeyReleasedEvent(event, _keybinds, _registry);
    }
    auto chargeShotKey = _keybinds->getKeyBinding(GameAction::CHARGE_SHOT);
    if (event.type == ::rtype::display::EventType::KeyPressed &&
        chargeShotKey.has_value() && event.key.code == *chargeShotKey) {
        LOG_INFO("[RtypeGameScene] *** CHARGE SHOT KEY PRESSED ***");
        if (!_registry->hasSingleton<ChargeShotInputState>()) {
            _registry->setSingleton<ChargeShotInputState>();
        }
        _registry->getSingleton<ChargeShotInputState>().isPressed = true;
    }
    if (event.type == ::rtype::display::EventType::KeyReleased &&
        chargeShotKey.has_value() && event.key.code == *chargeShotKey) {
        LOG_INFO("[RtypeGameScene] *** CHARGE SHOT KEY RELEASED ***");
        if (_registry->hasSingleton<ChargeShotInputState>()) {
            _registry->getSingleton<ChargeShotInputState>().isPressed = false;
        }
    }
    auto chargeShotBtn =
        _keybinds->getJoyButtonBinding(GameAction::CHARGE_SHOT);
    if (event.type == ::rtype::display::EventType::JoystickButtonPressed &&
        chargeShotBtn.has_value() &&
        event.joystickButton.button == *chargeShotBtn) {
        LOG_INFO(
            "[RtypeGameScene] *** CHARGE SHOT JOYSTICK BUTTON PRESSED ***");
        if (!_registry->hasSingleton<ChargeShotInputState>()) {
            _registry->setSingleton<ChargeShotInputState>();
        }
        _registry->getSingleton<ChargeShotInputState>().isPressed = true;
    }
    if (event.type == ::rtype::display::EventType::JoystickButtonReleased &&
        chargeShotBtn.has_value() &&
        event.joystickButton.button == *chargeShotBtn) {
        LOG_INFO(
            "[RtypeGameScene] *** CHARGE SHOT JOYSTICK BUTTON RELEASED ***");
        if (_registry->hasSingleton<ChargeShotInputState>()) {
            _registry->getSingleton<ChargeShotInputState>().isPressed = false;
        }
    }
    if (event.type == ::rtype::display::EventType::MouseButtonPressed &&
        event.mouseButton.button == ::rtype::display::MouseButton::Right) {
        LOG_INFO("[RtypeGameScene] *** CHARGE SHOT MOUSE RIGHT PRESSED ***");
        if (!_registry->hasSingleton<ChargeShotInputState>()) {
            _registry->setSingleton<ChargeShotInputState>();
        }
        _registry->getSingleton<ChargeShotInputState>().isPressed = true;
    }
    if (event.type == ::rtype::display::EventType::MouseButtonReleased &&
        event.mouseButton.button == ::rtype::display::MouseButton::Right) {
        LOG_INFO("[RtypeGameScene] *** CHARGE SHOT MOUSE RIGHT RELEASED ***");
        if (_registry->hasSingleton<ChargeShotInputState>()) {
            _registry->getSingleton<ChargeShotInputState>().isPressed = false;
        }
    }
}

std::uint16_t RtypeGameScene::getInputMask() const {
    std::uint16_t mask = RtypeInputHandler::getInputMask(_keybinds);
    if (_registry && _registry->hasSingleton<ChargeShotInputState>()) {
        auto& chargeState = _registry->getSingleton<ChargeShotInputState>();
        if (chargeState.isPressed && !chargeState.shouldFireShot) {
            mask &= ~::rtype::network::InputMask::kShoot;
        }
        if (chargeState.shouldFireShot) {
            mask &= ~(::rtype::network::InputMask::kShoot |
                      ::rtype::network::InputMask::kChargeLevelMask);
            switch (chargeState.releasedChargeLevel) {
                case ::rtype::games::rtype::shared::ChargeLevel::Level1:
                    mask |= ::rtype::network::InputMask::kChargeLevel1;
                    break;
                case ::rtype::games::rtype::shared::ChargeLevel::Level2:
                    mask |= ::rtype::network::InputMask::kChargeLevel2;
                    break;
                case ::rtype::games::rtype::shared::ChargeLevel::Level3:
                    mask |= ::rtype::network::InputMask::kChargeLevel3;
                    break;
                default:
                    mask |= ::rtype::network::InputMask::kShoot;
                    break;
            }
            LOG_DEBUG_CAT(
                ::rtype::LogCategory::Input,
                "[RtypeGameScene] Charged shot released at level "
                    << static_cast<int>(chargeState.releasedChargeLevel)
                    << ", mask=0x" << std::hex << static_cast<int>(mask));
        }
    }
    return mask;
}

void RtypeGameScene::setupEntityFactory() {
    LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                  "[RtypeGameScene] Setting up entityFactory");
    _networkSystem->setEntityFactory(
        RtypeEntityFactory::createNetworkEntityFactory(_registry,
                                                       _assetsManager));
}

void RtypeGameScene::setupLocalPlayerCallback() {
    auto registry = this->_registry;
    _networkSystem->onLocalPlayerAssigned(
        [this, registry](std::uint32_t userId, ECS::Entity entity) {
            if (registry->isAlive(entity)) {
                registry->emplaceComponent<ControllableTag>(entity);
                if (!registry->hasComponent<rs::ChargeComponent>(entity)) {
                    registry->emplaceComponent<rs::ChargeComponent>(entity);
                }
                if (!registry->hasComponent<ChargeShotVisual>(entity)) {
                    registry->emplaceComponent<ChargeShotVisual>(entity);
                }
                if (!registry->hasComponent<ChargeBarUI>(entity)) {
                    registry->emplaceComponent<ChargeBarUI>(entity);
                }
                if (!registry->hasComponent<ColorTint>(entity)) {
                    registry->emplaceComponent<ColorTint>(entity);
                }
                LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                              "[RtypeGameScene] Local player entity assigned "
                              "with charge visual components");
            }
            _localPlayerEntity = entity;
            _localPlayerId = userId;
            LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                          "[RtypeGameScene] Local player ID set to " << userId);
            if (registry->isAlive(entity) &&
                registry->hasComponent<rs::HealthComponent>(entity)) {
                const auto& health =
                    registry->getComponent<rs::HealthComponent>(entity);
                LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                              "[RtypeGameScene] Initial health: "
                                  << health.current << "/" << health.max);
                updateLivesDisplay(health.current, health.max);
            }
        });
}

void RtypeGameScene::setupHud() {
    const float barWidth = 220.f;
    const float barHeight = 18.f;
    const ::rtype::display::Vector2f barPos{20.f, 20.f};

    auto bg = _registry->spawnEntity();
    _registry->emplaceComponent<rs::TransformComponent>(bg, barPos.x, barPos.y);
    _registry->emplaceComponent<Rectangle>(
        bg, std::pair<float, float>{barWidth, barHeight},
        ::rtype::display::Color(30, 35, 45, 220),
        ::rtype::display::Color(30, 35, 45, 220));
    _registry->emplaceComponent<ZIndex>(bg, GraphicsConfig::ZINDEX_UI);
    _registry->emplaceComponent<HudTag>(bg);
    _registry->emplaceComponent<GameTag>(bg);
    _healthBarBgEntity = bg;

    auto fill = _registry->spawnEntity();
    _registry->emplaceComponent<rs::TransformComponent>(fill, barPos.x,
                                                        barPos.y);
    _registry->emplaceComponent<Rectangle>(
        fill, std::pair<float, float>{barWidth, barHeight},
        ::rtype::display::Color(90, 220, 140, 240),
        ::rtype::display::Color(90, 220, 140, 240));
    _registry->emplaceComponent<ZIndex>(fill, GraphicsConfig::ZINDEX_UI + 1);
    _registry->emplaceComponent<HudTag>(fill);
    _registry->emplaceComponent<GameTag>(fill);
    _healthBarFillEntity = fill;

    auto hpText = EntityFactory::createStaticText(
        _registry, _assetsManager, "HP: 100/100", "title_font",
        ::rtype::display::Vector2f{barPos.x + barWidth / 2.0f,
                                   barPos.y + barHeight / 2.0f},
        20.f);
    _registry->emplaceComponent<CenteredTextTag>(hpText);
    _registry->emplaceComponent<ZIndex>(hpText, GraphicsConfig::ZINDEX_UI + 2);
    _registry->emplaceComponent<HudTag>(hpText);
    _registry->emplaceComponent<GameTag>(hpText);
    _healthTextEntity = hpText;
    _livesTextEntity = hpText;

    auto pingText = EntityFactory::createStaticText(
        _registry, _assetsManager, "Ping: 0ms", "title_font",
        ::rtype::display::Vector2f{1800.f, 20.f}, 20.f);
    _registry->emplaceComponent<ZIndex>(pingText,
                                        GraphicsConfig::ZINDEX_UI + 2);
    _registry->emplaceComponent<HudTag>(pingText);
    _registry->emplaceComponent<GameTag>(pingText);
    _pingTextEntity = pingText;
}

void RtypeGameScene::updateLivesDisplay(int current, int max) {
    LOG_DEBUG_CAT(
        ::rtype::LogCategory::UI,
        "[RtypeGameScene] updateLivesDisplay: " << current << "/" << max);
    _lastKnownLives = current;
    _lastKnownMaxLives = max;
    updateHealthBar(current, max);
}

void RtypeGameScene::handleHealthUpdate(
    const ::rtype::client::EntityHealthEvent& event) {
    LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                  "[RtypeGameScene] Health update for local player: current="
                      << event.current << " max=" << event.max);

    if (_lastKnownLives > event.current) {
        int damageAmount = _lastKnownLives - event.current;
        triggerDamageFlash(damageAmount);
        spawnDamagePopup(damageAmount);
    }
    if (event.current <= 0) {
        clearDamageVignette();
        setHealthBarVisible(false);
    } else {
        setHealthBarVisible(true);
    }
    updateLivesDisplay(event.current, event.max);
}

void RtypeGameScene::updateHealthBar(int current, int max) {
    LOG_DEBUG_CAT(
        ::rtype::LogCategory::UI,
        "[RtypeGameScene] updateHealthBar called: " << current << "/" << max);
    if (!_healthBarFillEntity.has_value() ||
        !_registry->isAlive(*_healthBarFillEntity)) {
        LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                      "[RtypeGameScene] Health bar fill entity not valid");
        return;
    }

    const float barWidth = 220.f;
    const float ratio =
        max > 0
            ? std::max(0.f, std::min(1.f, current / static_cast<float>(max)))
            : 0.f;
    auto& rect = _registry->getComponent<Rectangle>(*_healthBarFillEntity);
    rect.size.first = barWidth * ratio;
    LOG_DEBUG_CAT(
        ::rtype::LogCategory::UI,
        "[RtypeGameScene] Health bar width set to: " << rect.size.first);

    if (_healthTextEntity.has_value() &&
        _registry->isAlive(*_healthTextEntity) &&
        _registry->hasComponent<Text>(*_healthTextEntity)) {
        auto& text = _registry->getComponent<Text>(*_healthTextEntity);
        text.textContent =
            "HP: " + std::to_string(current) + "/" + std::to_string(max);
        LOG_DEBUG_CAT(
            ::rtype::LogCategory::UI,
            "[RtypeGameScene] HP text updated to: " << text.textContent);
    } else {
        LOG_DEBUG_CAT(
            ::rtype::LogCategory::UI,
            "[RtypeGameScene] Health text entity not valid for update");
    }
}

void RtypeGameScene::updatePingDisplay() {
    if (!_pingTextEntity.has_value() || !_registry->isAlive(*_pingTextEntity) ||
        !_registry->hasComponent<Text>(*_pingTextEntity)) {
        return;
    }

    if (_networkClient && _networkClient->isConnected()) {
        std::uint32_t latency = _networkClient->latencyMs();
        auto& text = _registry->getComponent<Text>(*_pingTextEntity);
        text.textContent = "Ping: " + std::to_string(latency) + "ms";

        if (latency < 50) {
            text.color = ::rtype::display::Color(90, 220, 140, 255);
        } else if (latency < 100) {
            text.color = ::rtype::display::Color(220, 220, 90, 255);
        } else if (latency < 200) {
            text.color = ::rtype::display::Color(255, 165, 0, 255);
        } else {
            text.color = ::rtype::display::Color(220, 90, 90, 255);
        }
    }
}

void RtypeGameScene::setupDamageVignette() {
    const float screenWidth = static_cast<float>(_display->getWindowSize().x);
    const float screenHeight = static_cast<float>(_display->getWindowSize().y);

    _vignetteEntities.clear();

    for (int layer = 0; layer < kVignetteLayers; ++layer) {
        float layerRatio =
            static_cast<float>(layer) / static_cast<float>(kVignetteLayers - 1);
        float inset = layerRatio * 80.f;
        float thickness = 50.f + layerRatio * 40.f;

        const ::rtype::display::Color vignetteColor(255, 0, 0, 0);

        auto top = _registry->spawnEntity();
        _registry->emplaceComponent<rs::TransformComponent>(top, 0.f, inset);
        _registry->emplaceComponent<Rectangle>(
            top, std::pair<float, float>{screenWidth, thickness}, vignetteColor,
            vignetteColor);
        _registry->emplaceComponent<ZIndex>(
            top, GraphicsConfig::ZINDEX_UI + 100 + layer);
        _registry->emplaceComponent<HudTag>(top);
        _registry->emplaceComponent<GameTag>(top);
        _vignetteEntities.push_back(top);

        auto bottom = _registry->spawnEntity();
        _registry->emplaceComponent<rs::TransformComponent>(
            bottom, 0.f, screenHeight - inset - thickness);
        _registry->emplaceComponent<Rectangle>(
            bottom, std::pair<float, float>{screenWidth, thickness},
            vignetteColor, vignetteColor);
        _registry->emplaceComponent<ZIndex>(
            bottom, GraphicsConfig::ZINDEX_UI + 100 + layer);
        _registry->emplaceComponent<HudTag>(bottom);
        _registry->emplaceComponent<GameTag>(bottom);
        _vignetteEntities.push_back(bottom);

        auto left = _registry->spawnEntity();
        _registry->emplaceComponent<rs::TransformComponent>(left, inset, 0.f);
        _registry->emplaceComponent<Rectangle>(
            left, std::pair<float, float>{thickness, screenHeight},
            vignetteColor, vignetteColor);
        _registry->emplaceComponent<ZIndex>(
            left, GraphicsConfig::ZINDEX_UI + 100 + layer);
        _registry->emplaceComponent<HudTag>(left);
        _registry->emplaceComponent<GameTag>(left);
        _vignetteEntities.push_back(left);

        auto right = _registry->spawnEntity();
        _registry->emplaceComponent<rs::TransformComponent>(
            right, screenWidth - inset - thickness, 0.f);
        _registry->emplaceComponent<Rectangle>(
            right, std::pair<float, float>{thickness, screenHeight},
            vignetteColor, vignetteColor);
        _registry->emplaceComponent<ZIndex>(
            right, GraphicsConfig::ZINDEX_UI + 100 + layer);
        _registry->emplaceComponent<HudTag>(right);
        _registry->emplaceComponent<GameTag>(right);
        _vignetteEntities.push_back(right);
    }

    _lastVignetteSize = {0, 0};
    refreshDamageVignetteLayout();
}

void RtypeGameScene::refreshDamageVignetteLayout() {
    if (_vignetteEntities.size() < kVignetteLayers * 4) {
        return;
    }

    const ::rtype::display::Vector2i currentSize = _display->getWindowSize();
    const ::rtype::display::Vector2f viewCenter = _display->getViewCenter();
    const ::rtype::display::Vector2f viewSize = _display->getViewSize();
    const float viewLeft = viewCenter.x - viewSize.x / 2.0f;
    const float viewTop = viewCenter.y - viewSize.y / 2.0f;
    const float screenWidth = viewSize.x;
    const float screenHeight = viewSize.y;

    int entityIndex = 0;
    for (int layer = 0; layer < kVignetteLayers; ++layer) {
        float layerRatio =
            static_cast<float>(layer) / static_cast<float>(kVignetteLayers - 1);
        float inset = layerRatio * 80.f;
        float thickness = 50.f + layerRatio * 40.f;

        if (entityIndex < static_cast<int>(_vignetteEntities.size())) {
            auto ent = _vignetteEntities[entityIndex++];
            if (_registry->isAlive(ent) &&
                _registry->hasComponent<rs::TransformComponent>(ent) &&
                _registry->hasComponent<Rectangle>(ent)) {
                auto& pos =
                    _registry->getComponent<rs::TransformComponent>(ent);
                pos.x = viewLeft;
                pos.y = viewTop + inset;
                auto& rect = _registry->getComponent<Rectangle>(ent);
                rect.size = {screenWidth, thickness};
            }
        }

        if (entityIndex < static_cast<int>(_vignetteEntities.size())) {
            auto ent = _vignetteEntities[entityIndex++];
            if (_registry->isAlive(ent) &&
                _registry->hasComponent<rs::TransformComponent>(ent) &&
                _registry->hasComponent<Rectangle>(ent)) {
                auto& pos =
                    _registry->getComponent<rs::TransformComponent>(ent);
                pos.x = viewLeft;
                pos.y = viewTop + screenHeight - inset - thickness;
                auto& rect = _registry->getComponent<Rectangle>(ent);
                rect.size = {screenWidth, thickness};
            }
        }

        if (entityIndex < static_cast<int>(_vignetteEntities.size())) {
            auto ent = _vignetteEntities[entityIndex++];
            if (_registry->isAlive(ent) &&
                _registry->hasComponent<rs::TransformComponent>(ent) &&
                _registry->hasComponent<Rectangle>(ent)) {
                auto& pos =
                    _registry->getComponent<rs::TransformComponent>(ent);
                pos.x = viewLeft + inset;
                pos.y = viewTop;
                auto& rect = _registry->getComponent<Rectangle>(ent);
                rect.size = {thickness, screenHeight};
            }
        }

        if (entityIndex < static_cast<int>(_vignetteEntities.size())) {
            auto ent = _vignetteEntities[entityIndex++];
            if (_registry->isAlive(ent) &&
                _registry->hasComponent<rs::TransformComponent>(ent) &&
                _registry->hasComponent<Rectangle>(ent)) {
                auto& pos =
                    _registry->getComponent<rs::TransformComponent>(ent);
                pos.x = viewLeft + screenWidth - inset - thickness;
                pos.y = viewTop;
                auto& rect = _registry->getComponent<Rectangle>(ent);
                rect.size = {thickness, screenHeight};
            }
        }
    }

    _lastVignetteSize = currentSize;
}

void RtypeGameScene::updateDamageVignette(float deltaTime) {
    refreshDamageVignetteLayout();
    if (_vignetteAlpha <= 0.0F) {
        return;
    }
    _vignetteAlpha =
        std::max(0.0F, _vignetteAlpha - kVignetteFadeSpeed * deltaTime);

    int layerIndex = 0;
    for (auto& entity : _vignetteEntities) {
        if (_registry->isAlive(entity) &&
            _registry->hasComponent<Rectangle>(entity)) {
            int layer = layerIndex / 4;
            float layerRatio = static_cast<float>(layer) /
                               static_cast<float>(kVignetteLayers - 1);
            float layerAlpha = _vignetteAlpha * (1.0f - layerRatio * 0.7f);

            auto& rect = _registry->getComponent<Rectangle>(entity);
            rect.currentColor = ::rtype::display::Color(
                255, 0, 0, static_cast<uint8_t>(layerAlpha));
            rect.mainColor = rect.currentColor;
        }
        ++layerIndex;
    }
}

void RtypeGameScene::clearDamageVignette() {
    _vignetteAlpha = 0.0F;
    int layerIndex = 0;
    for (auto& entity : _vignetteEntities) {
        if (_registry->isAlive(entity) &&
            _registry->hasComponent<Rectangle>(entity)) {
            auto& rect = _registry->getComponent<Rectangle>(entity);
            rect.currentColor = ::rtype::display::Color(255, 0, 0, 0);
            rect.mainColor = rect.currentColor;
        }
        ++layerIndex;
    }
}

void RtypeGameScene::setHealthBarVisible(bool visible) {
    auto toggleEntity = [this, visible](std::optional<ECS::Entity> optEnt) {
        if (!optEnt.has_value()) return;
        auto ent = *optEnt;
        if (!_registry->isAlive(ent)) return;
        if (_registry->hasComponent<HiddenComponent>(ent)) {
            _registry->getComponent<HiddenComponent>(ent).isHidden = !visible;
        } else {
            _registry->emplaceComponent<HiddenComponent>(
                ent, HiddenComponent{!visible});
        }
    };

    toggleEntity(_healthBarBgEntity);
    toggleEntity(_healthBarFillEntity);
    toggleEntity(_healthTextEntity);
    toggleEntity(_livesTextEntity);
}

void RtypeGameScene::triggerDamageFlash(int damageAmount) {
    (void)damageAmount;
    _damageFlashTimer = 0.5F;
    _vignetteAlpha = kVignetteMaxAlpha;
    int layerIndex = 0;
    for (auto& entity : _vignetteEntities) {
        if (_registry->isAlive(entity) &&
            _registry->hasComponent<Rectangle>(entity)) {
            int layer = layerIndex / 4;
            float layerRatio = static_cast<float>(layer) /
                               static_cast<float>(kVignetteLayers - 1);
            float layerAlpha = _vignetteAlpha * (1.0f - layerRatio * 0.7f);

            auto& rect = _registry->getComponent<Rectangle>(entity);
            rect.currentColor = ::rtype::display::Color(
                255, 0, 0, static_cast<uint8_t>(layerAlpha));
            rect.mainColor = rect.currentColor;
        }
        ++layerIndex;
    }

    if (_healthBarFillEntity.has_value() &&
        _registry->isAlive(*_healthBarFillEntity)) {
        auto& rect = _registry->getComponent<Rectangle>(*_healthBarFillEntity);
        rect.currentColor = ::rtype::display::Color(255, 80, 80, 240);
    }

    if (_healthTextEntity.has_value() &&
        _registry->isAlive(*_healthTextEntity) &&
        _registry->hasComponent<Text>(*_healthTextEntity)) {
        auto& text = _registry->getComponent<Text>(*_healthTextEntity);
        text.color = ::rtype::display::Color(255, 100, 100, 255);
    }
}

void RtypeGameScene::spawnDamagePopup(int damage) {
    LOG_DEBUG_CAT(
        ::rtype::LogCategory::UI,
        "[RtypeGameScene] spawnDamagePopup called with damage=" << damage);

    std::optional<ECS::Entity> playerEntity = _localPlayerEntity;
    if (!playerEntity.has_value() && _networkSystem) {
        playerEntity = _networkSystem->getLocalPlayerEntity();
    }

    if (!playerEntity.has_value()) {
        LOG_DEBUG_CAT(
            ::rtype::LogCategory::UI,
            "[RtypeGameScene] No player entity available for damage popup");
        return;
    }

    if (!_registry->isAlive(*playerEntity)) {
        LOG_DEBUG_CAT(
            ::rtype::LogCategory::UI,
            "[RtypeGameScene] Player entity not alive for damage popup");
        return;
    }

    if (!_registry->hasComponent<rs::TransformComponent>(*playerEntity)) {
        LOG_DEBUG_CAT(
            ::rtype::LogCategory::UI,
            "[RtypeGameScene] Player entity has no Position for damage popup");
        return;
    }

    const auto& pos =
        _registry->getComponent<rs::TransformComponent>(*playerEntity);
    LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                  "[RtypeGameScene] Player position for popup: ("
                      << pos.x << ", " << pos.y << ")");

    if (!_assetsManager || !_assetsManager->fontManager) {
        LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                      "[RtypeGameScene] No assets manager for damage popup");
        return;
    }

    try {
        auto font = "title_font";
        VisualCueFactory::createDamagePopup(
            *_registry, ::rtype::display::Vector2f(pos.x + 20.f, pos.y - 10.f),
            damage, font, ::rtype::display::Color(255, 60, 60, 255));
        LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                      "[RtypeGameScene] Damage popup created successfully");
    } catch (const std::exception& e) {
        LOG_DEBUG_CAT(
            ::rtype::LogCategory::UI,
            "[RtypeGameScene] Failed to create damage popup: " << e.what());
    }
}

void RtypeGameScene::resetHudColors() {
    if (_healthBarFillEntity.has_value() &&
        _registry->isAlive(*_healthBarFillEntity)) {
        auto& rect = _registry->getComponent<Rectangle>(*_healthBarFillEntity);
        rect.currentColor = ::rtype::display::Color(90, 220, 140, 240);
    }

    if (_healthTextEntity.has_value() &&
        _registry->isAlive(*_healthTextEntity) &&
        _registry->hasComponent<Text>(*_healthTextEntity)) {
        auto& text = _registry->getComponent<Text>(*_healthTextEntity);
        text.color = ::rtype::display::Color::White();
    }
}

void RtypeGameScene::setupDisconnectCallback() {
    _networkSystem->onDisconnect([this](network::DisconnectReason reason) {
        showDisconnectModal(reason);
    });
}

void RtypeGameScene::showDisconnectModal(network::DisconnectReason reason) {
    if (_isDisconnected) {
        return;
    }
    _isDisconnected = true;
    std::string reasonMessage = getDisconnectMessage(reason);
    LOG_INFO("[RtypeGameScene] Disconnected from server, reason="
             << static_cast<int>(reason) << " message=" << reasonMessage);

    const ::rtype::display::Vector2i windowSize = _display->getWindowSize();
    const float centerX = windowSize.x / 2.0F;
    const float centerY = windowSize.y / 2.0F;

    auto overlayEntity = _registry->spawnEntity();
    auto overlaySize = std::make_pair(static_cast<float>(windowSize.x),
                                      static_cast<float>(windowSize.y));
    _registry->emplaceComponent<Rectangle>(
        overlayEntity, overlaySize, ::rtype::display::Color(0, 0, 0, 180),
        ::rtype::display::Color(0, 0, 0, 180));
    auto& overlayPos =
        _registry->emplaceComponent<rs::TransformComponent>(overlayEntity);
    overlayPos.x = 0;
    overlayPos.y = 0;
    _registry->emplaceComponent<ZIndex>(overlayEntity, 9000);
    _disconnectOverlayEntity = overlayEntity;

    auto panelEntity = _registry->spawnEntity();
    auto panelSize = std::make_pair(500.0F, 300.0F);
    _registry->emplaceComponent<Rectangle>(
        panelEntity, panelSize, ::rtype::display::Color(40, 40, 60, 255),
        ::rtype::display::Color(40, 40, 60, 255));
    auto& panelRect = _registry->getComponent<Rectangle>(panelEntity);
    panelRect.outlineColor = ::rtype::display::Color(120, 120, 150, 255);
    panelRect.outlineThickness = 3.0F;
    auto& panelPos =
        _registry->emplaceComponent<rs::TransformComponent>(panelEntity);
    panelPos.x = centerX - 250.0F;
    panelPos.y = centerY - 150.0F;
    _registry->emplaceComponent<ZIndex>(panelEntity, 9001);
    _disconnectPanelEntity = panelEntity;

    auto titleEntity = _registry->spawnEntity();
    auto titleFont = "title_font";
    _registry->emplaceComponent<Text>(
        titleEntity, titleFont, ::rtype::display::Color(255, 100, 100, 255), 36,
        "Connection Lost");
    auto& titlePos =
        _registry->emplaceComponent<rs::TransformComponent>(titleEntity);
    titlePos.x = centerX - 150.0F;
    titlePos.y = centerY - 120.0F;
    _registry->emplaceComponent<StaticTextTag>(titleEntity);
    _registry->emplaceComponent<ZIndex>(titleEntity, 9002);
    _disconnectTitleEntity = titleEntity;

    auto messageEntity = _registry->spawnEntity();
    auto mainFont = "main_font";
    _registry->emplaceComponent<Text>(
        messageEntity, mainFont, ::rtype::display::Color(220, 220, 220, 255),
        20, reasonMessage);
    auto& messagePos =
        _registry->emplaceComponent<rs::TransformComponent>(messageEntity);
    messagePos.x = centerX - 220.0F;
    messagePos.y = centerY - 50.0F;
    _registry->emplaceComponent<StaticTextTag>(messageEntity);
    LOG_INFO("[RtypeGameScene] Created disconnect message entity with text: "
             << reasonMessage);
    _registry->emplaceComponent<ZIndex>(messageEntity, 9002);
    _disconnectMessageEntity = messageEntity;

    Text buttonText(mainFont, ::rtype::display::Color::White(), 22,
                    "Return to Main Menu");
    rs::TransformComponent buttonPos{centerX - 125.0F, centerY + 80.0F};
    auto buttonSize = std::make_pair(250.0F, 50.0F);
    Rectangle buttonRect(buttonSize, ::rtype::display::Color(80, 120, 200, 255),
                         ::rtype::display::Color(100, 140, 220, 255));
    buttonRect.outlineColor = ::rtype::display::Color(120, 160, 240, 255);
    buttonRect.outlineThickness = 2.0F;

    std::function<void()> buttonCallback = [this]() {
        LOG_INFO("[RtypeGameScene] Returning to main menu after disconnect");
        cleanupDisconnectModal();
        _switchToScene(SceneManager::MAIN_MENU);
    };

    auto buttonEntity =
        EntityFactory::createButton(_registry, buttonText, buttonPos,
                                    buttonRect, _assetsManager, buttonCallback);

    _registry->emplaceComponent<ZIndex>(buttonEntity, 9003);
    _disconnectButtonEntity = buttonEntity;
}

void RtypeGameScene::cleanupDisconnectModal() {
    auto destroyEntity = [this](std::optional<ECS::Entity>& entity) {
        if (entity.has_value() && _registry->isAlive(*entity)) {
            _registry->killEntity(*entity);
        }
        entity.reset();
    };

    destroyEntity(_disconnectOverlayEntity);
    destroyEntity(_disconnectPanelEntity);
    destroyEntity(_disconnectTitleEntity);
    destroyEntity(_disconnectMessageEntity);
    destroyEntity(_disconnectButtonEntity);
    _isDisconnected = false;
}

std::string RtypeGameScene::getDisconnectMessage(
    network::DisconnectReason reason) const {
    switch (reason) {
        case network::DisconnectReason::Timeout:
            return "Server connection timed out.\nThe server may be down or "
                   "unreachable.";
        case network::DisconnectReason::MaxRetriesExceeded:
            return "Failed to connect after multiple attempts.\nPlease check "
                   "your connection.";
        case network::DisconnectReason::ProtocolError:
            return "A protocol error occurred.\nPlease restart the game.";
        case network::DisconnectReason::RemoteRequest:
            return "Server closed the connection.\nYou may have been kicked.";
        case network::DisconnectReason::LocalRequest:
            return "Disconnected from server.";
        case network::DisconnectReason::Banned:
            return "You have been banned from this server.";
        default:
            return "Connection lost for unknown reason.";
    }
}

void RtypeGameScene::toggleLowBandwidthMode() {
    _lowBandwidthMode = !_lowBandwidthMode;
    if (_networkClient) {
        _networkClient->setLowBandwidthMode(_lowBandwidthMode);
        LOG_INFO_CAT(::rtype::LogCategory::Network,
                     "[RtypeGameScene] Low bandwidth mode "
                         << (_lowBandwidthMode ? "ENABLED" : "DISABLED")
                         << " (F9 toggled)");
    }
    updateBandwidthIndicator();
}

void RtypeGameScene::updateBandwidthIndicator() {
    if (_bandwidthIndicatorEntity.has_value() &&
        _registry->isAlive(*_bandwidthIndicatorEntity)) {
        _registry->killEntity(*_bandwidthIndicatorEntity);
    }

    auto windowSize = _display->getWindowSize();
    float xPos = 10.0F;
    float yPos = static_cast<float>(windowSize.y) - 30.0F;

    auto indicator = _registry->spawnEntity();

    if (_lowBandwidthMode) {
        _registry->emplaceComponent<rs::TransformComponent>(indicator, xPos,
                                                            yPos);
        _registry->emplaceComponent<Text>(indicator, "title_font",
                                          ::rtype::display::Color(255, 180, 0),
                                          20, "LOW BANDWIDTH [F9]");
    } else {
        _registry->emplaceComponent<rs::TransformComponent>(indicator, xPos,
                                                            yPos + 6.0F);
        _registry->emplaceComponent<Text>(
            indicator, "title_font", ::rtype::display::Color(128, 128, 128), 14,
            "F9: Low Bandwidth");
    }

    _registry->emplaceComponent<ZIndex>(indicator,
                                        GraphicsConfig::ZINDEX_UI + 1);
    _registry->emplaceComponent<StaticTextTag>(indicator);
    _registry->emplaceComponent<GameTag>(indicator);

    _bandwidthIndicatorEntity = indicator;
}

void RtypeGameScene::showBandwidthNotification(std::uint32_t userId,
                                               bool enabled,
                                               std::uint8_t activeCount) {
    _lowBandwidthActiveCount = activeCount;

    if (_localPlayerId.has_value() && userId == *_localPlayerId) {
        return;
    }

    if (_bandwidthNotificationEntity.has_value() &&
        _registry->isAlive(*_bandwidthNotificationEntity)) {
        _registry->killEntity(*_bandwidthNotificationEntity);
    }

    auto windowSize = _display->getWindowSize();
    float xPos = static_cast<float>(windowSize.x) / 2.0F - 150.0F;
    float yPos = 60.0F;

    auto notification = _registry->spawnEntity();
    _registry->emplaceComponent<rs::TransformComponent>(notification, xPos,
                                                        yPos);

    std::string message = enabled ? "Player " + std::to_string(userId) +
                                        " enabled low bandwidth mode"
                                  : "Player " + std::to_string(userId) +
                                        " disabled low bandwidth mode";

    auto color = enabled ? ::rtype::display::Color(255, 200, 50)
                         : ::rtype::display::Color(100, 200, 100);

    _registry->emplaceComponent<Text>(notification, "title_font", color, 16,
                                      message);
    _registry->emplaceComponent<ZIndex>(notification,
                                        GraphicsConfig::ZINDEX_UI + 2);
    _registry->emplaceComponent<StaticTextTag>(notification);
    _registry->emplaceComponent<GameTag>(notification);

    _bandwidthNotificationEntity = notification;
    _bandwidthNotificationTimer = 3.0F;
}

void RtypeGameScene::setupBandwidthModeCallback() {
    if (_networkClient) {
        _networkClient->onBandwidthModeChanged(
            [this](std::uint32_t userId, bool lowBandwidth,
                   std::uint8_t activeCount) {
                showBandwidthNotification(userId, lowBandwidth, activeCount);
            });
    }
}

void RtypeGameScene::setupLevelAnnounceCallback() {
    if (_networkClient) {
        LOG_INFO_CAT(::rtype::LogCategory::UI,
                     "[RtypeGameScene] Setting up level announce callback");
        _networkClient->onLevelAnnounce(
            [this](const ::rtype::client::LevelAnnounceEvent& event) {
                LOG_INFO_CAT(
                    ::rtype::LogCategory::UI,
                    "[RtypeGameScene] Level announce callback triggered: "
                        << event.levelName << " background: "
                        << event.background << " music: " << event.levelMusic);
                showLevelAnnounce(event.levelName);
                if (_setBackground && !event.background.empty()) {
                    LOG_INFO_CAT(::rtype::LogCategory::UI,
                                 "[RtypeGameScene] Setting background to: "
                                     << event.background);
                    _setBackground(event.background);
                }
                if (_setLevelMusic && !event.levelMusic.empty()) {
                    LOG_INFO_CAT(::rtype::LogCategory::UI,
                                 "[RtypeGameScene] Setting level music to: "
                                     << event.levelMusic);
                    _setLevelMusic(event.levelMusic);
                }
            });
    }
}

void RtypeGameScene::showLevelAnnounce(const std::string& levelName) {
    LOG_INFO_CAT(
        ::rtype::LogCategory::UI,
        "[RtypeGameScene] showLevelAnnounce called with: " << levelName);

    if (_levelAnnounceBgEntity.has_value() &&
        _registry->isAlive(*_levelAnnounceBgEntity)) {
        _registry->killEntity(*_levelAnnounceBgEntity);
    }
    if (_levelAnnounceTextEntity.has_value() &&
        _registry->isAlive(*_levelAnnounceTextEntity)) {
        _registry->killEntity(*_levelAnnounceTextEntity);
    }

    auto windowSize = _display->getWindowSize();

    auto bg = EntityFactory::createRectangle(
        _registry,
        ::rtype::display::Vector2i(static_cast<int>(windowSize.x),
                                   static_cast<int>(windowSize.y)),
        ::rtype::display::Color(0, 0, 0, 175),
        ::rtype::display::Vector2f(0.0f, 0.0f));

    std::string displayTxt = "LEVEL: " + levelName;
    float estimatedWidth = static_cast<float>(displayTxt.length()) * 20.0f;
    float txtX = (static_cast<float>(windowSize.x) - estimatedWidth) / 2.0f;

    float centerX = static_cast<float>(windowSize.x) / 2.0f - 150.0f;

    auto txt = EntityFactory::createStaticText(
        _registry, _assetsManager, displayTxt, "title_font",
        ::rtype::display::Vector2f(
            centerX, static_cast<float>(windowSize.y) / 2.0f - 20.0f),
        40.f);

    _registry->emplaceComponent<ZIndex>(bg, GraphicsConfig::ZINDEX_UI + 5);
    _registry->emplaceComponent<ZIndex>(txt, GraphicsConfig::ZINDEX_UI + 6);

    _levelAnnounceBgEntity = bg;
    _levelAnnounceTextEntity = txt;
    _levelAnnounceTimer = 3.0f;

    if (!_isFirstLevelAnnounce) {
        VisualCueFactory::createConfetti(*_registry,
                                         static_cast<float>(windowSize.x),
                                         static_cast<float>(windowSize.y), 150);

        LOG_INFO_CAT(::rtype::LogCategory::UI,
                     "[RtypeGameScene] Level transition confetti triggered "
                     "(150 particles)");
    } else {
        LOG_INFO_CAT(
            ::rtype::LogCategory::UI,
            "[RtypeGameScene] First level announce - skipping confetti");
    }

    _isFirstLevelAnnounce = false;

    LOG_INFO_CAT(::rtype::LogCategory::UI,
                 "[RtypeGameScene] Level announce displayed for 3 seconds");
}

void RtypeGameScene::updateLevelAnnounce(float dt) {
    if (_levelAnnounceTimer > 0.0f) {
        _levelAnnounceTimer -= dt;
        if (_levelAnnounceTimer <= 0.0f) {
            _levelAnnounceTimer = 0.0f;
            LOG_INFO_CAT(::rtype::LogCategory::UI,
                         "[RtypeGameScene] Level announce destroyed after "
                         "timer elapsed");
            if (_levelAnnounceBgEntity.has_value() &&
                _registry->isAlive(*_levelAnnounceBgEntity)) {
                _registry->killEntity(*_levelAnnounceBgEntity);
                _levelAnnounceBgEntity.reset();
            }
            if (_levelAnnounceTextEntity.has_value() &&
                _registry->isAlive(*_levelAnnounceTextEntity)) {
                _registry->killEntity(*_levelAnnounceTextEntity);
                _levelAnnounceTextEntity.reset();
            }
        }
    }
}

}  // namespace rtype::games::rtype::client
