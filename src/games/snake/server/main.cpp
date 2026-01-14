/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Minimal Snake server - local testing using existing NetworkServer
*/

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <unordered_map>

#include <rtype/ecs.hpp>

#include "GameEngine.hpp"
#include "../shared/Components.hpp"

#include <server/network/NetworkServer.hpp>

using namespace std::chrono_literals;
using namespace rtype::games::snake::server;
using namespace rtype::games::snake::shared;

static ECS::Entity spawnPlayer(std::shared_ptr<ECS::Registry> reg, uint32_t playerId, int startX, int startY)
{
    auto head = reg->spawnEntity();
    reg->emplaceComponent<SnakeHeadComponent>(head, SnakeHeadComponent{.playerId = playerId});
    reg->emplaceComponent<PositionComponent>(head, PositionComponent{.gridX = startX, .gridY = startY});
    reg->emplaceComponent<VelocityComponent>(head, VelocityComponent{.dirX = 1, .dirY = 0});
    reg->emplaceComponent<PlayerInputComponent>(head, PlayerInputComponent{.playerId = playerId, .nextDirection = Direction::NONE});
    return head;
}

int main(int argc, char** argv)
{
    uint16_t port = 4242;
    if (argc > 1) {
        try {
            port = static_cast<uint16_t>(std::stoi(argv[1]));
        } catch (...) {
            std::cerr << "Usage: snake_server [port]" << std::endl;
            return 1;
        }
    }

    rtype::server::NetworkServer server;
    if (!server.start(port)) {
        std::cerr << "Failed to start NetworkServer on port " << port << std::endl;
        return 1;
    }

    std::cout << "Snake server listening on port " << port << " (local testing)" << std::endl;
    std::cout << "Start the Snake client (local) with: ./snake_game" << std::endl;

    auto registry = std::make_shared<ECS::Registry>();
    auto engine = std::make_unique<SnakeGameEngine>(registry, false);
    if (!engine->initialize()) {
        std::cerr << "Failed to initialize SnakeGameEngine" << std::endl;
        return 1;
    }

    std::unordered_map<uint32_t, ECS::Entity> players;
    std::unordered_map<uint32_t, bool> readyStates;
    bool gameStarted = false;

    auto checkAndStartGame = [&]() {
        if (gameStarted) return;
        if (players.size() < 2) return;
        for (auto &p : players) {
            auto it = readyStates.find(p.first);
            if (it == readyStates.end() || !it->second) return;
        }

        const float countdown = 3.0f;
        std::cout << "All players ready - starting in " << countdown << "s..." << std::endl;
        server.broadcastGameStart(countdown);
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(countdown * 1000.0f)));

        int idx = 0;
        for (auto &p : players) {
            int sx = (idx == 0) ? 5 : 14;
            int sy = (idx == 0) ? 5 : 14;
            auto ent = engine->spawnSnakeForPlayer(p.first, sx, sy);
            players[p.first] = ent;
            server.spawnEntity(ent.id, rtype::server::NetworkServer::EntityType::Player, 0, static_cast<float>(sx), static_cast<float>(sy));
            idx++;
        }

        engine->startGame();
        server.updateGameState(rtype::server::NetworkServer::GameState::Running);
        gameStarted = true;
        std::cout << "Game started" << std::endl;
    };

    server.onClientConnected([&](uint32_t userId) {
        std::cout << "Client connected: " << userId << std::endl;

        for (auto &rs : readyStates) {
            server.broadcastPlayerReadyState(rs.first, rs.second);
        }

        players[userId] = ECS::Entity{0};
        readyStates[userId] = false;
        server.broadcastPlayerReadyState(userId, false);
    });

    server.onClientDisconnected([&](uint32_t userId, rtype::network::DisconnectReason /*reason*/) {
        std::cout << "Client disconnected: " << userId << std::endl;
        auto it = players.find(userId);
        if (it != players.end()) {
            auto ent = it->second;
            if (ent.id != 0) {
                registry->killEntity(ent);
                server.destroyEntity(ent.id);
            }
            players.erase(it);
        }
        readyStates.erase(userId);

        if (gameStarted && players.size() < 2) {
            std::cout << "Not enough players - returning to lobby" << std::endl;
            engine->stopGame();
            server.updateGameState(rtype::server::NetworkServer::GameState::Lobby);
            gameStarted = false;
        }
    });

    server.onClientReady([&](uint32_t userId, bool isReady) {
        auto it = readyStates.find(userId);
        if (it == readyStates.end()) return;
        it->second = isReady;
        server.broadcastPlayerReadyState(userId, isReady);
        std::cout << "Client " << userId << (isReady ? " ready" : " not ready") << std::endl;
        checkAndStartGame();
    });

    server.onClientInput([&](uint32_t userId, uint8_t input) {
        auto it = players.find(userId);
        if (it == players.end()) return;
        auto ent = it->second;
        if (ent.id == 0) return;
        auto &inputComp = registry->getComponent<PlayerInputComponent>(ent);
        if (input & rtype::network::InputMask::kUp) inputComp.nextDirection = Direction::UP;
        else if (input & rtype::network::InputMask::kDown) inputComp.nextDirection = Direction::DOWN;
        else if (input & rtype::network::InputMask::kLeft) inputComp.nextDirection = Direction::LEFT;
        else if (input & rtype::network::InputMask::kRight) inputComp.nextDirection = Direction::RIGHT;
    });

    const std::chrono::milliseconds tickInterval(16);
    uint32_t serverTick = 0;
    bool running = true;

    while (running) {
        auto loopStart = std::chrono::steady_clock::now();

        server.poll();

        if (gameStarted) {
            engine->update(0.0166f);

            auto& gameState = registry->getSingleton<GameStateComponent>();
            if (gameState.isGameOver) {
                std::cout << "Game over! Final score: " << gameState.score << std::endl;
                server.sendGameOver(gameState.score);
                server.updateGameState(rtype::server::NetworkServer::GameState::Lobby);
                gameStarted = false;
                gameState.isGameOver = false;
                gameState.score = 0;
            }
        }

        std::vector<std::tuple<uint32_t, float, float, float, float>> moves;
        auto view = registry->view<PositionComponent>();
        view.each([&](ECS::Entity id, PositionComponent &pos) {
            float x = static_cast<float>(pos.gridX);
            float y = static_cast<float>(pos.gridY);
            moves.emplace_back(id.id, x, y, 0.0f, 0.0f);
        });

        if (gameStarted && !moves.empty()) {
            server.moveEntitiesBatch(moves);
        }

        serverTick++;

        auto elapsed = std::chrono::steady_clock::now() - loopStart;
        if (elapsed < tickInterval) std::this_thread::sleep_for(tickInterval - elapsed);
    }

    server.stop();
    return 0;
}
