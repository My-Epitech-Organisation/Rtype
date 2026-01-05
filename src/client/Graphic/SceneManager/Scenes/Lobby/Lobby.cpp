/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** Lobby.cpp
*/

#include "Lobby.hpp"

#include <algorithm>
#include <ranges>
#include <string>
#include <utility>

#include "Components/HiddenComponent.hpp"
#include "Components/PlayerIdComponent.hpp"
#include "Components/ZIndexComponent.hpp"
#include "EntityFactory/EntityFactory.hpp"
#include "GameScene/RtypeEntityFactory.hpp"
#include "SceneException.hpp"

constexpr float kBaseX = 210.0f;
constexpr float kBaseY = 200.0f;
constexpr float kBaseW = 1500.0f;
constexpr float kBaseH = 650.0f;

constexpr float kColCount = 4.0f;
constexpr float kColRawWidth = kBaseW / kColCount;
constexpr float kGapX = 15.0f;
constexpr float kMarginLeft = 45.0f;

constexpr float kStepX = kColRawWidth - kGapX;

constexpr float kStartPosX = kBaseX + kMarginLeft;

constexpr float kBoxWidth = kColRawWidth - kMarginLeft;
constexpr float kBoxTopY = kBaseY + (kBaseH / 5.0f);
constexpr float kBoxHeight = kBaseH / 1.5f;

constexpr float kBoxCenterY = kBoxTopY + (kBoxHeight / 2.0f);
constexpr float kBoxHalfW = kBoxWidth / 2.0f;

void Lobby::update(float dt) {
    if (!this->_registry) {
        LOG_ERROR("[Lobby] Registry is null in update!");
        return;
    }

    if (!this->_networkClient->isConnected()) {
        this->_switchToScene(SceneManager::Scene::MAIN_MENU);
    }

    if (!_pendingPlayerRemovals.empty()) {
        for (auto playerId : _pendingPlayerRemovals) {
            if (_listUser.find(playerId) != _listUser.end()) {
                LOG_INFO("[Lobby] Player "
                         << playerId
                         << " disconnected - removing from lobby UI");

                _disconnectedPlayers.insert(playerId);
                _playersToPosition.erase(playerId);
                _playerReadyStates.erase(playerId);
                _playerReadyIndicators.erase(playerId);

                _removePlayerInfoMenu(playerId);

                if (_nbrUser > 0) {
                    _nbrUser--;
                }
            }
        }
        _pendingPlayerRemovals.clear();
    }

    try {
        this->_registry->view<rtype::games::rtype::shared::PlayerIdComponent>()
            .each([this](auto playerEntt, auto& id) {
                uint32_t playerId = id.playerId;
                if (_disconnectedPlayers.count(playerId) > 0) {
                    return;
                }
                if (_playerIndexMap.find(playerId) == _playerIndexMap.end()) {
                    this->_nbrUser++;
                    LOG_INFO(
                        "[Lobby] Detected new player entity with playerId: "
                        << playerId);
                    this->_createPlayerInfoMenu(playerId, this->_nbrUser);
                }
            });
    } catch (const std::exception& e) {
        LOG_ERROR("[Lobby] Exception checking for new players: " << e.what());
    }

    for (auto it = _playersToPosition.begin();
         it != _playersToPosition.end();) {
        uint32_t playerId = *it;
        bool positioned = false;

        if (_playerIndexMap.find(playerId) == _playerIndexMap.end()) {
            ++it;
            continue;
        }
        int playerIndex = _playerIndexMap[playerId];
        float mySectionX = kStartPosX + ((playerIndex - 1) * kStepX);
        float myCenterX = mySectionX + kBoxHalfW;

        try {
            this->_registry
                ->view<rtype::games::rtype::shared::PlayerIdComponent,
                       rtype::games::rtype::client::ZIndex>()
                .each([this, playerId, myCenterX, &positioned](
                          auto playerEntt, auto id, auto& Zindex) {
                    if (playerId != id.playerId) return;

                    LOG_INFO("[Lobby] Positioning player "
                             << playerId << " sprite at x=" << myCenterX
                             << " y=" << kBoxCenterY);

                    if (this->_registry->hasComponent<
                            rtype::games::rtype::shared::Position>(
                            playerEntt)) {
                        auto& pos = this->_registry->getComponent<
                            rtype::games::rtype::shared::Position>(playerEntt);
                        pos.x = myCenterX;
                        pos.y = kBoxCenterY;
                    } else {
                        this->_registry->emplaceComponent<
                            rtype::games::rtype::shared::Position>(
                            playerEntt, myCenterX, kBoxCenterY);
                    }
                    Zindex.depth = 4;
                    positioned = true;
                });
        } catch (const std::exception& e) {
            LOG_ERROR("[Lobby] Exception positioning player "
                      << playerId << ": " << e.what());
        }

        if (positioned) {
            it = _playersToPosition.erase(it);
        } else {
            ++it;
        }
    }

    if (_countdownActive) {
        _countdownTimer -= dt;

        if (_registry->hasComponent<rtype::games::rtype::client::Text>(
                _countdownTextEntity)) {
            auto& text =
                _registry->getComponent<rtype::games::rtype::client::Text>(
                    _countdownTextEntity);
            if (_countdownTimer > 0) {
                int displayTime = static_cast<int>(std::ceil(_countdownTimer));
                text.textContent =
                    "Game starting in: " + std::to_string(displayTime);
                text.text.setString(text.textContent);
            } else {
                text.textContent = "GO!";
                text.text.setString(text.textContent);
            }
        }

        if (_countdownTimer <= -0.5f) {
            try {
                this->_switchToScene(SceneManager::Scene::IN_GAME);
            } catch (SceneNotFound& e) {
                LOG_ERROR(std::string("Error switching to Game Scene: ") +
                          std::string(e.what()));
            }
        }
    }
}

