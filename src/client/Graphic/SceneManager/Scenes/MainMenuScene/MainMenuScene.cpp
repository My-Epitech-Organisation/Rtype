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

#include "AllComponents.hpp"
#include "Components/TextInputComponent.hpp"
#include "EntityFactory/EntityFactory.hpp"
#include "Logger/Macros.hpp"
#include "SceneManager/SceneException.hpp"
#include "Scenes/Lobby/Lobby.hpp"

static constexpr float kConnectionPanelX = 750.f;
static constexpr float kConnectionPanelY = 350.f;
static constexpr float kConnectionPanelWidth = 450.f;
static constexpr float kConnectionPanelHeight = 350.f;
static constexpr float kInputWidth = 300.f;
static constexpr float kInputHeight = 40.f;
static constexpr float kInputOffsetX = 120.f;
static constexpr std::string kIp = "127.0.0.1";
static constexpr std::uint16_t kPort = 4242;

void MainMenuScene::_createAstroneerVessel() {
    auto astroneerVessel = this->_registry->spawnEntity();
    this->_registry->emplaceComponent<rtype::games::rtype::client::Image>(
        astroneerVessel, "astro_vessel");
    this->_registry
        ->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
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
            fakePlayer, "player_vessel");
        this->_registry
            ->emplaceComponent<rtype::games::rtype::client::TextureRect>(
                fakePlayer, std::pair<int, int>({0, 0}),
                std::pair<int, int>({33, 17}));
        this->_registry
            ->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
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
        rtype::display::Rect<float>(kConnectionPanelX, kConnectionPanelY,
                                    kConnectionPanelWidth,
                                    kConnectionPanelHeight));

    for (auto& s : panelEntities) {
        if (this->_registry
                ->hasComponent<rtype::games::rtype::client::Rectangle>(s))
            this->_registry
                ->emplaceComponent<rtype::games::rtype::client::ZIndex>(s, 10);
    }
    auto connectText = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "Connect to Server",
        "title_font",
        rtype::display::Vector2<float>(kConnectionPanelX + 40.f,
                                       kConnectionPanelY + 40.f),
        32);
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::SectionItemTag>(
            connectText);
    panelEntities.push_back(connectText);
    auto ipText = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "IP:", "main_font",
        rtype::display::Vector2<float>(kConnectionPanelX + 40.f,
                                       kConnectionPanelY + 105.f),
        24);
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::SectionItemTag>(ipText);
    panelEntities.push_back(ipText);
    this->_ipInputEntity = EntityFactory::createTextInput(
        this->_registry, this->_assetsManager,
        rtype::display::Vector2<float>(kConnectionPanelX + 120.f,
                                       kConnectionPanelY + 85.f),
        rtype::display::Vector2<float>(kInputWidth, kInputHeight), "127.0.0.1",
        "127.0.0.1", 15, false);
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::SectionItemTag>(
            this->_ipInputEntity);
    panelEntities.push_back(this->_ipInputEntity);
    auto portText = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "Port:", "main_font",
        rtype::display::Vector2<float>(kConnectionPanelX + 40.f,
                                       kConnectionPanelY + 165.f),
        24);
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::SectionItemTag>(
            portText);
    panelEntities.push_back(portText);
    this->_portInputEntity = EntityFactory::createTextInput(
        this->_registry, this->_assetsManager,
        rtype::display::Vector2<float>(kConnectionPanelX + 120.f,
                                       kConnectionPanelY + 145.f),
        rtype::display::Vector2<float>(kInputWidth, kInputHeight), "4242",
        "4242", 5, true);
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::SectionItemTag>(
            this->_portInputEntity);
    panelEntities.push_back(this->_portInputEntity);
    this->_statusEntity = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "", "main_font",
        rtype::display::Vector2<float>(
            kConnectionPanelX + kInputOffsetX / 2,
            kConnectionPanelY + 200.f + kInputHeight / 2),
        18);
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::SectionItemTag>(
            this->_statusEntity);
    panelEntities.push_back(this->_statusEntity);
    auto connectBtn = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            "main_font", rtype::display::Color::White(), 28, "Connect"),
        rtype::games::rtype::shared::TransformComponent(
            kConnectionPanelX + 15.f, kConnectionPanelY + 275.f),
        rtype::games::rtype::client::Rectangle(
            {200, 60}, rtype::display::Color(0, 150, 0, 255),
            rtype::display::Color(0, 200, 0, 255)),
        this->_assetsManager, std::function<void()>([this, switchToScene]() {
            this->_onConnectClicked(switchToScene);
        }));
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::SectionItemTag>(
            connectBtn);
    panelEntities.push_back(connectBtn);
    auto closeBtn = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            "main_font", rtype::display::Color::White(), 26, "Close"),
        rtype::games::rtype::shared::TransformComponent(
            kConnectionPanelX + 235.f, kConnectionPanelY + 275.f),
        rtype::games::rtype::client::Rectangle(
            {200, 60}, rtype::display::Color(150, 0, 0),
            rtype::display::Color(200, 0, 0)),
        this->_assetsManager, std::function<void()>([this]() {
            this->_connectPopUpVisible = false;
        }));
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::SectionItemTag>(
            closeBtn);
    panelEntities.push_back(closeBtn);
    for (auto& s : panelEntities) {
        if (!this->_registry->hasComponent<rtype::games::rtype::client::ZIndex>(
                s)) {
            if (this->_registry
                    ->hasComponent<rtype::games::rtype::client::SectionItemTag>(
                        s)) {
                this->_registry
                    ->emplaceComponent<rtype::games::rtype::client::ZIndex>(s,
                                                                            11);
            } else {
                this->_registry
                    ->emplaceComponent<rtype::games::rtype::client::ZIndex>(s,
                                                                            10);
            }
        } else {
            auto& zindex =
                this->_registry
                    ->getComponent<rtype::games::rtype::client::ZIndex>(s);
            if (this->_registry
                    ->hasComponent<rtype::games::rtype::client::SectionItemTag>(
                        s))
                zindex.depth = 11;
            else
                zindex.depth = 10;
        }
        this->_registry
            ->emplaceComponent<rtype::games::rtype::client::HiddenComponent>(
                s, true);
        this->_registry
            ->emplaceComponent<rtype::games::rtype::client::ConnectMenuTag>(s);
    }
    auto popUpBg = EntityFactory::createRectangle(
        this->_registry, rtype::display::Vector2<int>(1920, 1080),
        rtype::display::Color(0, 0, 0, 150));

    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        popUpBg, 10);
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
    if (!this->_networkClient) {
        this->_updateStatus("Error: Network not available",
                            rtype::display::Color::Red());
        return;
    }
    std::string ip = kIp;
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
                this->_updateStatus("Invalid port number",
                                    rtype::display::Color::Red());
                return;
            }
        }
    }

    _updateStatus("Connecting to " + ip + ":" + std::to_string(port) + "...",
                  rtype::display::Color{255, 255, 0, 255});
    std::weak_ptr<ECS::Registry> weakRegistry = _registry;
    ECS::Entity statusEntity = _statusEntity;

    if (_networkClient) {
        for (auto id : _connectedCallbackIds) {
            _networkClient->removeConnectedCallback(id);
        }
        _connectedCallbackIds.clear();
        for (auto id : _disconnectedCallbackIds) {
            _networkClient->removeDisconnectedCallback(id);
        }
        _disconnectedCallbackIds.clear();
    }

    auto onConnectedId = _networkClient->addConnectedCallback(
        [weakRegistry, switchToScene, statusEntity](std::uint32_t userId) {
            auto reg = weakRegistry.lock();
            if (!reg) return;

            LOG_INFO_CAT(::rtype::LogCategory::UI,
                         "[Client] Connected with user ID: " << userId);

            if (reg->isAlive(statusEntity) &&
                reg->hasComponent<rtype::games::rtype::client::Text>(
                    statusEntity)) {
                auto& text =
                    reg->getComponent<rtype::games::rtype::client::Text>(
                        statusEntity);
                text.textContent = "Connected! Entering lobby...";
                text.textContent = "Connected! Entering lobby...";
                text.color = rtype::display::Color::Green();
            }

            try {
                switchToScene(SceneManager::LOBBY);
            } catch (SceneNotFound& e) {
                LOG_ERROR_CAT(::rtype::LogCategory::UI,
                              std::string("Error switching to Lobby: ") +
                                  std::string(e.what()));
            }
        });
    _connectedCallbackIds.push_back(onConnectedId);

    auto onDisconnectedId = _networkClient->addDisconnectedCallback(
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
                text.textContent = reasonStr;
                text.color = rtype::display::Color::Red();
            }
        });
    _disconnectedCallbackIds.push_back(onDisconnectedId);
    if (!_networkClient->connect(ip, port)) {
        this->_connectPopUpVisible = true;
        _updateStatus("Failed to start connection",
                      rtype::display::Color::Red());
    }
}

