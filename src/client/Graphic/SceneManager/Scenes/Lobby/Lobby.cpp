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
#include "Graphic.hpp"
#include "GraphicsConstants.hpp"
#include "SceneException.hpp"
#include "games/rtype/client/Components/TextInputComponent.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"

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

    const bool isConnected = this->_networkClient->isConnected();
    if (!isConnected && _isConnected) {
        _isConnected = false;
        try {
            this->_switchToScene(SceneManager::Scene::MAIN_MENU);
        } catch (SceneNotFound& e) {
            LOG_ERROR(std::string("Error switching to Main Menu: ") + e.what());
        }
    } else if (isConnected) {
        _isConnected = true;
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

                    constexpr float spritePixelSize = 17.0f;
                    constexpr float spriteScale = 4.0f;
                    constexpr float scaledSpriteSize =
                        spritePixelSize * spriteScale;
                    constexpr float halfSize = scaledSpriteSize;

                    float posX = myCenterX - halfSize;
                    float posY = kBoxCenterY - halfSize;

                    LOG_INFO("[Lobby] Positioning player "
                             << playerId << " sprite at x=" << posX
                             << " y=" << posY << " (centered at " << myCenterX
                             << ", " << kBoxCenterY << ")");

                    if (this->_registry->hasComponent<
                            rtype::games::rtype::shared::TransformComponent>(
                            playerEntt)) {
                        auto& pos = this->_registry->getComponent<
                            rtype::games::rtype::shared::TransformComponent>(
                            playerEntt);
                        pos.x = posX;
                        pos.y = posY;
                    } else {
                        this->_registry->emplaceComponent<
                            rtype::games::rtype::shared::TransformComponent>(
                            playerEntt, posX, posY);
                    }

                    bool hadGameTag = this->_registry->hasComponent<
                        rtype::games::rtype::client::GameTag>(playerEntt);
                    if (hadGameTag) {
                        this->_registry->removeComponent<
                            rtype::games::rtype::client::GameTag>(playerEntt);
                    }
                    if (!this->_registry->hasComponent<
                            rtype::games::rtype::client::LobbyTag>(
                            playerEntt)) {
                        this->_registry->emplaceComponent<
                            rtype::games::rtype::client::LobbyTag>(playerEntt);
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
            } else {
                text.textContent = "GO!";
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

void Lobby::render(std::shared_ptr<rtype::display::IDisplay> window) {}

void Lobby::pollEvents(const rtype::display::Event& e) {
    if (_textInputSystem) {
        _textInputSystem->handleEvent(*_registry, e);
    }
}

void Lobby::_createPlayerInfoMenu(uint32_t userId, int index) {
    LOG_INFO("[Lobby] Creating player info menu for userId: "
             << userId << " at index: " << index);

    _playerIndexMap[userId] = index;

    float sectionX = kStartPosX + ((index - 1) * kStepX);
    float myCenterX = sectionX + kBoxHalfW;

    auto playerSection = EntityFactory::createSection(
        this->_registry, this->_assetsManager, "",
        ::rtype::display::Rect<float>(sectionX, kBoxTopY, kBoxWidth,
                                      kBoxHeight),
        1);
    this->_listUser.insert({userId, playerSection});
    this->_listEntity.insert(this->_listEntity.end(), playerSection.begin(),
                             playerSection.end());
    LOG_INFO("[Lobby] Created section with " << playerSection.size()
                                             << " entities");

    auto playerLabel = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager,
        "Player " + std::to_string(userId), "main_font",
        ::rtype::display::Vector2<float>(myCenterX - 60, kBoxTopY + 20), 36);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        playerLabel, 3);
    if (this->_registry->hasComponent<rtype::games::rtype::client::Text>(
            playerLabel)) {
        auto& text =
            this->_registry->getComponent<rtype::games::rtype::client::Text>(
                playerLabel);
        text.color = ::rtype::display::Color::Yellow();
    }
    this->_listUser[userId].push_back(playerLabel);
    this->_listEntity.push_back(playerLabel);
    LOG_INFO("[Lobby] Created player label entity");

    auto readyIndicator = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "WAITING...", "main_font",
        ::rtype::display::Vector2<float>(myCenterX - 80, kBoxTopY + 70), 28);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        readyIndicator, 3);
    if (this->_registry->hasComponent<rtype::games::rtype::client::Text>(
            readyIndicator)) {
        auto& text =
            this->_registry->getComponent<rtype::games::rtype::client::Text>(
                readyIndicator);
        text.color = ::rtype::display::Color::Yellow();
    }
    _playerReadyIndicators[userId] = readyIndicator;
    this->_listUser[userId].push_back(readyIndicator);
    this->_listEntity.push_back(readyIndicator);
    LOG_INFO("[Lobby] Created ready indicator entity for userId: " << userId);

    _playersToPosition.insert(userId);
    auto backBtn = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            "main_font", ::rtype::display::Color::White(), 36, "Disconnect"),
        rtype::games::rtype::shared::TransformComponent(
            100.f, Graphic::WINDOW_HEIGHT - 180.f),
        rtype::games::rtype::client::Rectangle(
            {400, 75}, ::rtype::display::Color(200, 0, 0),
            ::rtype::display::Color::Red()),
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
        ::rtype::display::Rect<float>(kBaseX, kBaseY, kBaseW, kBaseH));
    this->_listEntity.insert(this->_listEntity.end(), section.begin(),
                             section.end());
    auto title = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "Game Info", "main_font",
        ::rtype::display::Vector2<float>(
            static_cast<float>(kBaseX + kBaseW / 2 - 100),
            static_cast<float>(kBaseY + 20)),
        48);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        title, 1);
    this->_listEntity.push_back(title);

    _readyButtonEntity = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            "main_font", ::rtype::display::Color::White(), 32, "Ready"),
        rtype::games::rtype::shared::TransformComponent(
            static_cast<float>(kBaseX + kBaseW - 280),
            static_cast<float>(kBaseY + kBaseH - 70)),
        rtype::games::rtype::client::Rectangle(
            std::pair<int, int>(250, 50), ::rtype::display::Color(70, 130, 180),
            ::rtype::display::Color(0, 150, 0)),
        this->_assetsManager, std::function<void()>([this]() {
            _isReady = !_isReady;

            if (_registry->hasComponent<rtype::games::rtype::client::Text>(
                    _readyButtonEntity)) {
                auto& text =
                    _registry->getComponent<rtype::games::rtype::client::Text>(
                        _readyButtonEntity);
                text.textContent = _isReady ? "Not Ready" : "Ready";
            }
            if (_registry->hasComponent<rtype::games::rtype::client::Rectangle>(
                    _readyButtonEntity)) {
                auto& rect =
                    _registry
                        ->getComponent<rtype::games::rtype::client::Rectangle>(
                            _readyButtonEntity);
                rect.mainColor = _isReady
                                     ? ::rtype::display::Color(0, 150, 0)
                                     : ::rtype::display::Color(70, 130, 180);
                rect.currentColor = rect.mainColor;
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
                }
                LOG_INFO("[Lobby] Countdown cancelled by player");
            }
        }));

    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        _readyButtonEntity, 1);
    this->_listEntity.push_back(_readyButtonEntity);

    _countdownTextEntity = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "", "main_font",
        ::rtype::display::Vector2<float>(
            static_cast<float>(kBaseX + kBaseW / 2 - 150),
            static_cast<float>(kBaseY + kBaseH / 2)),
        64);
    if (_registry->hasComponent<rtype::games::rtype::client::Text>(
            _countdownTextEntity)) {
        auto& text = _registry->getComponent<rtype::games::rtype::client::Text>(
            _countdownTextEntity);
        text.color = ::rtype::display::Color::Yellow();
    }
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        _countdownTextEntity, 10);
    this->_listEntity.push_back(_countdownTextEntity);
}

