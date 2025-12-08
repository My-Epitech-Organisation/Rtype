/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** GameScene.cpp
*/

#include "GameScene.hpp"

#include <iostream>
#include <memory>
#include <utility>

#include "Components/HiddenComponent.hpp"
#include "Components/ImageComponent.hpp"
#include "Components/PositionComponent.hpp"
#include "Components/SizeComponent.hpp"
#include "Components/TagComponent.hpp"
#include "Components/TextureRectComponent.hpp"
#include "Components/VelocityComponent.hpp"
#include "Components/ZIndexComponent.hpp"
#include "EntityFactory/EntityFactory.hpp"
#include "Graphic.hpp"
#include "SceneManager/SceneException.hpp"
#include "protocol/Payloads.hpp"

void GameScene::_handleKeyReleasedEvent(const sf::Event& event) {
    auto eventKeyRelease = event.getIf<sf::Event::KeyReleased>();
    auto keyPause = this->_keybinds->getKeyBinding(GameAction::PAUSE);
    if (!eventKeyRelease || !keyPause.has_value()) return;
    if (eventKeyRelease->code == *keyPause) {
        this->_registry
            ->view<rtype::games::rtype::client::HiddenComponent,
                   rtype::games::rtype::client::PauseMenuTag>()
            .each([](auto, rtype::games::rtype::client::HiddenComponent& hidden,
                     auto) { hidden.isHidden = !hidden.isHidden; });
    }
}

void GameScene::update() {
    // Poll network to receive entity updates from server
    if (_networkSystem) {
        _networkSystem->update();
    }

    // Build input mask from current key states
    std::uint8_t inputMask = rtype::network::InputMask::kNone;

    auto keyMoveUp = _keybinds->getKeyBinding(GameAction::MOVE_UP);
    if (keyMoveUp.has_value() && sf::Keyboard::isKeyPressed(*keyMoveUp)) {
        inputMask |= rtype::network::InputMask::kUp;
    }

    auto keyMoveDown = _keybinds->getKeyBinding(GameAction::MOVE_DOWN);
    if (keyMoveDown.has_value() && sf::Keyboard::isKeyPressed(*keyMoveDown)) {
        inputMask |= rtype::network::InputMask::kDown;
    }

    auto keyMoveLeft = _keybinds->getKeyBinding(GameAction::MOVE_LEFT);
    if (keyMoveLeft.has_value() && sf::Keyboard::isKeyPressed(*keyMoveLeft)) {
        inputMask |= rtype::network::InputMask::kLeft;
    }

    auto keyMoveRight = _keybinds->getKeyBinding(GameAction::MOVE_RIGHT);
    if (keyMoveRight.has_value() && sf::Keyboard::isKeyPressed(*keyMoveRight)) {
        inputMask |= rtype::network::InputMask::kRight;
    }

    // TODO: Add shoot key binding check
    // if (shootKeyPressed) inputMask |= rtype::network::InputMask::kShoot;

    // Send inputs to server only if changed (to avoid flooding)
    if (_networkSystem && _networkSystem->isConnected()) {
        if (inputMask != _lastInputMask) {
            _networkSystem->sendInput(inputMask);
            _lastInputMask = inputMask;
        }
    }
}

void GameScene::render(std::shared_ptr<sf::RenderWindow> window) {}

void GameScene::pollEvents(const sf::Event& e) {
    if (e.is<sf::Event::KeyReleased>()) {
        this->_handleKeyReleasedEvent(e);
    }
}