void MainMenuScene::_updateStatus(const std::string& message,
                                  rtype::display::Color color) {
    if (!_registry) return;
    if (!_registry->isAlive(_statusEntity)) return;
    if (!_registry->hasComponent<rtype::games::rtype::client::Text>(
            _statusEntity)) {
        return;
    }

    auto& text = _registry->getComponent<rtype::games::rtype::client::Text>(
        _statusEntity);
    text.textContent = message;
    text.textContent = message;
    text.color = color;
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

void MainMenuScene::render(std::shared_ptr<rtype::display::IDisplay> window) {
}

void MainMenuScene::pollEvents(const rtype::display::Event& e) {
    if (_textInputSystem) {
        _textInputSystem->handleEvent(*_registry, e);
    }
}

MainMenuScene::MainMenuScene(
    std::shared_ptr<ECS::Registry> ecs,
    std::shared_ptr<AssetManager> assetsManager,
    std::shared_ptr<rtype::display::IDisplay> window,
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
            "main_font", rtype::display::Color::White(), 36, "Play"),
        rtype::games::rtype::shared::TransformComponent(100, 350),
        rtype::games::rtype::client::Rectangle({400, 75},
                                               rtype::display::Color::Blue(),
                                               rtype::display::Color::Red()),
        this->_assetsManager,
        std::function<void()>([this]() { this->_connectPopUpVisible = true; }));
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        playBtn, 1);
    this->_listEntity.push_back(playBtn);
    auto howToPlayBtn = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            "main_font", rtype::display::Color::White(), 36, "How to Play"),
        rtype::games::rtype::shared::TransformComponent(100, 470),
        rtype::games::rtype::client::Rectangle({400, 75},
                                               rtype::display::Color::Blue(),
                                               rtype::display::Color::Red()),
        this->_assetsManager, std::function<void()>([switchToScene]() {
            try {
                switchToScene(SceneManager::HOW_TO_PLAY);
            } catch (SceneNotFound& e) {
                LOG_ERROR_CAT(::rtype::LogCategory::UI,
                              std::string("Error switching to How To Play: ") +
                                  std::string(e.what()));
            }
        }));
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        howToPlayBtn, 1);
    this->_listEntity.push_back(howToPlayBtn);
    auto settingsBtn = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            "main_font", rtype::display::Color::White(), 36, "Settings"),
        rtype::games::rtype::shared::TransformComponent(100, 590),
        rtype::games::rtype::client::Rectangle({400, 75},
                                               rtype::display::Color::Blue(),
                                               rtype::display::Color::Red()),
        this->_assetsManager, std::function<void()>([switchToScene]() {
            try {
                switchToScene(SceneManager::SETTINGS_MENU);
            } catch (SceneNotFound& e) {
                LOG_ERROR_CAT(
                    ::rtype::LogCategory::UI,
                    std::string("Error switching to Settings Menu: ") +
                        std::string(e.what()));
            }
        }));
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        settingsBtn, 1);
    this->_listEntity.push_back(settingsBtn);
    auto quitBtn = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            "main_font", rtype::display::Color::White(), 36, "Quit"),
        rtype::games::rtype::shared::TransformComponent(100, 710),
        rtype::games::rtype::client::Rectangle({400, 75},
                                               rtype::display::Color::Blue(),
                                               rtype::display::Color::Red()),
        this->_assetsManager,
        std::function<void()>([window = this->_window]() { window->close(); }));
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        quitBtn, 1);
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::ButtonMenuTag>(quitBtn);
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::ButtonMenuTag>(playBtn);
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::ButtonMenuTag>(
            howToPlayBtn);
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::ButtonMenuTag>(
            settingsBtn);
    this->_listEntity.push_back(quitBtn);
    this->_assetsManager->audioManager->load(
        "main_menu_music",
        this->_assetsManager->configGameAssets.assets.music.mainMenu);
    auto bgMusic = this->_assetsManager->audioManager->get("main_menu_music");
    this->_audio->loadMusic(bgMusic);
    this->_audio->setLoop(true);
    this->_audio->play();
    auto menuOnConnectedId = this->_networkClient->addConnectedCallback(
        [this, switchToScene](std::uint32_t userId) {
            LOG_INFO("[Client] Connected with user ID: " +
                     std::to_string(userId));
            this->_updateStatus("Connected! Starting game...",
                                rtype::display::Color::Green());
            try {
                switchToScene(SceneManager::LOBBY);
                this->_connectPopUpVisible = false;
            } catch (SceneNotFound& e) {
                LOG_ERROR(std::string("Error switching to Game: ") +
                          std::string(e.what()));
            }
        });
    _connectedCallbackIds.push_back(menuOnConnectedId);

    auto menuOnDisconnectedId = this->_networkClient->addDisconnectedCallback(
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
            _updateStatus(reasonStr, rtype::display::Color::Red());
        });
    _disconnectedCallbackIds.push_back(menuOnDisconnectedId);
}

MainMenuScene::~MainMenuScene() {
    if (_networkClient) {
        for (auto id : _connectedCallbackIds) {
            _networkClient->removeConnectedCallback(id);
        }
        for (auto id : _disconnectedCallbackIds) {
            _networkClient->removeDisconnectedCallback(id);
        }
    }
}