void Lobby::render(std::shared_ptr<sf::RenderWindow> window) {}

void Lobby::pollEvents(const sf::Event& e) {}

void Lobby::_createPlayerInfoMenu(uint32_t userId, int index) {
    LOG_INFO("[Lobby] Creating player info menu for userId: "
             << userId << " at index: " << index);

    _playerIndexMap[userId] = index;

    float sectionX = kStartPosX + ((index - 1) * kStepX);
    float myCenterX = sectionX + kBoxHalfW;

    auto playerSection = EntityFactory::createSection(
        this->_registry, this->_assetsManager, "",
        sf::FloatRect({sectionX, kBoxTopY}, {kBoxWidth, kBoxHeight}), 1);
    this->_listUser.insert({userId, playerSection});
    this->_listEntity.insert(this->_listEntity.end(), playerSection.begin(),
                             playerSection.end());
    LOG_INFO("[Lobby] Created section with " << playerSection.size()
                                             << " entities");

    auto playerLabel = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager,
        "Player " + std::to_string(userId), "main_font",
        sf::Vector2f(myCenterX - 60, kBoxTopY + 20), 36);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        playerLabel, 3);
    if (this->_registry->hasComponent<rtype::games::rtype::client::Text>(
            playerLabel)) {
        auto& text =
            this->_registry->getComponent<rtype::games::rtype::client::Text>(
                playerLabel);
        text.color = sf::Color::Yellow;
    }
    this->_listUser[userId].push_back(playerLabel);
    this->_listEntity.push_back(playerLabel);
    LOG_INFO("[Lobby] Created player label entity");

    auto readyIndicator = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "WAITING...", "main_font",
        sf::Vector2f(myCenterX - 80, kBoxTopY + 70), 28);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        readyIndicator, 3);
    if (this->_registry->hasComponent<rtype::games::rtype::client::Text>(
            readyIndicator)) {
        auto& text =
            this->_registry->getComponent<rtype::games::rtype::client::Text>(
                readyIndicator);
        text.color = sf::Color::Yellow;
    }
    _playerReadyIndicators[userId] = readyIndicator;
    this->_listUser[userId].push_back(readyIndicator);
    this->_listEntity.push_back(readyIndicator);
    LOG_INFO("[Lobby] Created ready indicator entity for userId: " << userId);

    _playersToPosition.insert(userId);
    auto backBtn = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("main_font"),
            sf::Color::White, 36, "Disconnect"),
        rtype::games::rtype::shared::Position(100, 900),
        rtype::games::rtype::client::Rectangle({400, 75}, sf::Color(200, 0, 0),
                                               sf::Color::Red),
        this->_assetsManager, std::function<void()>([this]() {
            try {
                LOG_INFO(
                    "[Lobby] Disconnect button clicked - clearing lobby state");
                this->_networkClient->disconnect();

                _playersToPosition.clear();
                _playerIndexMap.clear();
                _disconnectedPlayers.clear();

                for (auto& [userId, entities] : _listUser) {
                    for (auto entt : entities) {
                        if (this->_registry) {
                            this->_registry->killEntity(entt);
                        }
                    }
                }
                _listUser.clear();

                for (auto entt : _listEntity) {
                    if (this->_registry) {
                        this->_registry->killEntity(entt);
                    }
                }
                _listEntity.clear();

                this->_switchToScene(SceneManager::MAIN_MENU);
                LOG_INFO("[Lobby] Disconnected and switched to main menu");
            } catch (SceneNotFound& e) {
                LOG_ERROR(std::string("Error switching to Main Menu: ") +
                          std::string(e.what()));
            } catch (const std::exception& e) {
                LOG_ERROR("[Lobby] Exception during disconnect: " << e.what());
            }
        }));
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        backBtn, 2);
    this->_listEntity.push_back(backBtn);
}

