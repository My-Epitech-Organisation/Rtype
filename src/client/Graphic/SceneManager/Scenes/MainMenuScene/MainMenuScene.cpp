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

static constexpr float kConnectionPanelX = 750.f;
static constexpr float kConnectionPanelY = 350.f;
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

    for (auto& s : panelEntities) {
        if (this->_registry
                ->hasComponent<rtype::games::rtype::client::Rectangle>(s))
            this->_registry
                ->emplaceComponent<rtype::games::rtype::client::ZIndex>(s, 3);
    }
    panelEntities.push_back(EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "Connect to Server",
        "title_font",
        sf::Vector2f(kConnectionPanelX + 50.f, kConnectionPanelY + 20.f), 32));
    panelEntities.push_back(EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "IP:", "main_font",
        sf::Vector2f(kConnectionPanelX + kLabelOffsetX,
                     kConnectionPanelY + 90.f),
        24));
    this->_ipInputEntity = EntityFactory::createTextInput(
        this->_registry, this->_assetsManager,
        sf::FloatRect(sf::Vector2f(kConnectionPanelX + kInputOffsetX,
                                   kConnectionPanelY + 85.f),
                      sf::Vector2f(kInputWidth, kInputHeight)),
        "127.0.0.1", "127.0.0.1", 15, false);
    panelEntities.push_back(this->_ipInputEntity);
    panelEntities.push_back(EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "Port:", "main_font",
        sf::Vector2f(kConnectionPanelX + kLabelOffsetX,
                     kConnectionPanelY + 150.f),
        24));
    this->_portInputEntity = EntityFactory::createTextInput(
        this->_registry, this->_assetsManager,
        sf::FloatRect(sf::Vector2f(kConnectionPanelX + kInputOffsetX,
                                   kConnectionPanelY + 145.f),
                      sf::Vector2f(kInputWidth, kInputHeight)),
        "4242", "4242", 5, true);
    panelEntities.push_back(_portInputEntity);
    this->_statusEntity = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "", "main_font",
        sf::Vector2f(kConnectionPanelX + kLabelOffsetX,
                     kConnectionPanelY + 200.f),
        18);
    panelEntities.push_back(this->_statusEntity);
    panelEntities.push_back(EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("main_font"),
            sf::Color::White, 28, "Connect"),
        rtype::games::rtype::shared::Position(kConnectionPanelX + 15.f,
                                              kConnectionPanelY + 275.f),
        rtype::games::rtype::client::Rectangle({200, 60}, sf::Color(0, 150, 0),
                                               sf::Color(0, 200, 0)),
        this->_assetsManager, std::function<void()>([this, switchToScene]() {
            this->_onConnectClicked(switchToScene);
        })));
    panelEntities.push_back(EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("main_font"),
            sf::Color::White, 26, "Close"),
        rtype::games::rtype::shared::Position(kConnectionPanelX + 235.f,
                                              kConnectionPanelY + 275.f),
        rtype::games::rtype::client::Rectangle({200, 60}, sf::Color(150, 0, 0),
                                               sf::Color(200, 0, 0)),
        this->_assetsManager, std::function<void()>([this]() {
            this->_connectPopUpVisible = false;
        })));
    for (auto& s : panelEntities) {
        if (!this->_registry->hasComponent<rtype::games::rtype::client::ZIndex>(
                s))
            this->_registry
                ->emplaceComponent<rtype::games::rtype::client::ZIndex>(s, 4);
        this->_registry
            ->emplaceComponent<rtype::games::rtype::client::HiddenComponent>(
                s, true);
        this->_registry
            ->emplaceComponent<rtype::games::rtype::client::ConnectMenuTag>(s);
    }
    auto popUpBg = EntityFactory::createRectangle(
        this->_registry, sf::Vector2i(1920, 1080), sf::Color(0, 0, 0, 150));

    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        popUpBg, 2);
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::ConnectMenuTag>(
            popUpBg);
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::HiddenComponent>(
            popUpBg, true);
    panelEntities.push_back(popUpBg);
    this->_listEntity.insert(this->_listEntity.end(), panelEntities.begin(),
                             panelEntities.end());
}

