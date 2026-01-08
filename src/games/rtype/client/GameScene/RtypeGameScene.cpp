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

#include <SFML/System/Clock.hpp>

#include "AllComponents.hpp"
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
    std::shared_ptr<sf::RenderWindow> window,
    std::shared_ptr<KeyboardActions> keybinds,
    std::function<void(const SceneManager::Scene&)> switchToScene,
    std::shared_ptr<::rtype::client::NetworkClient> networkClient,
    std::shared_ptr<::rtype::client::ClientNetworkSystem> networkSystem,
    std::shared_ptr<AudioLib> audioLib)
    : AGameScene(std::move(registry), std::move(assetsManager),
                 std::move(window), std::move(keybinds),
                 std::move(switchToScene), std::move(networkClient),
                 std::move(networkSystem)) {
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
                                event.finalScore});
                    } else {
                        registry
                            ->getSingleton<
                                ::rtype::games::rtype::client::GameOverState>()
                            .finalScore = event.finalScore;
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

    auto bgEntities = EntityFactory::createBackground(this->_registry,
                                                      this->_assetsManager, "");
    entities.insert(entities.end(), bgEntities.begin(), bgEntities.end());
    LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                  "[RtypeGameScene] Background created with "
                      << bgEntities.size() << " entities");
    if (_networkSystem) {
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
    setupDamageVignette();
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
    const float dt = _uiClock.restart().asSeconds();

    updateDamageVignette(dt);
    updatePingDisplay();

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

    if (_networkSystem) {
        _networkSystem->update();
    }

    bool isPaused = false;
    if (_registry->hasSingleton<PauseState>()) {
        isPaused = _registry->getSingleton<PauseState>().isPaused;
    }

    if (isPaused) {
        return;
    }

    std::uint8_t inputMask = getInputMask();

    if (_networkSystem && _networkSystem->isConnected()) {
        bool shouldSend = false;

        constexpr std::uint8_t kMovementMask =
            ::rtype::network::InputMask::kUp |
            ::rtype::network::InputMask::kDown |
            ::rtype::network::InputMask::kLeft |
            ::rtype::network::InputMask::kRight;

        std::uint8_t currentMovement = inputMask & kMovementMask;
        std::uint8_t lastMovement = _lastInputMask & kMovementMask;

        if (currentMovement != lastMovement) {
            shouldSend = true;
        }

        bool isShootingNow =
            (inputMask & ::rtype::network::InputMask::kShoot) != 0;
        bool wasShootingLast =
            (_lastInputMask & ::rtype::network::InputMask::kShoot) != 0;

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

        if (shouldSend) {
            _networkSystem->sendInput(inputMask);
        }
        _lastInputMask = inputMask;
    }
}

void RtypeGameScene::render(std::shared_ptr<sf::RenderWindow> window) {
    // R-Type specific rendering if needed
}

void RtypeGameScene::pollEvents(const sf::Event& event) {
    if (event.is<sf::Event::KeyPressed>() ||
        event.is<sf::Event::KeyReleased>()) {
        RtypeInputHandler::updateKeyState(event);
    }
    if (event.is<sf::Event::KeyReleased>() ||
        event.is<sf::Event::JoystickButtonReleased>()) {
        RtypeInputHandler::handleKeyReleasedEvent(event, _keybinds, _registry);
    }
}