void Lobby::_removePlayerInfoMenu(uint32_t userId) {
    if (this->_listUser.find(userId) == this->_listUser.end()) {
        return;
    }
    for (auto entity : this->_listUser[userId]) {
        this->_registry->killEntity(entity);
        std::erase(this->_listEntity, entity);
    }
    this->_listUser.erase(userId);
}

void Lobby::_initInfoMenu() {
    auto section = EntityFactory::createSection(
        this->_registry, this->_assetsManager, "",
        sf::FloatRect({kBaseX, kBaseY}, {kBaseW, kBaseH}));
    this->_listEntity.insert(this->_listEntity.end(), section.begin(),
                             section.end());
    auto title = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "Game Info", "main_font",
        sf::Vector2f(static_cast<float>(kBaseX + kBaseW / 2 - 100),
                     static_cast<float>(kBaseY + 20)),
        48);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        title, 1);
    this->_listEntity.push_back(title);

    _readyButtonEntity = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("main_font"),
            sf::Color::White, 32, "Ready"),
        rtype::games::rtype::shared::Position(
            static_cast<float>(kBaseX + kBaseW - 280),
            static_cast<float>(kBaseY + kBaseH - 70)),
        rtype::games::rtype::client::Rectangle(std::pair<int, int>(250, 50),
                                               sf::Color(70, 130, 180),
                                               sf::Color(0, 150, 0)),
        this->_assetsManager, std::function<void()>([this]() {
            _isReady = !_isReady;

            if (_registry->hasComponent<rtype::games::rtype::client::Text>(
                    _readyButtonEntity)) {
                auto& text =
                    _registry->getComponent<rtype::games::rtype::client::Text>(
                        _readyButtonEntity);
                text.textContent = _isReady ? "Not Ready" : "Ready";
                text.text.setString(text.textContent);
            }
            if (_registry->hasComponent<rtype::games::rtype::client::Rectangle>(
                    _readyButtonEntity)) {
                auto& rect =
                    _registry
                        ->getComponent<rtype::games::rtype::client::Rectangle>(
                            _readyButtonEntity);
                rect.mainColor =
                    _isReady ? sf::Color(0, 150, 0) : sf::Color(70, 130, 180);
                rect.currentColor = rect.mainColor;
                rect.rectangle.setFillColor(rect.currentColor);
            }

            if (_networkSystem) {
                auto localUserId = _networkSystem->getLocalUserId();
                if (localUserId.has_value()) {
                    _updatePlayerReadyIndicator(*localUserId, _isReady);
                }
            }

            if (this->_networkClient->sendReady(_isReady)) {
                LOG_INFO("[Lobby] Sent C_READY packet: "
                         << (_isReady ? "ready" : "not ready"));
            } else {
                LOG_ERROR("[Lobby] Failed to send C_READY packet");
            }

            if (!_isReady && _countdownActive) {
                _countdownActive = false;
                _countdownTimer = 3.0f;
                if (_registry->hasComponent<rtype::games::rtype::client::Text>(
                        _countdownTextEntity)) {
                    auto& text =
                        _registry
                            ->getComponent<rtype::games::rtype::client::Text>(
                                _countdownTextEntity);
                    text.textContent = "";
                    text.text.setString(text.textContent);
                }
                LOG_INFO("[Lobby] Countdown cancelled by player");
            }
        }));

    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        _readyButtonEntity, 1);
    this->_listEntity.push_back(_readyButtonEntity);

    _countdownTextEntity = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "", "main_font",
        sf::Vector2f(static_cast<float>(kBaseX + kBaseW / 2 - 150),
                     static_cast<float>(kBaseY + kBaseH / 2)),
        64);
    if (_registry->hasComponent<rtype::games::rtype::client::Text>(
            _countdownTextEntity)) {
        auto& text = _registry->getComponent<rtype::games::rtype::client::Text>(
            _countdownTextEntity);
        text.color = sf::Color::Yellow;
    }
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        _countdownTextEntity, 10);
    this->_listEntity.push_back(_countdownTextEntity);
}

