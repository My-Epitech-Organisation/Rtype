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

std::vector<ECS::Entity> RtypeGameScene::initialize() {
    LOG_DEBUG("[RtypeGameScene] Initialize called");
    std::vector<ECS::Entity> entities;

    auto bgEntities = EntityFactory::createBackground(this->_registry,
                                                      this->_assetsManager, "");
    entities.insert(entities.end(), bgEntities.begin(), bgEntities.end());
    LOG_DEBUG("[RtypeGameScene] Background created with " << bgEntities.size()
                                                          << " entities");

    if (_networkSystem) {
        LOG_DEBUG("[RtypeGameScene] Setting up entity factory");
        setupEntityFactory();
        LOG_DEBUG("[RtypeGameScene] Setting up local player callback");
        setupLocalPlayerCallback();
        LOG_DEBUG("[RtypeGameScene] Setting up health update callback");
        _networkSystem->onHealthUpdate(
            [this](const ::rtype::client::EntityHealthEvent& event) {
                handleHealthUpdate(event);
            });
        LOG_DEBUG("[RtypeGameScene] Network callbacks configured");
    }

    LOG_DEBUG("[RtypeGameScene] Setting up HUD");
    setupHud();
    setupDamageVignette();
    LOG_DEBUG("[RtypeGameScene] HUD setup complete");
    auto pauseEntities = RtypePauseMenu::createPauseMenu(
        _registry, _assetsManager, _switchToScene);
    entities.insert(entities.end(), pauseEntities.begin(), pauseEntities.end());
    LOG_DEBUG("[RtypeGameScene] Pause menu created with "
              << pauseEntities.size() << " entities");

    if (!_registry->hasSingleton<PauseState>()) {
        _registry->setSingleton<PauseState>(PauseState{false});
        LOG_DEBUG("[RtypeGameScene] PauseState singleton created");
    } else {
        _registry->getSingleton<PauseState>().isPaused = false;
        LOG_DEBUG("[RtypeGameScene] PauseState reset to unpaused");
    }

    LOG_DEBUG("[RtypeGameScene] Initialize completed, total entities: "
              << entities.size());
    return entities;
}

void RtypeGameScene::update() {
    const float dt = _uiClock.restart().asSeconds();

    updateDamageVignette(dt);

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
        if (inputMask != _lastInputMask) {
            _networkSystem->sendInput(inputMask);
            _lastInputMask = inputMask;
        }
    }
}

void RtypeGameScene::render(std::shared_ptr<sf::RenderWindow> window) {
    // R-Type specific rendering if needed
}

void RtypeGameScene::pollEvents(const sf::Event& event) {
    if (event.is<sf::Event::KeyReleased>() ||
        event.is<sf::Event::JoystickButtonReleased>()) {
        RtypeInputHandler::handleKeyReleasedEvent(event, _keybinds, _registry);
    }
}

std::uint8_t RtypeGameScene::getInputMask() const {
    return RtypeInputHandler::getInputMask(_keybinds);
}

void RtypeGameScene::setupEntityFactory() {
    LOG_DEBUG("[RtypeGameScene] Setting up entityFactory");
    _networkSystem->setEntityFactory(
        RtypeEntityFactory::createNetworkEntityFactory(_registry,
                                                       _assetsManager));
}

void RtypeGameScene::setupLocalPlayerCallback() {
    auto registry = this->_registry;
    _networkSystem->onLocalPlayerAssigned([this, registry](std::uint32_t userId,
                                                           ECS::Entity entity) {
        if (registry->isAlive(entity)) {
            registry->emplaceComponent<ControllableTag>(entity);
            LOG_DEBUG("[RtypeGameScene] Local player entity assigned");
        }
        _localPlayerEntity = entity;
        _localPlayerId = userId;
        LOG_DEBUG("[RtypeGameScene] Local player ID set to " << userId);
        if (registry->isAlive(entity) &&
            registry->hasComponent<rs::HealthComponent>(entity)) {
            const auto& health =
                registry->getComponent<rs::HealthComponent>(entity);
            LOG_DEBUG("[RtypeGameScene] Initial health: " << health.current
                                                          << "/" << health.max);
            updateLivesDisplay(health.current, health.max);
        }
    });
}