void Lobby::_initChat() {
    auto btnOpenChat = EntityFactory::createButton(
        _registry,
        rtype::games::rtype::client::Text(
            "main_font", rtype::display::Color::White(), 30, "Chat"),
        rtype::games::rtype::shared::TransformComponent(
            575.f, Graphic::WINDOW_HEIGHT - (180.f - (75 / 2))),
        rtype::games::rtype::client::Rectangle(
            {100, 75}, rtype::display::Color(70, 130, 180),
            rtype::display::Color(0, 150, 0)),
        this->_assetsManager, std::function<void()>([this]() {
            for (auto s : this->_messageEntities) {
                if (_registry->hasComponent<
                        rtype::games::rtype::client::HiddenComponent>(s))
                    _registry
                        ->getComponent<
                            rtype::games::rtype::client::HiddenComponent>(s)
                        .isHidden = false;
            }
        }));
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::CenteredBtnTag>(
            btnOpenChat);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        btnOpenChat, 5);
    this->_listEntity.push_back(btnOpenChat);

    auto popupEffect = EntityFactory::createRectangle(
        this->_registry,
        ::rtype::display::Vector2i(static_cast<int>(Graphic::WINDOW_WIDTH),
                                   static_cast<int>(Graphic::WINDOW_HEIGHT)),
        ::rtype::display::Color(0, 0, 0, 150),
        ::rtype::display::Vector2f(0.f, 0.f));
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        popupEffect, 10);
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::HiddenComponent>(
            popupEffect, true);
    _messageEntities.push_back(popupEffect);

    auto chatSection = EntityFactory::createSection(
        _registry, this->_assetsManager, "Chat",
        ::rtype::display::Rect<float>(
            Graphic::WINDOW_WIDTH / 2 - kMessageSectionW / 2,
            Graphic::WINDOW_HEIGHT / 2 - kMessageSectionH / 2, kMessageSectionW,
            kMessageSectionH),
        10);
    for (auto s : chatSection) {
        this->_registry
            ->emplaceComponent<rtype::games::rtype::client::HiddenComponent>(
                s, true);
    }
    _messageEntities.insert(_messageEntities.end(), chatSection.begin(),
                            chatSection.end());

    std::function<void(ECS::Entity entity)> addElementToSection =
        [&](ECS::Entity entity) {
            if (_registry->hasComponent<
                    rtype::games::rtype::shared::TransformComponent>(entity)) {
                auto& pos = _registry->getComponent<
                    rtype::games::rtype::shared::TransformComponent>(entity);
                pos.x += Graphic::WINDOW_WIDTH / 2 - kMessageSectionW / 2;
                pos.y += Graphic::WINDOW_HEIGHT / 2 - kMessageSectionH / 2;
            }
            _messageEntities.push_back(entity);
        };

    // Chat messages display area
    auto chatMessagesDisplay = EntityFactory::createTextInput(
        _registry, this->_assetsManager,
        ::rtype::display::Vector2<float>(20.f, kMessageSectionH - 65.f),
        ::rtype::display::Vector2<float>(kMessageSectionW - 120.f, 50.f),
        "Type your message here", "", 0, false);
    _chatInputEntity = chatMessagesDisplay;
    _registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        chatMessagesDisplay, 12);
    _registry->emplaceComponent<rtype::games::rtype::client::HiddenComponent>(
        chatMessagesDisplay, true);
    addElementToSection(chatMessagesDisplay);

    auto btnSend = EntityFactory::createButton(
        _registry,
        rtype::games::rtype::client::Text(
            "main_font", rtype::display::Color::White(), 36, ">"),
        rtype::games::rtype::shared::TransformComponent(
            kMessageSectionW - 10.f - 80 / 2, kMessageSectionH - 40),
        rtype::games::rtype::client::Rectangle({80, 50},
                                               rtype::display::Color::Blue(),
                                               rtype::display::Color::Red()),
        this->_assetsManager, std::function<void()>([this]() {
            if (_registry->hasComponent<rtype::games::rtype::client::TextInput>(
                    _chatInputEntity)) {
                auto& input =
                    _registry
                        ->getComponent<rtype::games::rtype::client::TextInput>(
                            _chatInputEntity);
                if (!input.content.empty()) {
                    if (this->_networkClient->sendChat(input.content)) {
                        input.content = "";
                        input.cursorPosition = 0;
                    }
                }
            }
        }));
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::CenteredBtnTag>(
            btnSend);
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::HiddenComponent>(
            btnSend, true);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        btnSend, 11);
    addElementToSection(btnSend);
    auto btnClose = EntityFactory::createButton(
        _registry,
        rtype::games::rtype::client::Text(
            "main_font", rtype::display::Color::White(), 30, "X"),
        rtype::games::rtype::shared::TransformComponent(
            kMessageSectionW - 10.f - 80 / 2, 40),
        rtype::games::rtype::client::Rectangle(
            {55, 40}, rtype::display::Color::Red(),
            rtype::display::Color(255, 100, 100, 255)),
        this->_assetsManager, std::function<void()>([this]() {
            for (auto s : this->_messageEntities) {
                if (_registry->hasComponent<
                        rtype::games::rtype::client::HiddenComponent>(s))
                    _registry
                        ->getComponent<
                            rtype::games::rtype::client::HiddenComponent>(s)
                        .isHidden = true;
            }
        }));
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::CenteredBtnTag>(
            btnClose);
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::HiddenComponent>(
            btnClose, true);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        btnClose, 11);
    addElementToSection(btnClose);
    this->_listEntity.insert(this->_listEntity.end(), _messageEntities.begin(),
                             _messageEntities.end());
}

