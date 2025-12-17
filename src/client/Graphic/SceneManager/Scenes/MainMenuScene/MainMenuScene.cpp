/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** MainMenuScene.cpp
*/

#include "MainMenuScene.hpp"

#include <cstdlib>
#include <ctime>
#include <functional>
#include <memory>
#include <random>
#include <string>
#include <utility>

#include <SFML/Graphics/Text.hpp>

#include "AllComponents.hpp"
#include "Components/TextInputComponent.hpp"
#include "EntityFactory/EntityFactory.hpp"
#include "Logger/Macros.hpp"
#include "SceneManager/SceneException.hpp"

static constexpr float kConnectionPanelX = 1400.f;
static constexpr float kConnectionPanelY = 300.f;
static constexpr float kConnectionPanelWidth = 450.f;
static constexpr float kConnectionPanelHeight = 350.f;
static constexpr float kInputWidth = 300.f;
static constexpr float kInputHeight = 40.f;
static constexpr float kLabelOffsetX = 30.f;
static constexpr float kInputOffsetX = 120.f;
static constexpr std::string kIp = "127.0.0.1";
static constexpr std::uint16_t kPort = 4242;

void MainMenuScene::_createAstroneerVessel() {
    auto astroneerVessel = this->_registry->spawnEntity();
    this->_registry->emplaceComponent<rtype::games::rtype::client::Image>(
        astroneerVessel,
        this->_assetsManager->textureManager->get("astro_vessel"));
    this->_registry->emplaceComponent<rtype::games::rtype::shared::Position>(
        astroneerVessel, 1900, 1060);
    this->_registry->emplaceComponent<rtype::games::rtype::client::Size>(
        astroneerVessel, 0.3, 0.3);
    this->_registry
        ->emplaceComponent<rtype::games::rtype::shared::VelocityComponent>(
            astroneerVessel, -135.f, -75.f);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        astroneerVessel, -1);
    this->_listEntity.push_back(astroneerVessel);
}

void MainMenuScene::_createFakePlayer() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib150(1, 150);
    std::uniform_int_distribution<> distrib15(1, 15);

    for (int i = 0; i < nbr_vessels; i++) {
        auto fakePlayer = this->_registry->spawnEntity();
        this->_registry->emplaceComponent<rtype::games::rtype::client::Image>(
            fakePlayer,
            this->_assetsManager->textureManager->get("player_vessel"));
        this->_registry
            ->emplaceComponent<rtype::games::rtype::client::TextureRect>(
                fakePlayer, std::pair<int, int>({0, 0}),
                std::pair<int, int>({33, 17}));
        this->_registry
            ->emplaceComponent<rtype::games::rtype::shared::Position>(
                fakePlayer, (-10 * (distrib150(gen) + 50)),
                72 * (distrib15(gen) % 15));
        this->_registry->emplaceComponent<rtype::games::rtype::client::Size>(
            fakePlayer, 2.2, 2.2);
        this->_registry
            ->emplaceComponent<rtype::games::rtype::shared::VelocityComponent>(
                fakePlayer, (distrib150(gen) % 150) + 75, 0.f);
        this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
            fakePlayer, 0);
        this->_listEntity.push_back(fakePlayer);
    }
}

void MainMenuScene::_createConnectionPanel(
    std::function<void(const SceneManager::Scene&)> switchToScene) {
    auto panelEntities = EntityFactory::createSection(
        this->_registry, this->_assetsManager, "",
        sf::FloatRect(
            sf::Vector2f(kConnectionPanelX, kConnectionPanelY),
            sf::Vector2f(kConnectionPanelWidth, kConnectionPanelHeight)));
    for (auto entity : panelEntities) {
        this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
            entity, 10);
    }
    this->_listEntity.insert(this->_listEntity.end(), panelEntities.begin(),
                             panelEntities.end());

    auto title = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "Connect to Server",
        "title_font",
        sf::Vector2f(kConnectionPanelX + 50.f, kConnectionPanelY + 20.f), 32);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        title, 11);
    this->_listEntity.push_back(title);
    auto ipLabel = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "IP:", "main_font",
        sf::Vector2f(kConnectionPanelX + kLabelOffsetX,
                     kConnectionPanelY + 90.f),
        24);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        ipLabel, 11);
    this->_listEntity.push_back(ipLabel);
    _ipInputEntity = EntityFactory::createTextInput(
        this->_registry, this->_assetsManager,
        sf::FloatRect(sf::Vector2f(kConnectionPanelX + kInputOffsetX,
                                   kConnectionPanelY + 85.f),
                      sf::Vector2f(kInputWidth, kInputHeight)),
        "127.0.0.1", "127.0.0.1", 15, false);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        _ipInputEntity, 12);
    this->_listEntity.push_back(_ipInputEntity);
    auto portLabel = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "Port:", "main_font",
        sf::Vector2f(kConnectionPanelX + kLabelOffsetX,
                     kConnectionPanelY + 150.f),
        24);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        portLabel, 11);
    this->_listEntity.push_back(portLabel);
    _portInputEntity = EntityFactory::createTextInput(
        this->_registry, this->_assetsManager,
        sf::FloatRect(sf::Vector2f(kConnectionPanelX + kInputOffsetX,
                                   kConnectionPanelY + 145.f),
                      sf::Vector2f(kInputWidth, kInputHeight)),
        "4242", "4242", 5, true);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        _portInputEntity, 12);
    this->_listEntity.push_back(_portInputEntity);
    _statusEntity = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "", "main_font",
        sf::Vector2f(kConnectionPanelX + kLabelOffsetX,
                     kConnectionPanelY + 200.f),
        18);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        _statusEntity, 11);
    this->_listEntity.push_back(_statusEntity);
    auto connectButton = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("main_font"),
            sf::Color::White, 28, "Connect"),
        rtype::games::rtype::shared::Position(kConnectionPanelX + 125.f,
                                              kConnectionPanelY + 260.f),
        rtype::games::rtype::client::Rectangle({200, 60}, sf::Color(0, 150, 0),
                                               sf::Color(0, 200, 0)),
        this->_assetsManager, std::function<void()>([this, switchToScene]() {
            this->_onConnectClicked(switchToScene);
        }));
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        connectButton, 12);
    this->_listEntity.push_back(connectButton);
}