void RtypeGameScene::setupHud() {
    const float barWidth = 220.f;
    const float barHeight = 18.f;
    const sf::Vector2f barPos{20.f, 20.f};

    auto bg = _registry->spawnEntity();
    _registry->emplaceComponent<rs::Position>(bg, barPos.x, barPos.y);
    _registry->emplaceComponent<Rectangle>(
        bg, std::pair<float, float>{barWidth, barHeight},
        sf::Color(30, 35, 45, 220), sf::Color(30, 35, 45, 220));
    _registry->emplaceComponent<ZIndex>(bg, GraphicsConfig::ZINDEX_UI);
    _healthBarBgEntity = bg;

    auto fill = _registry->spawnEntity();
    _registry->emplaceComponent<rs::Position>(fill, barPos.x, barPos.y);
    _registry->emplaceComponent<Rectangle>(
        fill, std::pair<float, float>{barWidth, barHeight},
        sf::Color(90, 220, 140, 240), sf::Color(90, 220, 140, 240));
    _registry->emplaceComponent<ZIndex>(fill, GraphicsConfig::ZINDEX_UI + 1);
    _healthBarFillEntity = fill;

    auto hpText = EntityFactory::createStaticText(
        _registry, _assetsManager, "HP: --/--", "title_font",
        sf::Vector2f{barPos.x + barWidth + 16.f, barPos.y - 2.f}, 24.f);
    _registry->emplaceComponent<ZIndex>(hpText, GraphicsConfig::ZINDEX_UI + 2);
    _healthTextEntity = hpText;
    _livesTextEntity = hpText;
}

void RtypeGameScene::updateLivesDisplay(int current, int max) {
    LOG_DEBUG("[RtypeGameScene] updateLivesDisplay: " << current << "/" << max);
    _lastKnownLives = current;
    _lastKnownMaxLives = max;
    updateHealthBar(current, max);
}

