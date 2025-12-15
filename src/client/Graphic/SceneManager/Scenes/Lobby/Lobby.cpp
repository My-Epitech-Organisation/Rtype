/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** Lobby.cpp
*/

#include "Lobby.hpp"

#include <utility>

#include "Components/HiddenComponent.hpp"
#include "Components/ZIndexComponent.hpp"
#include "EntityFactory/EntityFactory.hpp"
#include "SceneException.hpp"

constexpr int kSectionX = 210;
constexpr int kSectionY = 250;
constexpr int kSectionW = 1500;
constexpr int kSectionH = 650;

void Lobby::update(float dt) {
    if (!this->_networkClient->ping() || !this->_networkClient->isConnected()) {
        this->_switchToScene(SceneManager::Scene::MAIN_MENU);
    }
}

void Lobby::render(std::shared_ptr<sf::RenderWindow> window) {}

void Lobby::pollEvents(const sf::Event& e) {}

void Lobby::_createPlayerInfoMenu(uint32_t userId, int index) {
    auto playerSection = EntityFactory::createSection(
        this->_registry, this->_assetsManager,
        "Player " + std::to_string(userId),
        sf::FloatRect({static_cast<float>((kSectionX + 45) +
                                          (index - 1) * (kSectionW / 4 - 15)),
                       kSectionY + (kSectionH / 5)},
                      {(kSectionW / 4 - 45), kSectionH / 1.5}),
        1);
    this->_listUser.insert({userId, playerSection});
    this->_listEntity.insert(this->_listEntity.end(), playerSection.begin(),
                             playerSection.end());
    auto backBtn = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("main_font"),
            sf::Color::White, 36, "Disconnect"),
        rtype::games::rtype::shared::Position(100, 900),
        rtype::games::rtype::client::Rectangle({400, 75}, sf::Color::Red,
                                               sf::Color::Red),
        this->_assetsManager, std::function<void()>([this]() {
            try {
                this->_networkClient->disconnect();
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
        sf::FloatRect({kSectionX, kSectionY}, {kSectionW, kSectionH}));
    this->_listEntity.insert(this->_listEntity.end(), section.begin(),
                             section.end());
    auto title = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "Game Info", "main_font",
        sf::Vector2f(static_cast<float>(kSectionX + kSectionW / 2 - 100),
                     static_cast<float>(kSectionY + 20)),
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
            static_cast<float>(kSectionX + kSectionW - 280),
            static_cast<float>(kSectionY + kSectionH - 70)),
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
    this->_listEntity = (EntityFactory::createBackground(
        this->_registry, this->_assetsManager, "Lobby"));
    this->_initInfoMenu();
    this->_nbrUser = 1;
    this->_createPlayerInfoMenu(0, this->_nbrUser);

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
