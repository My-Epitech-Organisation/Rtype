/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** Lobby.cpp
*/

#include "Lobby.hpp"

#include <ranges>
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
    if (!this->_networkClient->ping() || !this->_networkClient->isConnected()) {
        this->_switchToScene(SceneManager::Scene::MAIN_MENU);
    }
}

void Lobby::render(std::shared_ptr<sf::RenderWindow> window) {}

void Lobby::pollEvents(const sf::Event& e) {}

void Lobby::_createPlayerInfoMenu(uint32_t userId, int index) {
    float sectionX = kStartPosX + ((index - 1) * kStepX);
    auto playerSection = EntityFactory::createSection(
        this->_registry, this->_assetsManager,
        "Player " + std::to_string(userId),
        sf::FloatRect({sectionX, kBoxTopY}, {kBoxWidth, kBoxHeight}), 1);
    this->_listUser.insert({userId, playerSection});
    this->_listEntity.insert(this->_listEntity.end(), playerSection.begin(),
                             playerSection.end());
    this->_registry
        ->view<rtype::games::rtype::shared::PlayerIdComponent,
               rtype::games::rtype::client::ZIndex>()
        .each([this, userId](auto playerEntt, auto id, auto& Zindex) {
            if (userId != id.playerId) return;
            float mySectionX = kStartPosX + ((this->_nbrUser - 1) * kStepX);
            float myCenterX = mySectionX + kBoxHalfW;
            if (this->_registry
                    ->hasComponent<rtype::games::rtype::shared::Position>(
                        playerEntt)) {
                auto& pos =
                    this->_registry
                        ->getComponent<rtype::games::rtype::shared::Position>(
                            playerEntt);
                pos.x = myCenterX;
                pos.y = kBoxCenterY;
            } else {
                this->_registry
                    ->emplaceComponent<rtype::games::rtype::shared::Position>(
                        playerEntt, myCenterX, kBoxCenterY);
            }
            Zindex.depth = 4;
            if (this->_listUser[userId].back() == playerEntt) return;
            this->_listUser[userId].push_back(playerEntt);
        });
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
                this->_networkClient->disconnect();
                for (auto entities : this->_listUser | std::views::values) {
                    for (auto entt : entities)
                        this->_registry->killEntity(entt);
                }
                this->_switchToScene(SceneManager::MAIN_MENU);
            } catch (SceneNotFound& e) {
                LOG_ERROR(std::string("Error switching to Main Menu: ") +
                          std::string(e.what()));
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

    auto playBtn = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("main_font"),
            sf::Color::White, 32, "Start Game"),
        rtype::games::rtype::shared::Position(
            static_cast<float>(kBaseX + kBaseW - 280),
            static_cast<float>(kBaseY + kBaseH - 70)),
        rtype::games::rtype::client::Rectangle(std::pair<int, int>(250, 50),
                                               sf::Color(70, 130, 180),
                                               sf::Color(0, 150, 0)),
        this->_assetsManager, std::function<void()>([this]() {
            // TODO(Samuel): Send start game packet to server
        }));

    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        playBtn, 1);
    this->_listEntity.push_back(playBtn);
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
    this->_networkClient->onEntityMove(([](rtype::client::EntityMoveEvent) {}));
    this->_networkClient->onEntitySpawn(([this](rtype::client::EntitySpawnEvent
                                                    type) {
        if (type.type == rtype::network::EntityType::Player) {
            LOG_INFO("[Lobby] Spawned player entity with ID: "
                     << type.entityId << " for user ID: " << type.userId
                     << " Nbr players: " << this->_nbrUser);
            this->_nbrUser++;
            auto ent = this->_registry->spawnEntity();
            rtype::games::rtype::client::RtypeEntityFactory::setupPlayerEntity(
                *this->_registry, this->_assetsManager, ent, type.userId);
            this->_createPlayerInfoMenu(type.userId, this->_nbrUser);
        }
    }));
    this->_listEntity = (EntityFactory::createBackground(
        this->_registry, this->_assetsManager, "Lobby"));
    if (this->_networkClient->userId().has_value()) {
        this->_nbrUser = 1;
        auto ent = this->_registry->spawnEntity();
        rtype::games::rtype::client::RtypeEntityFactory::setupPlayerEntity(
            *this->_registry, this->_assetsManager, ent,
            *this->_networkClient->userId());
        this->_createPlayerInfoMenu(*this->_networkClient->userId(),
                                    this->_nbrUser);
    }
    this->_initInfoMenu();
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

Lobby::~Lobby() {
    if (_networkClient) {
        this->_networkClient->onEntitySpawn(
            [](rtype::client::EntitySpawnEvent) {});

        this->_networkClient->onEntityMove(
            [](rtype::client::EntityMoveEvent) {});
    }
}
