/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** GameEventProcessor - Processes game events and routes to network
*/

#ifndef SRC_SERVER_SERVERAPP_GAME_GAMEEVENT_GAMEEVENTPROCESSOR_HPP_
#define SRC_SERVER_SERVERAPP_GAME_GAMEEVENT_GAMEEVENTPROCESSOR_HPP_

#include <memory>

#include <rtype/engine.hpp>

namespace rtype::server {

class ServerNetworkSystem;

/**
 * @brief Processes game engine events and routes them to the network
 *
 * Handles:
 * - Entity spawned events -> broadcast spawn
 * - Entity destroyed events -> broadcast destroy
 * - Entity updated events -> update position
 * - Entity health changed events -> update health
 */
class GameEventProcessor {
   public:
    /**
     * @brief Construct a GameEventProcessor
     * @param gameEngine Shared pointer to the game engine
     * @param networkSystem Shared pointer to the network system
     * @param verbose Enable verbose logging
     */
    GameEventProcessor(std::shared_ptr<engine::IGameEngine> gameEngine,
                       std::shared_ptr<ServerNetworkSystem> networkSystem,
                       bool verbose = false);

    ~GameEventProcessor() = default;

    /**
     * @brief Process all pending game events
     */
    void processEvents();

    /**
     * @brief Sync entity positions with network
     */
    void syncEntityPositions();

   private:
    std::shared_ptr<engine::IGameEngine> _gameEngine;
    std::shared_ptr<ServerNetworkSystem> _networkSystem;
    bool _verbose;
};

}  // namespace rtype::server

#endif  // SRC_SERVER_SERVERAPP_GAME_GAMEEVENT_GAMEEVENTPROCESSOR_HPP_