Lobby::Lobby(std::shared_ptr<ECS::Registry> ecs,
             std::shared_ptr<AssetManager> assetManager,
             std::shared_ptr<sf::RenderWindow> window,
             std::function<void(const SceneManager::Scene&)> switchToScene,
             std::shared_ptr<rtype::client::NetworkClient> networkClient,
             std::shared_ptr<rtype::client::ClientNetworkSystem> networkSystem,
             std::shared_ptr<AudioLib> audioLib)
    : AScene(std::move(ecs), std::move(assetManager), std::move(window),
             std::move(audioLib)),
      _networkClient(std::move(networkClient)),
      _networkSystem(std::move(networkSystem)),
      _switchToScene(std::move(switchToScene)) {
    this->_nbrUser = 0;

    if (this->_networkSystem && this->_assetsManager) {
        this->_networkSystem->setEntityFactory(
            rtype::games::rtype::client::RtypeEntityFactory::
                createNetworkEntityFactory(this->_registry,
                                           this->_assetsManager));
    }

    this->_networkClient->onEntityMove(([](rtype::client::EntityMoveEvent) {}));

    this->_networkClient->onGameStart([this](float countdownDuration) {
        LOG_INFO("[Lobby] Server triggered game start with countdown: "
                 << countdownDuration << "s");
        _countdownActive = true;
        _countdownTimer = countdownDuration;
    });

    this->_networkClient->onPlayerReadyStateChanged(
        [this](std::uint32_t userId, bool isReady) {
            LOG_INFO("[Lobby] Server notified: Player "
                     << userId
                     << " ready state: " << (isReady ? "READY" : "NOT READY"));
            _updatePlayerReadyIndicator(userId, isReady);
        });

    this->_networkClient->onEntityDestroy([this](std::uint32_t entityId) {
        _pendingPlayerRemovals.push_back(entityId);
    });

    this->_networkClient->onDisconnected(
        [this](rtype::client::NetworkClient::DisconnectReason reason) {
            LOG_INFO("[Lobby] Client disconnected from server, reason="
                     << static_cast<int>(reason));

            _playersToPosition.clear();
            _playerIndexMap.clear();
            _playerReadyStates.clear();
            _playerReadyIndicators.clear();

            for (auto& [userId, entities] : _listUser) {
                for (auto entt : entities) {
                    if (_registry && _registry->isAlive(entt)) {
                        _registry->killEntity(entt);
                    }
                }
            }
            _listUser.clear();

            for (auto entt : _listEntity) {
                if (_registry && _registry->isAlive(entt)) {
                    _registry->killEntity(entt);
                }
            }
            _listEntity.clear();

            _nbrUser = 0;
            _isReady = false;
            _countdownActive = false;
            _countdownTimer = 3.0f;

            LOG_INFO(
                "[Lobby] Lobby cleaned up after disconnect, switching to main "
                "menu");
        });

    this->_listEntity = (EntityFactory::createBackground(
        this->_registry, this->_assetsManager, "Lobby"));
    this->_initInfoMenu();

    LOG_INFO("[Lobby] Checking for existing player entities...");
    this->_registry->view<rtype::games::rtype::shared::PlayerIdComponent>()
        .each([this](auto playerEntt, auto& id) {
            this->_nbrUser++;
            uint32_t playerId = id.playerId;
            if (_playerIndexMap.find(playerId) != _playerIndexMap.end()) {
                return;
            }
            LOG_INFO("[Lobby] Found existing player entity with playerId: "
                     << playerId << " Total players: " << this->_nbrUser);
            this->_createPlayerInfoMenu(playerId, this->_nbrUser);
        });

    /*
    ** TODO(Samuel): Create a new player info menu when a new user connects use
    *the network client onNewConnected callback and handle disconnected player
    **  this->_networkClient->onNewConnected(([this](uint32_t id) {
    **     this->_nbrUser++;
    **     this->_createPlayerInfoMenu(id, this->_nbrUser);
    **  }));
    **  this->_networkClient->onPlayerDisconnected(([this](uint32_t id) {
    **     this->_nbrUser--;
    **     this->_removePlayerInfoMenu(id);
    **  }));
    */
}

