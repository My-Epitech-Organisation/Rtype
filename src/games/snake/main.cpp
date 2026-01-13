/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Snake Game - Playable executable with IDisplay abstraction
*/

#include <chrono>
#include <filesystem>
#include <iostream>
#include <memory>
#include <thread>
#include <utility>

#include "include/rtype/display/IDisplay.hpp"
#include "lib/common/src/DLLoader/DLLoader.hpp"
#include "server/GameEngine.hpp"
#include "shared/Components.hpp"

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

std::string findDisplayLibrary(const std::string& preferredLib = "") {
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

        auto newGame = []() {
            auto reg = std::make_shared<ECS::Registry>();
            auto eng = std::make_unique<server::SnakeGameEngine>(reg);
            return std::pair{std::move(reg), std::move(eng)};
        };

        auto [registry, engine] = newGame();
        std::cout << "✓ ECS Registry created\n";
        std::cout << "✓ Snake Game Engine created\n";

        auto initGame = [&]() -> bool {
            if (!engine->initialize()) {
                std::cerr << "✗ Failed to initialize game engine\n";
                return false;
            }
            std::cout << "✓ Game Engine initialized\n";
            return true;
        };
        if (!initGame()) {
            delete display;
            return 1;
        }

        display->open(WINDOW_WIDTH, WINDOW_HEIGHT,
                      "Snake - Graphics Abstraction Demo", false);
        display->setFramerateLimit(60);

        display->loadFont("main",
                          "assets/fonts/Orbitron-VariableFont_wght.ttf");

        std::cout << "✓ Display window opened\n\n";

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

                if (showGameOver) {
                    if (event.type == EventType::KeyPressed &&
                        event.key.code == Key::Escape) {
                        running = false;
                        break;
                    }
                    if (event.type == EventType::KeyPressed &&
                        (event.key.code == Key::Return ||
                         event.key.code == Key::R)) {
                        auto pair = newGame();
                        registry = std::move(pair.first);
                        engine = std::move(pair.second);
                        if (!initGame()) {
                            running = false;
                            break;
                        }
                        showGameOver = false;
                        announcedGameOver = false;
                        accumulator = 0.0f;
                        frameCount = 0;
                        lastTime = std::chrono::high_resolution_clock::now();
                        lastFpsTime = lastTime;
                        continue;
                    }
                    if (event.type == EventType::MouseButtonPressed) {
                        int mx = event.mouseButton.x;
                        int my = event.mouseButton.y;
                        if (mx >= buttonX && mx <= buttonX + buttonWidth &&
                            my >= buttonY && my <= buttonY + buttonHeight) {
                            auto pair = newGame();
                            registry = std::move(pair.first);
                            engine = std::move(pair.second);
                            if (!initGame()) {
                                running = false;
                                break;
                            }
                            showGameOver = false;
                            announcedGameOver = false;
                            accumulator = 0.0f;
                            frameCount = 0;
                            lastTime =
                                std::chrono::high_resolution_clock::now();
                            lastFpsTime = lastTime;
                            continue;
                        }
                    }
                    continue;
                }

                if (event.type == EventType::KeyPressed) {
                    if (event.key.code == Key::Escape) {
                        running = false;
                        break;
                    }

                    shared::Direction dir = keyToDirection(event.key.code);
                    if (dir != shared::Direction::NONE) {
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
            }

            if (!running) break;

            auto& gameState =
                registry->getSingleton<shared::GameStateComponent>();
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

            auto headView = registry->view<shared::SnakeHeadComponent,
                                           shared::PositionComponent>();
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
                registry
                    ->view<shared::FoodComponent, shared::PositionComponent>();
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
                display->drawText("Replay", "main",
                                  {WINDOW_WIDTH / 2 - 35.0f,
                                   static_cast<float>(buttonY) + 12.0f},
                                  24, {255, 255, 255, 255});
                display->drawText("Press Enter or R", "main",
                                  {WINDOW_WIDTH / 2 - 85.0f,
                                   static_cast<float>(buttonY) + 60.0f},
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

        engine->shutdown();
        display->close();
        delete display;

        std::cout << "\n✓ Game closed successfully\n";

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "✗ Error: " << e.what() << "\n";
        return 1;
    }
}
