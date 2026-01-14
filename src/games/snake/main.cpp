/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Snake Game - Playable executable with IDisplay abstraction
*/

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <utility>

#include "include/rtype/display/IDisplay.hpp"
#include "lib/common/src/DLLoader/DLLoader.hpp"
#include "server/GameEngine.hpp"
#include "shared/Components.hpp"
#include "src/client/network/NetworkClient.hpp"

using rtype::display::Event;
using rtype::display::EventType;
using rtype::display::IDisplay;
using rtype::display::Key;

namespace server = rtype::games::snake::server;
namespace shared = rtype::games::snake::shared;

const int GRID_WIDTH = rtype::games::snake::server::SnakeGameConfig::GRID_WIDTH;
const int GRID_HEIGHT =
    rtype::games::snake::server::SnakeGameConfig::GRID_HEIGHT;
const int CELL_SIZE =
    static_cast<int>(rtype::games::snake::server::SnakeGameConfig::CELL_SIZE);
const int WINDOW_WIDTH = GRID_WIDTH * CELL_SIZE;
const int WINDOW_HEIGHT = GRID_HEIGHT * CELL_SIZE;

std::string findDisplayLibrary(const std::string& preferredLib) {
    std::vector<std::string> searchPaths = {
        "./display.so",
        "../lib/display.so",
        "./build/lib/display/display.so",
    };

    if (!preferredLib.empty()) {
        if (std::filesystem::exists(preferredLib)) {
            return preferredLib;
        }
    }

    for (const auto& path : searchPaths) {
        if (std::filesystem::exists(path)) {
            return path;
        }
    }

    return "";
}

shared::Direction keyToDirection(Key key) {
    switch (key) {
        case Key::Up:
        case Key::W:
            return shared::Direction::UP;
        case Key::Down:
        case Key::S:
            return shared::Direction::DOWN;
        case Key::Left:
        case Key::A:
            return shared::Direction::LEFT;
        case Key::Right:
        case Key::D:
            return shared::Direction::RIGHT;
        default:
            return shared::Direction::NONE;
    }
}