void Lobby::_addChatMessage(uint32_t userId, const std::string& message) {
    std::string formattedMsg;
    if (userId == 0) {
        formattedMsg = "[System]: " + message;
    } else {
        formattedMsg = "[Player " + std::to_string(userId) + "]: " + message;
    }
    if (formattedMsg.length() > kMessageMaxCharacters) {
        LOG_WARNING("[Lobby] Chat message from user "
                    << userId << " is too long and was truncated.");
        formattedMsg = formattedMsg.substr(0, kMessageMaxCharacters) + "...";
    }
    _chatHistory.push_back(formattedMsg);

    // Keep only last 10 messages
    if (_chatHistory.size() > kMessagesMaxDisplay) {
        _chatHistory.erase(_chatHistory.begin());
    }

    // Clear old entities
    for (auto ent : _chatHistoryEntities) {
        if (_registry->isAlive(ent)) _registry->killEntity(ent);
    }
    _chatHistoryEntities.clear();

    float startY = Graphic::WINDOW_HEIGHT / 2 - kMessageSectionH / 2 + 80;
    float startX = Graphic::WINDOW_WIDTH / 2 - kMessageSectionW / 2 + 35;

    for (const auto& msg : _chatHistory) {
        auto textEnt = EntityFactory::createStaticText(
            _registry, _assetsManager, msg, "main_font",
            ::rtype::display::Vector2<float>(startX, startY), 20);

        _registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
            textEnt, 12);
        _registry
            ->emplaceComponent<rtype::games::rtype::client::HiddenComponent>(
                textEnt, true);

        bool isChatHidden = true;
        if (!_messageEntities.empty()) {
            if (_registry->hasComponent<
                    rtype::games::rtype::client::HiddenComponent>(
                    _messageEntities[0])) {
                isChatHidden =
                    _registry
                        ->getComponent<
                            rtype::games::rtype::client::HiddenComponent>(
                            _messageEntities[0])
                        .isHidden;
            }
        }
        _registry
            ->getComponent<rtype::games::rtype::client::HiddenComponent>(
                textEnt)
            .isHidden = isChatHidden;

        _chatHistoryEntities.push_back(textEnt);
        _messageEntities.push_back(textEnt);
        startY += 30;
    }
}

