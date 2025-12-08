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
        this->_registry, this->_assetsManager, "", kConnectionPanelX,
        kConnectionPanelY, kConnectionPanelWidth, kConnectionPanelHeight);
    this->_listEntity.insert(this->_listEntity.end(), panelEntities.begin(),
                             panelEntities.end());

    auto title = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "Connect to Server",
        "title_font", kConnectionPanelX + 50.f, kConnectionPanelY + 20.f, 32);
    this->_listEntity.push_back(title);
    auto ipLabel = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "IP:", "title_font",
        kConnectionPanelX + kLabelOffsetX, kConnectionPanelY + 90.f, 24);
    this->_listEntity.push_back(ipLabel);
    _ipInputEntity = EntityFactory::createTextInput(
        this->_registry, this->_assetsManager,
        kConnectionPanelX + kInputOffsetX, kConnectionPanelY + 85.f,
        kInputWidth, kInputHeight, "127.0.0.1", "127.0.0.1", 15, false);
    this->_listEntity.push_back(_ipInputEntity);
    auto portLabel = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "Port:", "title_font",
        kConnectionPanelX + kLabelOffsetX, kConnectionPanelY + 150.f, 24);
    this->_listEntity.push_back(portLabel);
    _portInputEntity = EntityFactory::createTextInput(
        this->_registry, this->_assetsManager,
        kConnectionPanelX + kInputOffsetX, kConnectionPanelY + 145.f,
        kInputWidth, kInputHeight, "4242", "4242", 5, true);
    this->_listEntity.push_back(_portInputEntity);
    _statusEntity = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "", "title_font",
        kConnectionPanelX + kLabelOffsetX, kConnectionPanelY + 200.f, 18);
    this->_listEntity.push_back(_statusEntity);
    this->_listEntity.push_back(EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("title_font"),
            sf::Color::White, 28, "Connect"),
        rtype::games::rtype::shared::Position(kConnectionPanelX + 125.f,
                                              kConnectionPanelY + 260.f),
        rtype::games::rtype::client::Rectangle({200, 60}, sf::Color(0, 150, 0),
                                               sf::Color(0, 200, 0)),
        std::function<void()>([this, switchToScene]() {
            this->_onConnectClicked(switchToScene);
        })));
}

void MainMenuScene::_onConnectClicked(
    std::function<void(const SceneManager::Scene&)> switchToScene) {
    if (!_networkClient) {
        _updateStatus("Error: Network not available", sf::Color::Red);
        return;
    }
    std::string ip = "127.0.0.1";
    std::uint16_t port = 4242;

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
    _networkClient->onConnected([this, switchToScene](std::uint32_t userId) {
        LOG_INFO("[Client] Connected with user ID: " + std::to_string(userId));
        _updateStatus("Connected! Starting game...", sf::Color::Green);
        try {
            switchToScene(SceneManager::IN_GAME);
        } catch (SceneNotFound& e) {
            LOG_ERROR(std::string("Error switching to Game: ") + e.what());
        }
    });

    _networkClient->onDisconnected(
        [this](rtype::client::NetworkClient::DisconnectReason reason) {
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
            _updateStatus(reasonStr, sf::Color::Red);
        });
    if (!_networkClient->connect(ip, port)) {
        _updateStatus("Failed to start connection", sf::Color::Red);
    }
}

void MainMenuScene::_updateStatus(const std::string& message, sf::Color color) {
    if (_registry->hasComponent<rtype::games::rtype::client::Text>(
            _statusEntity)) {
        auto& text = _registry->getComponent<rtype::games::rtype::client::Text>(
            _statusEntity);
        text.textContent = message;
        text.text.setString(message);
        text.text.setFillColor(color);
    }
}

void MainMenuScene::update() {
    if (_networkClient && !_networkClient->isConnected()) {
        _networkClient->poll();
    }
}

void MainMenuScene::render(std::shared_ptr<sf::RenderWindow> window) {
    auto view = _registry->view<rtype::games::rtype::client::TextInput,
                                rtype::games::rtype::shared::Position,
                                rtype::games::rtype::client::TextInputTag>();

    view.each([window](auto, auto& input, auto& pos, auto) {
        input.background.setPosition({pos.x, pos.y});
        input.text.setPosition({pos.x + 10.f, pos.y + 5.f});
        window->draw(input.background);
        window->draw(input.text);
    });
}