void MainMenuScene::_onConnectClicked(
    std::function<void(const SceneManager::Scene&)> switchToScene) {
    std::cout << "Connect button clicked\n";
    if (!this->_networkClient) {
        this->_updateStatus("Error: Network not available", sf::Color::Red);
        return;
    }
x    std::string ip = kIp;
    std::uint16_t port = kPort;

    if (this->_registry->hasComponent<rtype::games::rtype::client::TextInput>(
            this->_ipInputEntity)) {
        auto& ipInput =
            this->_registry
                ->getComponent<rtype::games::rtype::client::TextInput>(
                    this->_ipInputEntity);
        if (!ipInput.content.empty()) {
            ip = ipInput.content;
        }
    }

    if (this->_registry->hasComponent<rtype::games::rtype::client::TextInput>(
            this->_portInputEntity)) {
        auto& portInput =
            this->_registry
                ->getComponent<rtype::games::rtype::client::TextInput>(
                    this->_portInputEntity);
        if (!portInput.content.empty()) {
            try {
                port = static_cast<std::uint16_t>(std::stoi(portInput.content));
            } catch (...) {
                this->_updateStatus("Invalid port number", sf::Color::Red);
                return;
            }
        }
    }
    std::cout << "Connecting to " << ip << ":" << port << "...\n";
    this->_updateStatus(
        "Connecting to " + ip + ":" + std::to_string(port) + "...",
        sf::Color::Yellow);
    if (!this->_networkClient->connect(ip, port)) {
        this->_connectPopUpVisible = true;
        this->_updateStatus("Failed to start connection", sf::Color::Red);
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

void MainMenuScene::update(float dt) {
    if (_networkClient && !_networkClient->isConnected()) {
        _networkClient->poll();
    }
    if (this->_connectPopUpVisible) {
        auto view =
            _registry->view<rtype::games::rtype::client::ConnectMenuTag,
                            rtype::games::rtype::client::HiddenComponent>();

        view.each([](auto, auto&, auto& hidden) { hidden.isHidden = false; });
    } else {
        auto view =
            _registry->view<rtype::games::rtype::client::ConnectMenuTag,
                            rtype::games::rtype::client::HiddenComponent>();

        view.each([](auto, auto&, auto& hidden) { hidden.isHidden = true; });
    }
    this->_registry
        ->view<rtype::games::rtype::client::UserEvent,
               rtype::games::rtype::client::ButtonMenuTag>()
        .each(
            [this](auto, rtype::games::rtype::client::UserEvent& event, auto) {
                if (this->_connectPopUpVisible)
                    event.isDisabled = true;
                else
                    event.isDisabled = false;
            });
}

void MainMenuScene::render(std::shared_ptr<sf::RenderWindow> window) {}

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
    std::shared_ptr<rtype::client::ClientNetworkSystem> networkSystem,
    std::shared_ptr<AudioLib> audioLib)
    : AScene(ecs, assetsManager, window, audioLib),
      _networkClient(std::move(networkClient)),
      _networkSystem(std::move(networkSystem)) {
    this->_listEntity = (EntityFactory::createBackground(
        this->_registry, this->_assetsManager, "R-TYPE"));
    this->_createAstroneerVessel();
    this->_createFakePlayer();
    this->_createConnectionPanel(switchToScene);
    auto btnPlay = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("main_font"),
            sf::Color::White, 36, "Play"),
        rtype::games::rtype::shared::Position(100, 350),
        rtype::games::rtype::client::Rectangle({400, 75}, sf::Color::Blue,
                                               sf::Color::Red),
        this->_assetsManager, std::function<void()>([this, switchToScene]() {
            try {
                this->_connectPopUpVisible = true;
            } catch (SceneNotFound& e) {
                LOG_ERROR(std::string("Error switching to Game Menu: ") +
                          std::string(e.what()));
            }
        }));
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        btnPlay, 1);
    this->_listEntity.push_back(btnPlay);
    auto btnHowPlay = EntityFactory::createButton(
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
        btnHowPlay, 1);
    this->_listEntity.push_back(btnHowPlay);
    auto btnSettings = EntityFactory::createButton(
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
        btnSettings, 1);
    this->_listEntity.push_back(btnSettings);
    auto btnQuit = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("main_font"),
            sf::Color::White, 36, "Quit"),
        rtype::games::rtype::shared::Position(100, 710),
        rtype::games::rtype::client::Rectangle({400, 75}, sf::Color::Blue,
                                               sf::Color::Red),
        this->_assetsManager,
        std::function<void()>([this]() { this->_window->close(); })

    );
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        btnQuit, 1);
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::ButtonMenuTag>(btnQuit);
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::ButtonMenuTag>(btnPlay);
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::ButtonMenuTag>(
            btnHowPlay);
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::ButtonMenuTag>(
            btnSettings);
    this->_listEntity.push_back(btnQuit);
    this->_assetsManager->audioManager->load(
        "main_menu_music",
        this->_assetsManager->configGameAssets.assets.music.mainMenu);
    auto bgMusic = this->_assetsManager->audioManager->get("main_menu_music");
    this->_audio->loadMusic(bgMusic);
    this->_audio->setLoop(true);
    this->_audio->play();
    this->_networkClient->onConnected([this,
                                       switchToScene](std::uint32_t userId) {
        LOG_INFO("[Client] Connected with user ID: " + std::to_string(userId));
        this->_updateStatus("Connected! Starting game...", sf::Color::Green);
        try {
            switchToScene(SceneManager::LOBBY);
            this->_connectPopUpVisible = false;
        } catch (SceneNotFound& e) {
            LOG_ERROR(std::string("Error switching to Game: ") +
                      std::string(e.what()));
        }
    });

    this->_networkClient->onDisconnected(
        [this](rtype::client::NetworkClient::DisconnectReason reason) {
            std::string reasonStr;
            this->_connectPopUpVisible = true;
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
}

MainMenuScene::~MainMenuScene() {
    if (_networkClient) {
        _networkClient->onConnected([](std::uint32_t) {});
        _networkClient->onDisconnected([](rtype::network::DisconnectReason) {});
    }
}