void MainMenuScene::_onConnectClicked(
    std::function<void(const SceneManager::Scene&)> switchToScene) {
    if (!_networkClient) {
        _updateStatus("Error: Network not available", sf::Color::Red);
        return;
    }
    std::string ip = kIp;
    std::uint16_t port = kPort;

    if (_registry->hasComponent<rtype::games::rtype::client::TextInput>(
            _ipInputEntity)) {
        auto& ipInput =
            _registry->getComponent<rtype::games::rtype::client::TextInput>(
                _ipInputEntity);
        if (!ipInput.content.empty()) {
            ip = ipInput.content;
        }
    }

    if (_registry->hasComponent<rtype::games::rtype::client::TextInput>(
            _portInputEntity)) {
        auto& portInput =
            _registry->getComponent<rtype::games::rtype::client::TextInput>(
                _portInputEntity);
        if (!portInput.content.empty()) {
            try {
                port = static_cast<std::uint16_t>(std::stoi(portInput.content));
            } catch (...) {
                _updateStatus("Invalid port number", sf::Color::Red);
                return;
            }
        }
    }

    _updateStatus("Connecting to " + ip + ":" + std::to_string(port) + "...",
                  sf::Color::Yellow);
    std::weak_ptr<ECS::Registry> weakRegistry = _registry;
    ECS::Entity statusEntity = _statusEntity;

    _networkClient->onConnected([weakRegistry, switchToScene,
                                 statusEntity](std::uint32_t userId) {
        auto reg = weakRegistry.lock();
        if (!reg) return;

        LOG_INFO("[Client] Connected with user ID: " << userId);

        if (reg->isAlive(statusEntity) &&
            reg->hasComponent<rtype::games::rtype::client::Text>(
                statusEntity)) {
            auto& text = reg->getComponent<rtype::games::rtype::client::Text>(
                statusEntity);
            text.textContent = "Connected! Starting game...";
            text.text.setString("Connected! Starting game...");
            text.text.setFillColor(sf::Color::Green);
        }

        try {
            switchToScene(SceneManager::IN_GAME);
        } catch (SceneNotFound& e) {
            LOG_ERROR(std::string("Error switching to Game: ") +
                      std::string(e.what()));
        }
    });

    _networkClient->onDisconnected(
        [weakRegistry,
         statusEntity](rtype::client::NetworkClient::DisconnectReason reason) {
            auto reg = weakRegistry.lock();
            if (!reg) return;

            std::string reasonStr;
            switch (reason) {
                case rtype::network::DisconnectReason::Timeout:
                    reasonStr = "Connection timed out";
                    break;
                case rtype::network::DisconnectReason::MaxRetriesExceeded:
                    reasonStr = "Server unreachable";
                    break;
                case rtype::network::DisconnectReason::ProtocolError:
                    reasonStr = "Protocol error";
                    break;
                case rtype::network::DisconnectReason::RemoteRequest:
                    reasonStr = "Server closed connection";
                    break;
                default:
                    reasonStr = "Disconnected";
                    break;
            }

            if (reg->isAlive(statusEntity) &&
                reg->hasComponent<rtype::games::rtype::client::Text>(
                    statusEntity)) {
                auto& text =
                    reg->getComponent<rtype::games::rtype::client::Text>(
                        statusEntity);
                text.textContent = reasonStr;
                text.text.setString(reasonStr);
                text.text.setFillColor(sf::Color::Red);
            }
        });
    if (!_networkClient->connect(ip, port)) {
        _updateStatus("Failed to start connection", sf::Color::Red);
    }
}

void MainMenuScene::_updateStatus(const std::string& message, sf::Color color) {
    if (!_registry) return;
    if (!_registry->isAlive(_statusEntity)) return;
    if (!_registry->hasComponent<rtype::games::rtype::client::Text>(
            _statusEntity)) {
        return;
    }

    auto& text = _registry->getComponent<rtype::games::rtype::client::Text>(
        _statusEntity);
    text.textContent = message;
    text.text.setString(message);
    text.text.setFillColor(color);
}