void MainMenuScene::pollEvents(const sf::Event& e) {
    if (auto* mousePressed = e.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left) {
            float mouseX = static_cast<float>(mousePressed->position.x);
            float mouseY = static_cast<float>(mousePressed->position.y);
            auto view =
                _registry->view<rtype::games::rtype::client::TextInput,
                                rtype::games::rtype::shared::Position,
                                rtype::games::rtype::client::TextInputTag>();

            view.each([mouseX, mouseY](auto, auto& input, auto& pos, auto) {
                sf::FloatRect bounds({pos.x, pos.y},
                                     {input.background.getSize().x,
                                      input.background.getSize().y});
                input.setFocus(bounds.contains({mouseX, mouseY}));
            });
        }
    }
    if (auto* textEntered = e.getIf<sf::Event::TextEntered>()) {
        auto view =
            _registry->view<rtype::games::rtype::client::TextInput,
                            rtype::games::rtype::client::TextInputTag>();

        view.each([textEntered](auto, auto& input, auto) {
            if (input.isFocused && textEntered->unicode >= 32 &&
                textEntered->unicode < 127) {
                input.handleTextInput(static_cast<char>(textEntered->unicode));
            }
        });
    }
    if (auto* keyPressed = e.getIf<sf::Event::KeyPressed>()) {
        auto view =
            _registry->view<rtype::games::rtype::client::TextInput,
                            rtype::games::rtype::client::TextInputTag>();

        view.each([keyPressed](auto, auto& input, auto) {
            if (!input.isFocused) return;

            if (keyPressed->code == sf::Keyboard::Key::Backspace) {
                input.handleBackspace();
            } else if (keyPressed->code == sf::Keyboard::Key::Escape) {
                input.setFocus(false);
            }
        });
    }
}

MainMenuScene::MainMenuScene(
    std::shared_ptr<ECS::Registry> ecs,
    std::shared_ptr<AssetManager> assetsManager,
    std::shared_ptr<sf::RenderWindow> window,
    std::function<void(const SceneManager::Scene&)> switchToScene,
    std::shared_ptr<rtype::client::NetworkClient> networkClient,
    std::shared_ptr<rtype::client::ClientNetworkSystem> networkSystem)
    : AScene(ecs, assetsManager, window),
      _networkClient(std::move(networkClient)),
      _networkSystem(std::move(networkSystem)) {
    this->_listEntity = (EntityFactory::createBackground(
        this->_registry, this->_assetsManager, "R-TYPE"));
    this->_createAstroneerVessel();
    this->_createFakePlayer();
    this->_createConnectionPanel(switchToScene);
    this->_listEntity.push_back(EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("title_font"),
            sf::Color::White, 36, "Solo (Offline)"),
        rtype::games::rtype::shared::Position(100, 350),
        rtype::games::rtype::client::Rectangle({400, 75}, sf::Color::Blue,
                                               sf::Color::Red),
        std::function<void()>([switchToScene]() {
            try {
                switchToScene(SceneManager::IN_GAME);
            } catch (SceneNotFound& e) {
                LOG_ERROR(std::string("Error switching to Game Menu: ") +
                          e.what());
            }
        })));
    this->_listEntity.push_back(EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("title_font"),
            sf::Color::White, 36, "Settings"),
        rtype::games::rtype::shared::Position(100, 460),
        rtype::games::rtype::client::Rectangle({400, 75}, sf::Color::Blue,
                                               sf::Color::Red),
        std::function<void()>([switchToScene]() {
            try {
                switchToScene(SceneManager::SETTINGS_MENU);
            } catch (SceneNotFound& e) {
                LOG_ERROR(std::string("Error switching to Settings Menu: ") +
                          e.what());
            }
        })));
    this->_listEntity.push_back(EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("title_font"),
            sf::Color::White, 36, "Quit"),
        rtype::games::rtype::shared::Position(100, 570),
        rtype::games::rtype::client::Rectangle({400, 75}, sf::Color::Blue,
                                               sf::Color::Red),
        std::function<void()>([this]() { this->_window->close(); })));
}