Lobby::Lobby(std::shared_ptr<ECS::Registry> ecs,
             std::shared_ptr<AssetManager> assetManager,
             std::shared_ptr<::rtype::display::IDisplay> window,
             std::function<void(const SceneManager::Scene&)> switchToScene,
             std::shared_ptr<rtype::client::NetworkClient> networkClient,
             std::shared_ptr<rtype::client::ClientNetworkSystem> networkSystem,
             std::shared_ptr<AudioLib> audioLib)
    : AScene(ecs, assetManager, window, audioLib),
      _networkClient(std::move(networkClient)),
      _networkSystem(std::move(networkSystem)),
      _textInputSystem(
          std::make_shared<rtype::games::rtype::client::TextInputSystem>(
              window)),
      _switchToScene(std::move(switchToScene)) {
    this->_nbrUser = 0;

    _isConnected =
        (this->_networkClient && this->_networkClient->isConnected());

    if (this->_networkSystem && this->_assetsManager) {
        this->_networkSystem->setEntityFactory(
            rtype::games::rtype::client::RtypeEntityFactory::
                createNetworkEntityFactory(this->_registry,
                                           this->_assetsManager));
    }

    this->_networkClient->onEntityMove(([](rtype::client::EntityMoveEvent) {}));

    this->_networkClient->onGameStart([this](float countdownDuration) {
        try {
            if (!_initialized || !this->_registry) {
                return;
            }

            LOG_INFO("[Lobby] Server triggered game start with countdown: "
                     << countdownDuration << "s");
            if (countdownDuration > 0.0f) {
                _countdownActive = true;
                _countdownTimer = countdownDuration;
            } else {
                _countdownActive = false;
                _countdownTimer = 0.0f;
                if (_registry->hasComponent<rtype::games::rtype::client::Text>(
                        _countdownTextEntity)) {
                    auto& text =
                        _registry
                            ->getComponent<rtype::games::rtype::client::Text>(
                                _countdownTextEntity);
                    text.textContent = "";
                }
                LOG_INFO("[Lobby] Countdown cancelled by server");
            }
        } catch (const std::exception& e) {
            LOG_ERROR("[Lobby] Exception in onGameStart: " << e.what());
        } catch (...) {
            LOG_ERROR("[Lobby] Unknown exception in onGameStart");
        }
    });

    this->_networkClient->onGameStateChange(
        [this](rtype::client::GameStateEvent event) {
            try {
                if (!_initialized || !this->_registry) {
                    return;
                }

                LOG_INFO("[Lobby] Received game state change: "
                         << static_cast<int>(event.state));

                if (event.state == rtype::network::GameState::Running) {
                    LOG_INFO(
                        "[Lobby] Server indicates game is now Running - "
                        "switching to game scene");
                    _countdownActive = false;
                    this->_switchToScene(SceneManager::Scene::IN_GAME);
                }
            } catch (const std::exception& e) {
                LOG_ERROR(
                    "[Lobby] Exception in onGameStateChange: " << e.what());
            } catch (...) {
                LOG_ERROR("[Lobby] Unknown exception in onGameStateChange");
            }
        });

    this->_networkClient->onJoinLobbyResponse([this](bool accepted,
                                                     uint8_t reason) {
        try {
            if (!_initialized || !this->_registry) {
                return;
            }

            if (!accepted) {
                LOG_ERROR("[Lobby] Join lobby rejected by server, reason="
                          << static_cast<int>(reason));
                this->_networkClient->disconnect();
                return;
            }

            LOG_INFO("[Lobby] Join lobby accepted by server");
        } catch (const std::exception& e) {
            LOG_ERROR("[Lobby] Exception in onJoinLobbyResponse: " << e.what());
        } catch (...) {
            LOG_ERROR("[Lobby] Unknown exception in onJoinLobbyResponse");
        }
    });

    this->_networkClient->onPlayerReadyStateChanged([this](std::uint32_t userId,
                                                           bool isReady) {
        try {
            if (!_initialized) {
                return;
            }

            if (!this->_registry) {
                return;
            }

            LOG_INFO("[Lobby] Server notified: Player "
                     << userId
                     << " ready state: " << (isReady ? "READY" : "NOT READY"));

            if (_disconnectedPlayers.count(userId) > 0) {
                LOG_INFO("[Lobby] Ignoring ready state for disconnected player "
                         << userId);
                return;
            }

            if (_listUser.find(userId) == _listUser.end()) {
                LOG_DEBUG("[Lobby] Received ready state for player "
                          << userId
                          << " but player menu not created yet - waiting...");
                return;
            }

            _updatePlayerReadyIndicator(userId, isReady);
        } catch (const std::exception& e) {
            LOG_ERROR(
                "[Lobby] Exception in onPlayerReadyStateChanged: " << e.what());
        } catch (...) {
            LOG_ERROR("[Lobby] Unknown exception in onPlayerReadyStateChanged");
        }
    });

    this->_networkClient->onEntityDestroy([this](std::uint32_t entityId) {
        try {
            if (!_initialized || !this->_registry) {
                return;
            }
            this->onEntityDestroyEvent(entityId);
        } catch (const std::exception& e) {
            LOG_ERROR("[Lobby] Exception in onEntityDestroy: " << e.what());
        } catch (...) {
            LOG_ERROR("[Lobby] Unknown exception in onEntityDestroy");
        }
    });

    this->_networkClient->onDisconnected([this](rtype::client::NetworkClient::
                                                    DisconnectReason reason) {
        try {
            if (!_initialized || !this->_registry) {
                return;
            }

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

            _isConnected = false;
            _initialized = false;
            try {
                this->_switchToScene(SceneManager::Scene::MAIN_MENU);
            } catch (SceneNotFound& e) {
                LOG_ERROR(std::string("Error switching to Main Menu: ") +
                          e.what());
            }
            LOG_INFO(
                "[Lobby] Lobby cleaned up after disconnect, switching to main "
                "menu");
        } catch (const std::exception& e) {
            LOG_ERROR("[Lobby] Exception in onDisconnected: " << e.what());
        } catch (...) {
            LOG_ERROR("[Lobby] Unknown exception in onDisconnected");
        }
    });

    this->_networkClient->onChatReceived(
        [this](std::uint32_t userId, std::string message) {
            this->_addChatMessage(userId, message);
        });

    this->_listEntity = (EntityFactory::createBackground(
        this->_registry, this->_assetsManager, "Lobby"));
    this->_initInfoMenu();
    this->_initChat();

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

    _initialized = true;
    LOG_INFO("[Lobby] Initialization complete");
}