GameScene::GameScene(
    std::shared_ptr<ECS::Registry> ecs,
    std::shared_ptr<AssetManager> textureManager,
    std::shared_ptr<sf::RenderWindow> window,
    std::shared_ptr<KeyboardActions> keybinds,
    std::function<void(const SceneManager::Scene&)> switchToScene,
    std::shared_ptr<rtype::client::NetworkClient> networkClient,
    std::shared_ptr<rtype::client::ClientNetworkSystem> networkSystem)
    : AScene(ecs, textureManager, window),
      _keybinds(keybinds),
      _networkClient(std::move(networkClient)),
      _networkSystem(std::move(networkSystem)) {
    // Create background
    this->_listEntity = (EntityFactory::createBackground(
        this->_registry, this->_assetsManager, ""));

    // Configure network entity factory to create entities with graphics
    if (_networkSystem) {
        std::cout << "[GameScene] Setting up entityFactory" << std::endl;
        auto assetsManager = this->_assetsManager;
        auto registry = this->_registry;
        _networkSystem->setEntityFactory(
            [assetsManager,
             registry](ECS::Registry& reg,
                       const rtype::client::EntitySpawnEvent& event)
                -> ECS::Entity {
                std::cout << "[GameScene::entityFactory] Creating entity type="
                          << static_cast<int>(event.type)
                          << " pos=(" << event.x << ", " << event.y << ")"
                          << std::endl;

                auto entity = reg.spawnEntity();

                // Add Position component at spawn location
                reg.emplaceComponent<rtype::games::rtype::shared::Position>(
                    entity, event.x, event.y);

                // Add Velocity component (initial velocity is 0)
                reg.emplaceComponent<
                    rtype::games::rtype::shared::VelocityComponent>(
                    entity, 0.f, 0.f);

                // Add graphics based on entity type
                switch (event.type) {
                    case rtype::network::EntityType::Player:
                        std::cout << "[GameScene::entityFactory] Adding Player components" << std::endl;
                        reg.emplaceComponent<rtype::games::rtype::client::Image>(
                            entity,
                            assetsManager->textureManager->get("player_vessel"));
                        reg.emplaceComponent<
                            rtype::games::rtype::client::TextureRect>(
                            entity, std::pair<int, int>({0, 0}),
                            std::pair<int, int>({33, 17}));
                        reg.emplaceComponent<rtype::games::rtype::client::Size>(
                            entity, 4, 4);
                        reg.emplaceComponent<
                            rtype::games::rtype::client::PlayerTag>(entity);
                        reg.emplaceComponent<
                            rtype::games::rtype::client::ZIndex>(entity, 0);
                        break;

                    case rtype::network::EntityType::Bydos:
                        // TODO: Add Bydos enemy sprite when available
                        reg.emplaceComponent<rtype::games::rtype::client::Image>(
                            entity,
                            assetsManager->textureManager->get("player_vessel"));
                        reg.emplaceComponent<
                            rtype::games::rtype::client::TextureRect>(
                            entity, std::pair<int, int>({0, 0}),
                            std::pair<int, int>({33, 17}));
                        reg.emplaceComponent<rtype::games::rtype::client::Size>(
                            entity, 3, 3);
                        reg.emplaceComponent<
                            rtype::games::rtype::client::ZIndex>(entity, 0);
                        break;

                    case rtype::network::EntityType::Missile:
                        // TODO: Add Missile sprite when available
                        reg.emplaceComponent<rtype::games::rtype::client::Size>(
                            entity, 1, 1);
                        reg.emplaceComponent<
                            rtype::games::rtype::client::ZIndex>(entity, 1);
                        break;
                }

                return entity;
            });

        // Register callback to mark local player as controllable
        _networkSystem->onLocalPlayerAssigned(
            [this](std::uint32_t /*userId*/, ECS::Entity entity) {
                if (_registry->isAlive(entity)) {
                    _registry->emplaceComponent<
                        rtype::games::rtype::client::ControllableTag>(entity);
                    std::cout << "[GameScene] Local player entity assigned"
                              << std::endl;
                }
            });
    }

    // Create pause menu
    auto sectionX = (Graphic::WINDOW_WIDTH - SizeXPauseMenu) / 2;
    auto sectionY = (Graphic::WINDOW_HEIGHT - SizeYPauseMenu) / 2;
    auto pauseEntities = EntityFactory::createSection(
        this->_registry, this->_assetsManager, "", sectionX, sectionY,
        SizeXPauseMenu, SizeYPauseMenu);
    auto titleEntity = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, PauseMenuTitle, "title_font",
        (sectionX + SizeXPauseMenu / 2) -
            ((PauseMenuTitle.length() - 2) * (SizeFontPauseMenu / 2)),
        sectionY, SizeFontPauseMenu);
    auto& titleText =
        this->_registry->getComponent<rtype::games::rtype::client::Text>(
            titleEntity);
    sf::FloatRect bounds = titleText.text.getLocalBounds();
    float centeredX = sectionX + (SizeXPauseMenu - bounds.size.x) / 2;
    auto& titlePos =
        this->_registry->getComponent<rtype::games::rtype::shared::Position>(
            titleEntity);
    titlePos.x = centeredX;
    pauseEntities.push_back(titleEntity);

    pauseEntities.push_back(EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("title_font"),
            sf::Color::White, 30, "Menu"),
        rtype::games::rtype::shared::Position(
            sectionX + ((SizeXPauseMenu / 2) - (150 / 2)),
            sectionY + SizeYPauseMenu - 75),
        rtype::games::rtype::client::Rectangle({150, 55}, sf::Color::Blue,
                                               sf::Color::Red),
        std::function<void()>([switchToScene]() {
            try {
                switchToScene(SceneManager::MAIN_MENU);
            } catch (SceneNotFound& e) {
                std::cerr << "Error switching to Main Menu: " << e.what()
                          << std::endl;
            }
        })));

    for (auto& entt : pauseEntities) {
        this->_registry
            ->emplaceComponent<rtype::games::rtype::client::HiddenComponent>(
                entt, true);
        this->_registry
            ->emplaceComponent<rtype::games::rtype::client::PauseMenuTag>(entt);
    }
    this->_listEntity.insert(this->_listEntity.end(), pauseEntities.begin(),
                             pauseEntities.end());
}