std::uint8_t RtypeGameScene::getInputMask() const {
    return RtypeInputHandler::getInputMask(_keybinds);
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
                LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                              "[RtypeGameScene] Local player entity assigned");
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
    const sf::Vector2f barPos{20.f, 20.f};

    auto bg = _registry->spawnEntity();
    _registry->emplaceComponent<rs::TransformComponent>(bg, barPos.x, barPos.y);
    _registry->emplaceComponent<Rectangle>(
        bg, std::pair<float, float>{barWidth, barHeight},
        sf::Color(30, 35, 45, 220), sf::Color(30, 35, 45, 220));
    _registry->emplaceComponent<ZIndex>(bg, GraphicsConfig::ZINDEX_UI);
    _registry->emplaceComponent<HudTag>(bg);
    _registry->emplaceComponent<GameTag>(bg);
    _healthBarBgEntity = bg;

    auto fill = _registry->spawnEntity();
    _registry->emplaceComponent<rs::TransformComponent>(fill, barPos.x,
                                                        barPos.y);
    _registry->emplaceComponent<Rectangle>(
        fill, std::pair<float, float>{barWidth, barHeight},
        sf::Color(90, 220, 140, 240), sf::Color(90, 220, 140, 240));
    _registry->emplaceComponent<ZIndex>(fill, GraphicsConfig::ZINDEX_UI + 1);
    _registry->emplaceComponent<HudTag>(fill);
    _registry->emplaceComponent<GameTag>(fill);
    _healthBarFillEntity = fill;

    auto hpText = EntityFactory::createStaticText(
        _registry, _assetsManager, "HP: --/--", "title_font",
        sf::Vector2f{barPos.x + barWidth + strlen("HP: --/--") / 2 * 24,
                     barPos.y + barHeight / 2},
        24.f);
    _registry->emplaceComponent<ZIndex>(hpText, GraphicsConfig::ZINDEX_UI + 2);
    _registry->emplaceComponent<HudTag>(hpText);
    _registry->emplaceComponent<GameTag>(hpText);
    _healthTextEntity = hpText;
    _livesTextEntity = hpText;

    auto pingText = EntityFactory::createStaticText(
        _registry, _assetsManager, "Ping: 0ms", "title_font",
        sf::Vector2f{1800.f, 20.f}, 20.f);
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
            text.color = sf::Color(90, 220, 140);
        } else if (latency < 100) {
            text.color = sf::Color(220, 220, 90);
        } else if (latency < 200) {
            text.color = sf::Color(255, 165, 0);
        } else {
            text.color = sf::Color(220, 90, 90);
        }
    }
}