void Lobby::_updatePlayerReadyIndicator(uint32_t userId, bool isReady) {
    if (!_registry) {
        LOG_WARNING(
            "[Lobby] _updatePlayerReadyIndicator called but registry is null!");
        return;
    }

    if (_disconnectedPlayers.count(userId) > 0) {
        return;
    }

    if (_listUser.find(userId) == _listUser.end()) {
        return;
    }

    if (_playerReadyIndicators.find(userId) == _playerReadyIndicators.end()) {
        LOG_WARNING("[Lobby] No ready indicator for player " << userId);
        return;
    }

    if (isReady) {
        _playerReadyStates.insert(userId);
    } else {
        _playerReadyStates.erase(userId);
    }

    auto indicatorEntity = _playerReadyIndicators[userId];

    if (!_registry->isAlive(indicatorEntity)) {
        LOG_WARNING("[Lobby] Ready indicator entity is not alive for player "
                    << userId);
        _playerReadyIndicators.erase(userId);
        return;
    }

    if (_registry->hasComponent<rtype::games::rtype::client::Text>(
            indicatorEntity)) {
        auto& text = _registry->getComponent<rtype::games::rtype::client::Text>(
            indicatorEntity);
        if (isReady) {
            text.textContent = "READY";
            text.color = ::rtype::display::Color::Green();
        } else {
            text.textContent = "WAITING...";
            text.color = ::rtype::display::Color::Yellow();
        }
    }
}