void RtypeGameScene::handleHealthUpdate(
    const ::rtype::client::EntityHealthEvent& event) {
    LOG_DEBUG("[RtypeGameScene] Health update for local player: current="
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
    LOG_DEBUG("[RtypeGameScene] updateHealthBar called: " << current << "/"
                                                          << max);
    if (!_healthBarFillEntity.has_value() ||
        !_registry->isAlive(*_healthBarFillEntity)) {
        LOG_DEBUG("[RtypeGameScene] Health bar fill entity not valid");
        return;
    }

    const float barWidth = 220.f;
    const float ratio =
        max > 0
            ? std::max(0.f, std::min(1.f, current / static_cast<float>(max)))
            : 0.f;
    auto& rect = _registry->getComponent<Rectangle>(*_healthBarFillEntity);
    rect.size.first = barWidth * ratio;
    LOG_DEBUG("[RtypeGameScene] Health bar width set to: " << rect.size.first);

    if (_healthTextEntity.has_value() &&
        _registry->isAlive(*_healthTextEntity) &&
        _registry->hasComponent<Text>(*_healthTextEntity)) {
        auto& text = _registry->getComponent<Text>(*_healthTextEntity);
        text.textContent =
            "HP: " + std::to_string(current) + "/" + std::to_string(max);
        LOG_DEBUG("[RtypeGameScene] HP text updated to: " << text.textContent);
    } else {
        LOG_DEBUG("[RtypeGameScene] Health text entity not valid for update");
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
        _registry->emplaceComponent<rs::Position>(top, 0.f, inset);
        _registry->emplaceComponent<Rectangle>(
            top, std::pair<float, float>{screenWidth, thickness}, vignetteColor,
            vignetteColor);
        _registry->emplaceComponent<ZIndex>(
            top, GraphicsConfig::ZINDEX_UI + 100 + layer);
        _registry->emplaceComponent<ScreenSpaceTag>(top);
        _vignetteEntities.push_back(top);

        auto bottom = _registry->spawnEntity();
        _registry->emplaceComponent<rs::Position>(
            bottom, 0.f, screenHeight - inset - thickness);
        _registry->emplaceComponent<Rectangle>(
            bottom, std::pair<float, float>{screenWidth, thickness},
            vignetteColor, vignetteColor);
        _registry->emplaceComponent<ZIndex>(
            bottom, GraphicsConfig::ZINDEX_UI + 100 + layer);
        _registry->emplaceComponent<ScreenSpaceTag>(bottom);
        _vignetteEntities.push_back(bottom);

        auto left = _registry->spawnEntity();
        _registry->emplaceComponent<rs::Position>(left, inset, 0.f);
        _registry->emplaceComponent<Rectangle>(
            left, std::pair<float, float>{thickness, screenHeight},
            vignetteColor, vignetteColor);
        _registry->emplaceComponent<ZIndex>(
            left, GraphicsConfig::ZINDEX_UI + 100 + layer);
        _registry->emplaceComponent<ScreenSpaceTag>(left);
        _vignetteEntities.push_back(left);

        auto right = _registry->spawnEntity();
        _registry->emplaceComponent<rs::Position>(
            right, screenWidth - inset - thickness, 0.f);
        _registry->emplaceComponent<Rectangle>(
            right, std::pair<float, float>{thickness, screenHeight},
            vignetteColor, vignetteColor);
        _registry->emplaceComponent<ZIndex>(
            right, GraphicsConfig::ZINDEX_UI + 100 + layer);
        _registry->emplaceComponent<ScreenSpaceTag>(right);
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
                _registry->hasComponent<rs::Position>(ent) &&
                _registry->hasComponent<Rectangle>(ent)) {
                auto& pos = _registry->getComponent<rs::Position>(ent);
                pos.x = viewLeft;
                pos.y = viewTop + inset;
                auto& rect = _registry->getComponent<Rectangle>(ent);
                rect.size = {screenWidth, thickness};
            }
        }

        if (entityIndex < static_cast<int>(_vignetteEntities.size())) {
            auto ent = _vignetteEntities[entityIndex++];
            if (_registry->isAlive(ent) &&
                _registry->hasComponent<rs::Position>(ent) &&
                _registry->hasComponent<Rectangle>(ent)) {
                auto& pos = _registry->getComponent<rs::Position>(ent);
                pos.x = viewLeft;
                pos.y = viewTop + screenHeight - inset - thickness;
                auto& rect = _registry->getComponent<Rectangle>(ent);
                rect.size = {screenWidth, thickness};
            }
        }

        if (entityIndex < static_cast<int>(_vignetteEntities.size())) {
            auto ent = _vignetteEntities[entityIndex++];
            if (_registry->isAlive(ent) &&
                _registry->hasComponent<rs::Position>(ent) &&
                _registry->hasComponent<Rectangle>(ent)) {
                auto& pos = _registry->getComponent<rs::Position>(ent);
                pos.x = viewLeft + inset;
                pos.y = viewTop;
                auto& rect = _registry->getComponent<Rectangle>(ent);
                rect.size = {thickness, screenHeight};
            }
        }

        if (entityIndex < static_cast<int>(_vignetteEntities.size())) {
            auto ent = _vignetteEntities[entityIndex++];
            if (_registry->isAlive(ent) &&
                _registry->hasComponent<rs::Position>(ent) &&
                _registry->hasComponent<Rectangle>(ent)) {
                auto& pos = _registry->getComponent<rs::Position>(ent);
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
    LOG_DEBUG(
        "[RtypeGameScene] spawnDamagePopup called with damage=" << damage);

    std::optional<ECS::Entity> playerEntity = _localPlayerEntity;
    if (!playerEntity.has_value() && _networkSystem) {
        playerEntity = _networkSystem->getLocalPlayerEntity();
    }

    if (!playerEntity.has_value()) {
        LOG_DEBUG(
            "[RtypeGameScene] No player entity available for damage popup");
        return;
    }

    if (!_registry->isAlive(*playerEntity)) {
        LOG_DEBUG("[RtypeGameScene] Player entity not alive for damage popup");
        return;
    }

    if (!_registry->hasComponent<rs::Position>(*playerEntity)) {
        LOG_DEBUG(
            "[RtypeGameScene] Player entity has no Position for damage popup");
        return;
    }

    const auto& pos = _registry->getComponent<rs::Position>(*playerEntity);
    LOG_DEBUG("[RtypeGameScene] Player position for popup: (" << pos.x << ", "
                                                              << pos.y << ")");

    if (!_assetsManager || !_assetsManager->fontManager) {
        LOG_DEBUG("[RtypeGameScene] No assets manager for damage popup");
        return;
    }

    try {
        const sf::Font& font = _assetsManager->fontManager->get("title_font");
        VisualCueFactory::createDamagePopup(
            *_registry, sf::Vector2f(pos.x + 20.f, pos.y - 10.f), damage, font,
            sf::Color(255, 60, 60));
        LOG_DEBUG("[RtypeGameScene] Damage popup created successfully");
    } catch (const std::exception& e) {
        LOG_DEBUG(
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

}  // namespace rtype::games::rtype::client