void RtypeGameScene::setupDamageVignette() {
    const float screenWidth = static_cast<float>(_window->getSize().x);
    const float screenHeight = static_cast<float>(_window->getSize().y);

    _vignetteEntities.clear();

    for (int layer = 0; layer < kVignetteLayers; ++layer) {
        float layerRatio =
            static_cast<float>(layer) / static_cast<float>(kVignetteLayers - 1);
        float inset = layerRatio * 80.f;
        float thickness = 50.f + layerRatio * 40.f;

        const sf::Color vignetteColor(255, 0, 0, 0);

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

    const sf::Vector2u currentSize = _window->getSize();
    const sf::View defaultView = _window->getDefaultView();
    const float viewLeft =
        defaultView.getCenter().x - defaultView.getSize().x / 2.0f;
    const float viewTop =
        defaultView.getCenter().y - defaultView.getSize().y / 2.0f;
    const float screenWidth = defaultView.getSize().x;
    const float screenHeight = defaultView.getSize().y;

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
            rect.currentColor =
                sf::Color(255, 0, 0, static_cast<uint8_t>(layerAlpha));
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
            rect.currentColor = sf::Color(255, 0, 0, 0);
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
            rect.currentColor =
                sf::Color(255, 0, 0, static_cast<uint8_t>(layerAlpha));
            rect.mainColor = rect.currentColor;
        }
        ++layerIndex;
    }

    if (_healthBarFillEntity.has_value() &&
        _registry->isAlive(*_healthBarFillEntity)) {
        auto& rect = _registry->getComponent<Rectangle>(*_healthBarFillEntity);
        rect.currentColor = sf::Color(255, 80, 80, 240);
    }

    if (_healthTextEntity.has_value() &&
        _registry->isAlive(*_healthTextEntity) &&
        _registry->hasComponent<Text>(*_healthTextEntity)) {
        auto& text = _registry->getComponent<Text>(*_healthTextEntity);
        text.color = sf::Color(255, 100, 100);
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
        auto font = _assetsManager->fontManager->get("title_font");
        VisualCueFactory::createDamagePopup(
            *_registry, sf::Vector2f(pos.x + 20.f, pos.y - 10.f), damage, font,
            sf::Color(255, 60, 60));
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
        rect.currentColor = sf::Color(90, 220, 140, 240);
    }

    if (_healthTextEntity.has_value() &&
        _registry->isAlive(*_healthTextEntity) &&
        _registry->hasComponent<Text>(*_healthTextEntity)) {
        auto& text = _registry->getComponent<Text>(*_healthTextEntity);
        text.color = sf::Color::White;
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

    const sf::Vector2u windowSize = _window->getSize();
    const float centerX = windowSize.x / 2.0F;
    const float centerY = windowSize.y / 2.0F;

    auto overlayEntity = _registry->spawnEntity();
    auto overlaySize = std::make_pair(static_cast<float>(windowSize.x),
                                      static_cast<float>(windowSize.y));
    _registry->emplaceComponent<Rectangle>(overlayEntity, overlaySize,
                                           sf::Color(0, 0, 0, 180),
                                           sf::Color(0, 0, 0, 180));
    auto& overlayPos =
        _registry->emplaceComponent<rs::TransformComponent>(overlayEntity);
    overlayPos.x = 0;
    overlayPos.y = 0;
    _registry->emplaceComponent<ZIndex>(overlayEntity, 9000);
    _disconnectOverlayEntity = overlayEntity;

    auto panelEntity = _registry->spawnEntity();
    auto panelSize = std::make_pair(500.0F, 300.0F);
    _registry->emplaceComponent<Rectangle>(panelEntity, panelSize,
                                           sf::Color(40, 40, 60, 255),
                                           sf::Color(40, 40, 60, 255));
    auto& panelRect = _registry->getComponent<Rectangle>(panelEntity);
    panelRect.outlineColor = sf::Color(120, 120, 150, 255);
    panelRect.outlineThickness = 3.0F;
    auto& panelPos =
        _registry->emplaceComponent<rs::TransformComponent>(panelEntity);
    panelPos.x = centerX - 250.0F;
    panelPos.y = centerY - 150.0F;
    _registry->emplaceComponent<ZIndex>(panelEntity, 9001);
    _disconnectPanelEntity = panelEntity;

    auto titleEntity = _registry->spawnEntity();
    auto titleFont = _assetsManager->fontManager->get("title_font");
    _registry->emplaceComponent<Text>(titleEntity, titleFont,
                                      sf::Color(255, 100, 100), 36,
                                      "Connection Lost");
    auto& titlePos =
        _registry->emplaceComponent<rs::TransformComponent>(titleEntity);
    titlePos.x = centerX - 150.0F;
    titlePos.y = centerY - 120.0F;
    _registry->emplaceComponent<StaticTextTag>(titleEntity);
    _registry->emplaceComponent<ZIndex>(titleEntity, 9002);
    _disconnectTitleEntity = titleEntity;

    auto messageEntity = _registry->spawnEntity();
    auto mainFont = _assetsManager->fontManager->get("main_font");
    _registry->emplaceComponent<Text>(
        messageEntity, mainFont, sf::Color(220, 220, 220), 20, reasonMessage);
    auto& messagePos =
        _registry->emplaceComponent<rs::TransformComponent>(messageEntity);
    messagePos.x = centerX - 220.0F;
    messagePos.y = centerY - 50.0F;
    _registry->emplaceComponent<StaticTextTag>(messageEntity);
    LOG_INFO("[RtypeGameScene] Created disconnect message entity with text: "
             << reasonMessage);
    _registry->emplaceComponent<ZIndex>(messageEntity, 9002);
    _disconnectMessageEntity = messageEntity;

    Text buttonText(mainFont, sf::Color::White, 22, "Return to Main Menu");
    rs::TransformComponent buttonPos{centerX - 125.0F, centerY + 80.0F};
    auto buttonSize = std::make_pair(250.0F, 50.0F);
    Rectangle buttonRect(buttonSize, sf::Color(80, 120, 200, 255),
                         sf::Color(100, 140, 220, 255));
    buttonRect.outlineColor = sf::Color(120, 160, 240, 255);
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

}  // namespace rtype::games::rtype::client