void Lobby::onEntityDestroyEvent(std::uint32_t entityId) {
    bool found = false;
    for (const auto& [userId, entities] : _listUser) {
        auto it = std::find_if(
            entities.begin(), entities.end(),
            [entityId](const ECS::Entity& ent) { return ent.id == entityId; });
        if (it != entities.end()) {
            _pendingPlayerRemovals.push_back(userId);
            found = true;
            break;
        }
    }
    if (!found) {
        LOG_WARNING("[Lobby] onEntityDestroy: unknown entityId "
                    << entityId << " - no matching user found");
    }
}

Lobby::~Lobby() {
    LOG_INFO("[Lobby] Destroying Lobby scene...");

    _initialized = false;

    if (this->_registry) {
        LOG_INFO("[Lobby] Resetting player positions for game scene...");
        this->_registry
            ->view<rtype::games::rtype::shared::PlayerIdComponent,
                   rtype::games::rtype::shared::TransformComponent>()
            .each([this](auto playerEntt, auto& id, auto& pos) {
                pos.x = 100.0f;
                pos.y = 150.0f + ((id.playerId - 1) * 100.0f);
                LOG_INFO("[Lobby] Reset player "
                         << id.playerId << " position to (" << pos.x << ", "
                         << pos.y << ")");

                if (!this->_registry
                         ->hasComponent<rtype::games::rtype::client::GameTag>(
                             playerEntt)) {
                    this->_registry->emplaceComponent<
                        rtype::games::rtype::client::GameTag>(playerEntt);
                    LOG_DEBUG("[Lobby] Re-added GameTag to player "
                              << id.playerId << " for in-game scene rendering");
                }
                if (this->_registry
                        ->hasComponent<rtype::games::rtype::client::LobbyTag>(
                            playerEntt)) {
                    this->_registry->removeComponent<
                        rtype::games::rtype::client::LobbyTag>(playerEntt);
                }
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