void Lobby::_updatePlayerReadyIndicator(uint32_t userId, bool isReady) {
    if (isReady) {
        _playerReadyStates.insert(userId);
    } else {
        _playerReadyStates.erase(userId);
    }

    if (_playerReadyIndicators.find(userId) != _playerReadyIndicators.end()) {
        auto indicatorEntity = _playerReadyIndicators[userId];
        if (_registry && _registry->isAlive(indicatorEntity)) {
            if (_registry->hasComponent<rtype::games::rtype::client::Text>(
                    indicatorEntity)) {
                auto& text =
                    _registry->getComponent<rtype::games::rtype::client::Text>(
                        indicatorEntity);
                if (isReady) {
                    text.textContent = "READY";
                    text.color = sf::Color::Green;
                } else {
                    text.textContent = "WAITING...";
                    text.color = sf::Color::Yellow;
                }
                text.text.setString(text.textContent);
                text.text.setFillColor(text.color);
            }
        }
    }
}

Lobby::~Lobby() {
    LOG_INFO("[Lobby] Destroying Lobby scene...");

    if (this->_registry) {
        LOG_INFO("[Lobby] Resetting player positions for game scene...");
        this->_registry
            ->view<rtype::games::rtype::shared::PlayerIdComponent,
                   rtype::games::rtype::shared::Position>()
            .each([](auto playerEntt, auto& id, auto& pos) {
                pos.x = 100.0f;
                pos.y = 150.0f + ((id.playerId - 1) * 100.0f);
                LOG_INFO("[Lobby] Reset player "
                         << id.playerId << " position to (" << pos.x << ", "
                         << pos.y << ")");
            });
    }

    for (auto& [userId, entities] : _listUser) {
        for (auto entity : entities) {
            if (_registry) {
                _registry->killEntity(entity);
            }
        }
    }
    _listUser.clear();

    for (auto entity : _listEntity) {
        if (_registry) {
            _registry->killEntity(entity);
        }
    }
    _listEntity.clear();

    for (auto& [userId, indicatorEntity] : _playerReadyIndicators) {
        if (_registry && _registry->isAlive(indicatorEntity)) {
            _registry->killEntity(indicatorEntity);
        }
    }
    _playerReadyIndicators.clear();

    _playersToPosition.clear();
    _playerIndexMap.clear();
    _playerReadyStates.clear();
    _disconnectedPlayers.clear();
    _pendingPlayerRemovals.clear();
    _nbrUser = 0;
    _isReady = false;
    _countdownActive = false;
    _countdownTimer = 3.0f;

    if (_networkSystem) {
        _networkSystem->registerCallbacks();
    }

    LOG_INFO("[Lobby] Lobby scene destroyed successfully");
}