static int runApplication(IDisplay* display) {  // NOLINT(readability/fn_size)
    auto createNetworkConfig = []() {
        rtype::client::NetworkClient::Config cfg;
        cfg.connectionConfig.reliabilityConfig.retransmitTimeout =
            std::chrono::milliseconds(1000);
        cfg.connectionConfig.reliabilityConfig.maxRetries = 15;
        return cfg;
    };

    std::unique_ptr<server::SnakeGameEngine> engine;
    std::shared_ptr<ECS::Registry> registry;

    display->open(WINDOW_WIDTH, WINDOW_HEIGHT,
                  "Snake - Graphics Abstraction Demo", false);
    display->setFramerateLimit(60);

    display->loadFont("main", "assets/fonts/Orbitron-VariableFont_wght.ttf");

    std::cout << "✓ Display window opened\n\n";

    enum class AppMode {
        Menu,
        SingleplayerRun,
        MultiplayerInput,
        MultiplayerLobby,
        Playing
    };
    AppMode mode = AppMode::Menu;
    int menuIndex = 0;

    std::string mpHost = "127.0.0.1";
    std::string mpPortStr = "4242";
    int inputField = 0;

    std::shared_ptr<rtype::client::NetworkClient> netClient;
    bool netConnected = false;
    bool joinedLobby = false;
    bool isMultiplayerMode = false;
    std::unordered_map<uint32_t, bool> lobbyReadyStates;
    std::unordered_map<uint32_t, ECS::Entity> remoteEntities;
    std::optional<uint32_t> myUserId;

    auto joinLobby = [&]() {
        if (!netClient) return;
        if (!netClient->sendJoinLobby("")) {
            std::cerr << "✗ Failed to send join lobby\n";
        }
    };

    std::cout << "═══════════════════════════════════════\n";
    std::cout << "    SNAKE GAME - Graphics Abstraction\n";
    std::cout << "      Library: " << display->getLibName() << "\n";
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "Controls:\n";
    std::cout << "  ↑ W / ↓ S / ← A / → D   Move\n";
    std::cout << "  ESC                      Quit\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "═══════════════════════════════════════\n";
    std::cout << "    SNAKE GAME - Graphics Abstraction\n";
    std::cout << "      Library: " << display->getLibName() << "\n";
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "Controls:\n";
    std::cout << "  ↑ W / ↓ S / ← A / → D   Move\n";
    std::cout << "  ESC                      Quit\n";
    std::cout << "═══════════════════════════════════════\n\n";

    auto lastTime = std::chrono::high_resolution_clock::now();
    bool running = true;
    bool showGameOver = false;
    bool announcedGameOver = false;
    float countdownTimer = 0.0f;  // For displaying countdown during game start
    int frameCount = 0;
    auto lastFpsTime = lastTime;
    float accumulator = 0.0f;
    const float FRAME_TIME = 0.0166f;

    const int buttonWidth = 180;
    const int buttonHeight = 50;
    const int buttonX = (WINDOW_WIDTH - buttonWidth) / 2;
    const int buttonY = (WINDOW_HEIGHT / 2) + 20;

    while (running && display->isOpen()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto deltaSeconds =
            std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        Event event;
        while (display->pollEvent(event)) {
            if (event.type == EventType::Closed) {
                running = false;
                break;
            }

            if (mode == AppMode::Menu) {
                if (event.type == EventType::KeyPressed) {
                    if (event.key.code == Key::Escape) {
                        running = false;
                        break;
                    } else if (event.key.code == Key::Up) {
                        menuIndex = std::max(0, menuIndex - 1);
                    } else if (event.key.code == Key::Down) {
                        menuIndex = std::min(1, menuIndex + 1);
                    } else if (event.key.code == Key::Return) {
                        if (menuIndex == 0) {
                            mode = AppMode::SingleplayerRun;
                            registry = std::make_shared<ECS::Registry>();
                            engine = std::make_unique<server::SnakeGameEngine>(
                                registry);
                            if (!engine->initialize()) {
                                std::cerr << "✗ Failed to initialize game "
                                             "engine\n";
                                running = false;
                                break;
                            }
                            std::cout << "✓ Game Engine initialized\n";
                            mode = AppMode::Playing;
                        } else {
                            mode = AppMode::MultiplayerInput;
                            inputField = 0;
                        }
                    }
                } else if (event.type == EventType::MouseButtonPressed) {
                    int mx = event.mouseButton.x;
                    int my = event.mouseButton.y;
                    int singleY = WINDOW_HEIGHT / 2 - 20;
                    int multiY = WINDOW_HEIGHT / 2 + 40;
                    if (mx >= buttonX && mx <= buttonX + buttonWidth) {
                        if (my >= singleY && my <= singleY + buttonHeight) {
                            menuIndex = 0;
                            mode = AppMode::SingleplayerRun;
                            registry = std::make_shared<ECS::Registry>();
                            engine = std::make_unique<server::SnakeGameEngine>(
                                registry);
                            if (!engine->initialize()) {
                                std::cerr << "✗ Failed to initialize game "
                                             "engine\n";
                                running = false;
                                break;
                            }
                            mode = AppMode::Playing;
                        } else if (my >= multiY &&
                                   my <= multiY + buttonHeight) {
                            menuIndex = 1;
                            mode = AppMode::MultiplayerInput;
                            inputField = 0;
                        }
                    }
                }
                continue;
            }

            if (mode == AppMode::MultiplayerInput) {
                auto doConnect = [&]() {
                    uint16_t port = 4242;
                    try {
                        port = static_cast<uint16_t>(std::stoi(mpPortStr));
                    } catch (...) {
                        std::cerr << "Invalid port" << std::endl;
                        return;
                    }

                    registry = std::make_shared<ECS::Registry>();
                    engine = std::make_unique<server::SnakeGameEngine>(registry,
                                                                       false);
                    if (!engine->initialize()) {
                        std::cerr << "✗ Failed to initialize game engine\n";
                        return;
                    }

                    netClient = std::make_shared<rtype::client::NetworkClient>(
                        createNetworkConfig());

                    netClient->onConnected([&](std::uint32_t myId) {
                        netConnected = true;
                        myUserId = myId;
                        lobbyReadyStates[myId] = false;
                        std::cout << "Connected, my id: " << myId << std::endl;
                        joinLobby();
                    });

                    netClient->onDisconnected(
                        [&](rtype::client::NetworkClient::
                                DisconnectReason /*reason*/) {
                            netConnected = false;
                            joinedLobby = false;
                            lobbyReadyStates.clear();
                            remoteEntities.clear();
                            myUserId.reset();
                            std::cout << "Disconnected from server\n";
                        });

                    netClient->onJoinLobbyResponse(
                        [&](bool accepted, uint8_t reason) {
                            if (accepted) {
                                joinedLobby = true;
                                mode = AppMode::MultiplayerLobby;
                                std::cout << "Joined lobby" << std::endl;
                            } else {
                                std::cerr << "Join lobby refused. Reason: "
                                          << int(reason) << std::endl;
                            }
                        });

                    netClient->onPlayerReadyStateChanged(
                        [&](uint32_t userId, bool isReady) {
                            lobbyReadyStates[userId] = isReady;
                        });

                    netClient->onEntitySpawn([&](rtype::client::EntitySpawnEvent
                                                     ev) {
                        ECS::Entity e = registry->spawnEntity();
                        remoteEntities[ev.entityId] = e;
                        if (ev.type == rtype::network::EntityType::Player) {
                            registry
                                ->emplaceComponent<shared::SnakeHeadComponent>(
                                    e, shared::SnakeHeadComponent{
                                           .playerId = ev.userId});
                            registry
                                ->emplaceComponent<shared::PositionComponent>(
                                    e, shared::PositionComponent{
                                           .gridX = static_cast<int>(ev.x),
                                           .gridY = static_cast<int>(ev.y)});
                            registry
                                ->emplaceComponent<shared::VelocityComponent>(
                                    e, shared::VelocityComponent{.dirX = 1,
                                                                 .dirY = 0});
                            registry->emplaceComponent<
                                shared::PlayerInputComponent>(
                                e,
                                shared::PlayerInputComponent{
                                    .playerId = ev.userId,
                                    .nextDirection = shared::Direction::NONE});
                        } else if (ev.type ==
                                   rtype::network::EntityType::Pickup) {
                            registry->emplaceComponent<shared::FoodComponent>(
                                e, shared::FoodComponent{.value = 10});
                            registry
                                ->emplaceComponent<shared::PositionComponent>(
                                    e, shared::PositionComponent{
                                           .gridX = static_cast<int>(ev.x),
                                           .gridY = static_cast<int>(ev.y)});
                        } else {
                            registry
                                ->emplaceComponent<shared::PositionComponent>(
                                    e, shared::PositionComponent{
                                           .gridX = static_cast<int>(ev.x),
                                           .gridY = static_cast<int>(ev.y)});
                        }
                    });

                    netClient->onEntityMoveBatch(
                        [&](rtype::client::EntityMoveBatchEvent batch) {
                            for (auto& mv : batch.entities) {
                                auto it = remoteEntities.find(mv.entityId);
                                if (it == remoteEntities.end()) continue;
                                auto& ent = it->second;
                                auto& pos = registry->getComponent<
                                    shared::PositionComponent>(ent);
                                pos.gridX = static_cast<int>(mv.x);
                                pos.gridY = static_cast<int>(mv.y);
                            }
                        });

                    netClient->onGameStart([&](float countdown) {
                        countdownTimer = countdown;
                        std::cout << "Server game starting in " << countdown
                                  << "s" << std::endl;
                        isMultiplayerMode = true;
                        mode = AppMode::Playing;
                    });

                    netClient->onGameOver([&](rtype::client::GameOverEvent ev) {
                        std::cout << "\n\u2717 GAME OVER! Final Score: "
                                  << ev.finalScore << "\n";
                        auto& gameState =
                            registry
                                ->getSingleton<shared::GameStateComponent>();
                        gameState.score = ev.finalScore;
                        gameState.isGameOver = true;
                        showGameOver = true;
                        announcedGameOver = true;
                    });

                    if (!netClient->connect(mpHost, port)) {
                        std::cerr << "Failed to initiate connection to "
                                  << mpHost << ":" << port << std::endl;
                    }
                };

                if (event.type == EventType::MouseButtonPressed) {
                    int mx = event.mouseButton.x;
                    int my = event.mouseButton.y;
                    float connectY = 250.0f;
                    if (mx >= buttonX && mx <= buttonX + buttonWidth &&
                        my >= static_cast<int>(connectY) &&
                        my <= static_cast<int>(connectY) + buttonHeight) {
                        doConnect();
                        continue;
                    }
                } else if (event.type == EventType::KeyPressed) {
                    if (event.key.code == Key::Escape) {
                        mode = AppMode::Menu;
                        continue;
                    } else if (event.key.code == Key::Tab) {
                        inputField = (inputField + 1) % 2;
                    } else if (event.key.code == Key::Return) {
                        doConnect();
                        continue;
                    }
                } else if (event.type == EventType::TextEntered) {
                    char c = static_cast<char>(event.text.unicode);
                    if (c == '\b') {
                        if (inputField == 0 && !mpHost.empty())
                            mpHost.pop_back();
                        else if (inputField == 1 && !mpPortStr.empty())
                            mpPortStr.pop_back();
                    } else if (c >= 32 && c < 127) {
                        if (inputField == 0)
                            mpHost.push_back(c);
                        else if (inputField == 1)
                            mpPortStr.push_back(c);
                    }
                }
                continue;
            }

            if (mode == AppMode::MultiplayerLobby) {
                if (event.type == EventType::KeyPressed) {
                    if (event.key.code == Key::Escape) {
                        if (netClient && netClient->isConnected())
                            netClient->disconnect();
                        mode = AppMode::Menu;
                        continue;
                    } else if (event.key.code == Key::Space) {
                        bool isReady = false;
                        if (myUserId && lobbyReadyStates.find(*myUserId) !=
                                            lobbyReadyStates.end())
                            isReady = !lobbyReadyStates[*myUserId];
                        netClient->sendReady(isReady);
                    }
                }
                continue;
            }

            if (event.type == EventType::KeyPressed) {
                if (event.key.code == Key::Escape) {
                    running = false;
                    break;
                }

                if (showGameOver && (event.key.code == Key::Return ||
                                     event.key.code == Key::R)) {
                    if (isMultiplayerMode) {
                        if (netClient && netClient->isConnected())
                            netClient->disconnect();
                        mode = AppMode::Menu;
                        showGameOver = false;
                        announcedGameOver = false;
                        isMultiplayerMode = false;
                    } else {
                        registry = std::make_shared<ECS::Registry>();
                        engine =
                            std::make_unique<server::SnakeGameEngine>(registry);
                        if (!engine->initialize()) {
                            std::cerr << "✗ Failed to initialize game engine\n";
                            running = false;
                            break;
                        }
                        showGameOver = false;
                        announcedGameOver = false;
                        engine->startGame();
                    }
                    continue;
                }

                if (!showGameOver) {
                    shared::Direction dir = keyToDirection(event.key.code);
                    if (dir != shared::Direction::NONE) {
                        if (netClient && netClient->isConnected() && myUserId) {
                            uint8_t mask = 0;
                            if (dir == shared::Direction::UP)
                                mask = rtype::network::InputMask::kUp;
                            else if (dir == shared::Direction::DOWN)
                                mask = rtype::network::InputMask::kDown;
                            else if (dir == shared::Direction::LEFT)
                                mask = rtype::network::InputMask::kLeft;
                            else if (dir == shared::Direction::RIGHT)
                                mask = rtype::network::InputMask::kRight;
                            netClient->sendInput(mask);
                        }

                        auto headView =
                            registry->view<shared::SnakeHeadComponent,
                                           shared::PlayerInputComponent>();
                        headView.each(
                            [dir](ECS::Entity /*id*/,
                                  shared::SnakeHeadComponent& /*head*/,
                                  shared::PlayerInputComponent& input) {
                                input.nextDirection = dir;
                            });
                    }
                }
            } else if (event.type == EventType::MouseButtonPressed &&
                       showGameOver) {
                int mx = event.mouseButton.x;
                int my = event.mouseButton.y;
                if (mx >= buttonX && mx <= buttonX + buttonWidth &&
                    my >= static_cast<int>(buttonY) &&
                    my <= static_cast<int>(buttonY) + buttonHeight) {
                    if (isMultiplayerMode) {
                        if (netClient && netClient->isConnected())
                            netClient->disconnect();
                        mode = AppMode::Menu;
                        showGameOver = false;
                        announcedGameOver = false;
                        isMultiplayerMode = false;
                    } else {
                        registry = std::make_shared<ECS::Registry>();
                        engine =
                            std::make_unique<server::SnakeGameEngine>(registry);
                        if (!engine->initialize()) {
                            std::cerr << "✗ Failed to initialize game engine\n";
                            running = false;
                            break;
                        }
                        showGameOver = false;
                        announcedGameOver = false;
                        engine->startGame();
                    }
                }
                continue;
            }
        }

        if (!running) break;

        if (netClient) netClient->poll();

        if (mode == AppMode::Menu) {
            display->clear({30, 30, 30, 255});
            display->drawText("Snake - Play", "main",
                              {WINDOW_WIDTH / 2 - 90.0f, 60.0f}, 28,
                              {255, 255, 255, 255});
            int singleY = WINDOW_HEIGHT / 2 - 20;
            int multiY = WINDOW_HEIGHT / 2 + 40;
            display->drawRectangle(
                {static_cast<float>(buttonX), static_cast<float>(singleY)},
                {static_cast<float>(buttonWidth),
                 static_cast<float>(buttonHeight)},
                menuIndex == 0 ? rtype::display::Color{80, 160, 80, 255}
                               : rtype::display::Color{60, 120, 200, 255},
                {20, 60, 120, 255}, 2);
            display->drawText(
                "Singleplayer", "main",
                {WINDOW_WIDTH / 2 - 55.0f, static_cast<float>(singleY) + 12.0f},
                20, {255, 255, 255, 255});
            display->drawRectangle(
                {static_cast<float>(buttonX), static_cast<float>(multiY)},
                {static_cast<float>(buttonWidth),
                 static_cast<float>(buttonHeight)},
                menuIndex == 1 ? rtype::display::Color{80, 160, 80, 255}
                               : rtype::display::Color{60, 120, 200, 255},
                {20, 60, 120, 255}, 2);
            display->drawText(
                "Multiplayer", "main",
                {WINDOW_WIDTH / 2 - 60.0f, static_cast<float>(multiY) + 12.0f},
                20, {255, 255, 255, 255});
            display->display();
            continue;
        }

        if (mode == AppMode::MultiplayerInput) {
            display->clear({25, 25, 35, 255});
            display->drawText("Multiplayer - Connect", "main",
                              {WINDOW_WIDTH / 2 - 120.0f, 40.0f}, 26,
                              {255, 255, 255, 255});
            float y = 130.0f;
            const float labelX = 80.0f;
            const float boxX = 200.0f;
            const float boxW = 280.0f;
            const float boxH = 32.0f;

            display->drawText("Host:", "main", {labelX, y + 6.0f}, 18,
                              {180, 180, 180, 255});
            display->drawRectangle(
                {boxX, y}, {boxW, boxH},
                inputField == 0 ? rtype::display::Color{50, 50, 70, 255}
                                : rtype::display::Color{40, 40, 50, 255},
                inputField == 0 ? rtype::display::Color{100, 180, 255, 255}
                                : rtype::display::Color{80, 80, 100, 255},
                2);
            display->drawText(mpHost + (inputField == 0 ? "_" : ""), "main",
                              {boxX + 8.0f, y + 6.0f}, 18,
                              {255, 255, 255, 255});
            y += 50.0f;

            display->drawText("Port:", "main", {labelX, y + 6.0f}, 18,
                              {180, 180, 180, 255});
            display->drawRectangle(
                {boxX, y}, {boxW, boxH},
                inputField == 1 ? rtype::display::Color{50, 50, 70, 255}
                                : rtype::display::Color{40, 40, 50, 255},
                inputField == 1 ? rtype::display::Color{100, 180, 255, 255}
                                : rtype::display::Color{80, 80, 100, 255},
                2);
            display->drawText(mpPortStr + (inputField == 1 ? "_" : ""), "main",
                              {boxX + 8.0f, y + 6.0f}, 18,
                              {255, 255, 255, 255});
            y += 70.0f;

            display->drawRectangle({static_cast<float>(buttonX), y},
                                   {static_cast<float>(buttonWidth),
                                    static_cast<float>(buttonHeight)},
                                   {60, 140, 220, 255}, {30, 80, 160, 255}, 2);
            display->drawText("Connect", "main",
                              {WINDOW_WIDTH / 2 - 35.0f, y + 14.0f}, 20,
                              {255, 255, 255, 255});

            display->drawText(
                "Tab to switch fields | Enter to connect | Esc to go back",
                "main", {50.0f, WINDOW_HEIGHT - 40.0f}, 14,
                {140, 140, 160, 255});
            display->display();
            continue;
        }

        if (mode == AppMode::MultiplayerLobby) {
            display->clear({20, 20, 40, 255});
            display->drawText("Lobby - Players", "main", {20.0f, 20.0f}, 22,
                              {255, 255, 255, 255});
            float y = 70.0f;
            for (auto& p : lobbyReadyStates) {
                std::string txt = "Player " + std::to_string(p.first) + " - " +
                                  (p.second ? "READY" : "NOT READY");
                display->drawText(
                    txt, "main", {40.0f, y}, 18,
                    p.second ? rtype::display::Color{80, 255, 80, 255}
                             : rtype::display::Color{255, 120, 120, 255});
                y += 26.0f;
            }
            display->drawText("Press Space to toggle Ready | Esc to leave",
                              "main", {20.0f, WINDOW_HEIGHT - 40.0f}, 16,
                              {200, 200, 200, 255});
            display->display();
            continue;
        }

        auto& gameState = registry->getSingleton<shared::GameStateComponent>();

        if (countdownTimer > 0.0f) {
            countdownTimer -= deltaSeconds;
            if (countdownTimer < 0.0f) countdownTimer = 0.0f;
        }

        if (!isMultiplayerMode) {
            if (!showGameOver &&
                (!engine->isRunning() || gameState.isGameOver)) {
                showGameOver = true;
                if (!announcedGameOver) {
                    std::cout
                        << "\n✗ GAME OVER! Final Score: " << gameState.score
                        << "\n";
                    announcedGameOver = true;
                }
            }

            if (!showGameOver && engine->isRunning()) {
                accumulator += deltaSeconds;

                while (accumulator >= FRAME_TIME) {
                    engine->update(FRAME_TIME);
                    accumulator -= FRAME_TIME;
                }
            }
        }

        display->clear({20, 20, 20, 255});

        for (int x = 0; x <= GRID_WIDTH; x++) {
            display->drawRectangle({static_cast<float>(x * CELL_SIZE), 0},
                                   {1, static_cast<float>(WINDOW_HEIGHT)},
                                   {60, 60, 60, 255}, {60, 60, 60, 255}, 0);
        }
        for (int y = 0; y <= GRID_HEIGHT; y++) {
            display->drawRectangle({0, static_cast<float>(y * CELL_SIZE)},
                                   {static_cast<float>(WINDOW_WIDTH), 1},
                                   {60, 60, 60, 255}, {60, 60, 60, 255}, 0);
        }

        auto headView =
            registry
                ->view<shared::SnakeHeadComponent, shared::PositionComponent>();
        headView.each([&display](ECS::Entity /*id*/,
                                 shared::SnakeHeadComponent& /*head*/,
                                 shared::PositionComponent& pos) {
            display->drawRectangle(
                {static_cast<float>(pos.gridX * CELL_SIZE + 2),
                 static_cast<float>(pos.gridY * CELL_SIZE + 2)},
                {static_cast<float>(CELL_SIZE - 4),
                 static_cast<float>(CELL_SIZE - 4)},
                {0, 255, 0, 255}, {0, 200, 0, 255}, 2);
        });

        auto segmentView = registry->view<shared::SnakeSegmentComponent,
                                          shared::PositionComponent>();
        segmentView.each([&display](ECS::Entity /*id*/,
                                    shared::SnakeSegmentComponent& /*seg*/,
                                    shared::PositionComponent& pos) {
            display->drawRectangle(
                {static_cast<float>(pos.gridX * CELL_SIZE + 2),
                 static_cast<float>(pos.gridY * CELL_SIZE + 2)},
                {static_cast<float>(CELL_SIZE - 4),
                 static_cast<float>(CELL_SIZE - 4)},
                {0, 180, 0, 255}, {0, 150, 0, 255}, 1);
        });

        auto foodView =
            registry->view<shared::FoodComponent, shared::PositionComponent>();
        foodView.each([&display](ECS::Entity /*id*/,
                                 shared::FoodComponent& /*food*/,
                                 shared::PositionComponent& pos) {
            display->drawRectangle(
                {static_cast<float>(pos.gridX * CELL_SIZE + 4),
                 static_cast<float>(pos.gridY * CELL_SIZE + 4)},
                {static_cast<float>(CELL_SIZE - 8),
                 static_cast<float>(CELL_SIZE - 8)},
                {255, 0, 0, 255}, {200, 0, 0, 255}, 2);
        });

        std::string scoreText = "Score: " + std::to_string(gameState.score);
        display->drawText(scoreText, "main", {10, 10}, 24,
                          {255, 255, 255, 255});

        if (countdownTimer > 0.0f) {
            int countdownSecs = static_cast<int>(countdownTimer) + 1;
            display->drawText("Starting in " + std::to_string(countdownSecs),
                              "main",
                              {WINDOW_WIDTH / 2 - 80.0f, WINDOW_HEIGHT / 2}, 32,
                              {255, 255, 100, 255});
        }

        if (showGameOver) {
            display->drawRectangle({0, 0},
                                   {static_cast<float>(WINDOW_WIDTH),
                                    static_cast<float>(WINDOW_HEIGHT)},
                                   {0, 0, 0, 180}, {0, 0, 0, 0}, 0);
            display->drawText(
                "GAME OVER", "main",
                {WINDOW_WIDTH / 2 - 90.0f, WINDOW_HEIGHT / 2 - 80.0f}, 32,
                {255, 80, 80, 255});
            display->drawText(
                "Score: " + std::to_string(gameState.score), "main",
                {WINDOW_WIDTH / 2 - 70.0f, WINDOW_HEIGHT / 2 - 40.0f}, 24,
                {255, 255, 255, 255});
            display->drawRectangle(
                {static_cast<float>(buttonX), static_cast<float>(buttonY)},
                {static_cast<float>(buttonWidth),
                 static_cast<float>(buttonHeight)},
                {60, 120, 200, 255}, {20, 60, 120, 255}, 2);
            display->drawText(
                "Replay", "main",
                {WINDOW_WIDTH / 2 - 35.0f, static_cast<float>(buttonY) + 12.0f},
                24, {255, 255, 255, 255});
            display->drawText(
                "Press Enter or R", "main",
                {WINDOW_WIDTH / 2 - 85.0f, static_cast<float>(buttonY) + 60.0f},
                18, {200, 200, 200, 255});
        }

        display->display();

        if (showGameOver) {
            continue;
        }

        frameCount++;
        auto fpsDelta =
            std::chrono::duration<float>(currentTime - lastFpsTime).count();
        if (fpsDelta >= 1.0f) {
            std::cout << "FPS: " << frameCount
                      << "  |  Score: " << gameState.score << "\n";
            frameCount = 0;
            lastFpsTime = currentTime;
        }
    }

    if (netClient && netClient->isConnected()) {
        netClient->disconnect();
    }
    engine->shutdown();
    display->close();

    std::cout << "\n✓ Game closed successfully\n";
    return 0;
}  // NOLINT(readability/fn_size)

int main(int argc, char** argv) {
    try {
        std::string preferredLib = "";
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg.find("--lib=") == 0) {
                preferredLib = arg.substr(6);
            }
        }

        std::string libPath = findDisplayLibrary(preferredLib);
        if (libPath.empty()) {
            std::cerr << "✗ Could not find display library (display.so)\n";
            std::cerr << "  Build with: ./build.sh -snake\n";
            return 1;
        }

        std::cout << "✓ Loading display library: " << libPath << "\n";

        auto displayLoader =
            std::make_unique<rtype::common::DLLoader<IDisplay>>(libPath);
        IDisplay* display = displayLoader->getInstance("createInstanceDisplay");

        if (!display) {
            std::cerr << "✗ Failed to create display instance\n";
            return 1;
        }

        std::cout << "✓ Display created: " << display->getLibName() << "\n";

        int result = runApplication(display);
        delete display;

        return result;
    } catch (const std::exception& e) {
        std::cerr << "✗ Error: " << e.what() << "\n";
        return 1;
    }
}
