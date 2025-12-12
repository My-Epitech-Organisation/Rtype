/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** EntitySpawnerFactory - Registry-based factory for entity spawners
*/

#ifndef SRC_SERVER_SERVERAPP_GAME_ENTITYSPAWNERFACTORY_ENTITYSPAWNERFACTORY_HPP_
#define SRC_SERVER_SERVERAPP_GAME_ENTITYSPAWNERFACTORY_ENTITYSPAWNERFACTORY_HPP_

#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <rtype/ecs.hpp>
#include <rtype/engine.hpp>

#include "server/shared/IEntitySpawner.hpp"
#include "server/shared/IGameConfig.hpp"

// Forward declaration
namespace rtype::server {
class ServerNetworkSystem;
}

namespace rtype::server {

/// @brief Optional reference to a game engine
using GameEngineOpt =
    std::optional<std::reference_wrapper<engine::IGameEngine>>;
/// @brief Optional reference to a game config
using GameConfigOpt = std::optional<std::reference_wrapper<const IGameConfig>>;

/**
 * @brief Factory for creating game-specific entity spawners
 *
 * This factory uses a registry pattern to allow games to register their
 * entity spawner implementations. This enables adding new games without
 * modifying the server code.
 *
 * Usage:
 * @code
 * // In game-specific code (e.g., RType):
 * EntitySpawnerFactory::registerSpawner("rtype",
 *     [](auto registry, auto netSys, auto engine, auto config) {
 *         return std::make_unique<RTypeEntitySpawner>(...);
 *     });
 *
 * // In server code:
 * auto spawner = EntitySpawnerFactory::create("rtype", registry, netSys,
 * engine, config);
 * @endcode
 *
 * Thread-safety: All operations are thread-safe.
 */
class EntitySpawnerFactory {
   public:
    /**
     * @brief Creator function type for entity spawners
     */
    using Creator = std::function<std::unique_ptr<IEntitySpawner>(
        std::shared_ptr<ECS::Registry>, std::shared_ptr<ServerNetworkSystem>,
        GameEngineOpt, GameConfigOpt)>;

    /**
     * @brief Register an entity spawner creator
     *
     * @param gameId Unique identifier for the game
     * @param creator Function that creates the entity spawner
     * @return true if registration succeeded, false if gameId already exists
     */
    static bool registerSpawner(const std::string& gameId, Creator creator);

    /**
     * @brief Unregister an entity spawner
     *
     * @param gameId The game identifier to remove
     * @return true if unregistration succeeded, false if gameId not found
     */
    static bool unregisterSpawner(const std::string& gameId);

    /**
     * @brief Create an entity spawner instance
     *
     * @param gameId The game identifier
     * @param registry Shared pointer to the ECS registry
     * @param networkSystem Shared pointer to the network system
     * @param gameEngine Optional reference to the game engine
     * @param gameConfig Optional reference to the game config
     * @return Unique pointer to the created spawner, or nullptr if gameId not
     * found
     */
    static std::unique_ptr<IEntitySpawner> create(
        const std::string& gameId, std::shared_ptr<ECS::Registry> registry,
        std::shared_ptr<ServerNetworkSystem> networkSystem,
        GameEngineOpt gameEngine, GameConfigOpt gameConfig);

    /**
     * @brief Check if a spawner is registered
     *
     * @param gameId The game identifier to check
     * @return true if the spawner is registered
     */
    static bool isRegistered(const std::string& gameId);

    /**
     * @brief Get list of registered game IDs
     *
     * @return Vector of registered game identifiers
     */
    static std::vector<std::string> getRegisteredSpawners();

    /**
     * @brief Clear all registered spawners
     *
     * Useful for testing.
     */
    static void clearRegistry();

   private:
    EntitySpawnerFactory() = default;

    static std::unordered_map<std::string, Creator>& getRegistry();
    static std::mutex& getMutex();
};

}  // namespace rtype::server

#endif  // SRC_SERVER_SERVERAPP_GAME_ENTITYSPAWNERFACTORY_ENTITYSPAWNERFACTORY_HPP_