void MainMenuScene::update(float dt) {
    if (_networkClient && !_networkClient->isConnected()) {
        _networkClient->poll();
    }
}

void MainMenuScene::render(std::shared_ptr<sf::RenderWindow> window) {
    auto view = _registry->view<rtype::games::rtype::client::TextInput,
                                rtype::games::rtype::shared::Position,
                                rtype::games::rtype::client::TextInputTag>();

    view.each([window, this](auto entity, auto& input, auto& pos, auto) {
        input.background.setPosition({pos.x, pos.y});
        input.text.setPosition({pos.x + 10.f, pos.y + 5.f});
        window->draw(input.background);
        window->draw(input.text);
    });
}

void MainMenuScene::pollEvents(const sf::Event& e) {
    if (_textInputSystem) {
        _textInputSystem->handleEvent(*_registry, e);
    }
}

MainMenuScene::MainMenuScene(
    std::shared_ptr<ECS::Registry> ecs,
    std::shared_ptr<AssetManager> assetsManager,
    std::shared_ptr<sf::RenderWindow> window,
    std::function<void(const SceneManager::Scene&)> switchToScene,
    std::shared_ptr<rtype::client::NetworkClient> networkClient,
    std::shared_ptr<rtype::client::ClientNetworkSystem> networkSystem,
    std::shared_ptr<AudioLib> audioLib)
    : AScene(ecs, assetsManager, window, audioLib),
      _networkClient(std::move(networkClient)),
      _networkSystem(std::move(networkSystem)),
      _textInputSystem(
          std::make_shared<rtype::games::rtype::client::TextInputSystem>(
              window)) {
    this->_listEntity = (EntityFactory::createBackground(
        this->_registry, this->_assetsManager, "R-TYPE"));
    this->_createAstroneerVessel();
    this->_createFakePlayer();
    this->_createConnectionPanel(switchToScene);
    auto playBtn = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("main_font"),
            sf::Color::White, 36, "Play"),
        rtype::games::rtype::shared::Position(100, 350),
        rtype::games::rtype::client::Rectangle({400, 75}, sf::Color::Blue,
                                               sf::Color::Red),
        this->_assetsManager, std::function<void()>([switchToScene]() {
            try {
                switchToScene(SceneManager::IN_GAME);
            } catch (SceneNotFound& e) {
                LOG_ERROR(std::string("Error switching to Game Menu: ") +
                          std::string(e.what()));
            }
        }));
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        playBtn, 1);
    this->_listEntity.push_back(playBtn);
    auto howToPlayBtn = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("main_font"),
            sf::Color::White, 36, "How to Play"),
        rtype::games::rtype::shared::Position(100, 470),
        rtype::games::rtype::client::Rectangle({400, 75}, sf::Color::Blue,
                                               sf::Color::Red),
        this->_assetsManager, std::function<void()>([switchToScene]() {
            try {
                switchToScene(SceneManager::HOW_TO_PLAY);
            } catch (SceneNotFound& e) {
                LOG_ERROR(std::string("Error switching to How To Play: ") +
                          std::string(e.what()));
            }
        }));
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        howToPlayBtn, 1);
    this->_listEntity.push_back(howToPlayBtn);
    auto settingsBtn = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("main_font"),
            sf::Color::White, 36, "Settings"),
        rtype::games::rtype::shared::Position(100, 590),
        rtype::games::rtype::client::Rectangle({400, 75}, sf::Color::Blue,
                                               sf::Color::Red),
        this->_assetsManager, std::function<void()>([switchToScene]() {
            try {
                switchToScene(SceneManager::SETTINGS_MENU);
            } catch (SceneNotFound& e) {
                LOG_ERROR(std::string("Error switching to Settings Menu: ") +
                          std::string(e.what()));
            }
        }));
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        settingsBtn, 1);
    this->_listEntity.push_back(settingsBtn);
    auto quitBtn = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("main_font"),
            sf::Color::White, 36, "Quit"),
        rtype::games::rtype::shared::Position(100, 710),
        rtype::games::rtype::client::Rectangle({400, 75}, sf::Color::Blue,
                                               sf::Color::Red),
        this->_assetsManager,
        std::function<void()>([this]() { this->_window->close(); }));
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        quitBtn, 1);
    this->_listEntity.push_back(quitBtn);
    this->_assetsManager->audioManager->load(
        "main_menu_music",
        this->_assetsManager->configGameAssets.assets.music.mainMenu);
    auto bgMusic = this->_assetsManager->audioManager->get("main_menu_music");
    this->_audio->loadMusic(bgMusic);
    this->_audio->setLoop(true);
    this->_audio->play();
}

MainMenuScene::~MainMenuScene() {
    if (_networkClient) {
        _networkClient->onConnected([](std::uint32_t) {});
        _networkClient->onDisconnected([](rtype::network::DisconnectReason) {});
    }
}
