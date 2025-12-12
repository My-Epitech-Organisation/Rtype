/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** GameEngineFactory - Registry-based factory for game engines
*/

#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "IGameEngine.hpp"

// Forward declaration
namespace ECS {
class Registry;
}

namespace rtype::engine {

/**
 * @brief Factory for creating game engine instances
 *
 * This factory uses a registry pattern to allow games to register their
 * engine implementations. This enables adding new games without modifying
 * the server code.
 *
 * Usage:
 * @code
 * // In game-specific code (e.g., RType):
 * GameEngineFactory::registerGame("rtype", [](auto registry) {
 *     return std::make_unique<RTypeGameEngine>(registry);
 * });
 *
 * // In server code:
 * auto engine = GameEngineFactory::create("rtype", registry);
 * @endcode
 *
 * Thread-safety: All operations are thread-safe.
 */
class GameEngineFactory {
   public:
    /**
     * @brief Creator function type for game engines
     */
    using Creator =
        std::function<std::unique_ptr<IGameEngine>(std::shared_ptr<ECS::Registry>)>;

    /**
     * @brief Register a game engine creator
     *
     * @param gameId Unique identifier for the game (e.g., "rtype", "spaceinvaders")
     * @param creator Function that creates the game engine
     * @return true if registration succeeded, false if gameId already exists
     */
    static bool registerGame(const std::string& gameId, Creator creator);

    /**
     * @brief Unregister a game engine
     *
     * @param gameId The game identifier to remove
     * @return true if unregistration succeeded, false if gameId not found
     */
    static bool unregisterGame(const std::string& gameId);

    /**
     * @brief Create a game engine instance
     *
     * @param gameId The game identifier
     * @param registry Shared pointer to the ECS registry
     * @return Unique pointer to the created engine, or nullptr if gameId not found
     */
    static std::unique_ptr<IGameEngine> create(
        const std::string& gameId,
        std::shared_ptr<ECS::Registry> registry);

    /**
     * @brief Check if a game is registered
     *
     * @param gameId The game identifier to check
     * @return true if the game is registered
     */
    static bool isRegistered(const std::string& gameId);

    /**
     * @brief Get list of registered game IDs
     *
     * @return Vector of registered game identifiers
     */
    static std::vector<std::string> getRegisteredGames();

    /**
     * @brief Get the number of registered games
     *
     * @return Count of registered games
     */
    static std::size_t getRegisteredCount();

    /**
     * @brief Clear all registered games
     *
     * Useful for testing.
     */
    static void clearRegistry();

    /**
     * @brief Get the default game ID
     *
     * @return The default game ID, or empty string if none set
     */
    static std::string getDefaultGame();

    /**
     * @brief Set the default game ID
     *
     * @param gameId The game to set as default
     * @return true if the game exists and was set as default
     */
    static bool setDefaultGame(const std::string& gameId);

   private:
    GameEngineFactory() = default;

    static std::unordered_map<std::string, Creator>& getRegistry();
    static std::mutex& getMutex();
    static std::string& getDefaultGameId();
};

/**
 * @brief Helper class for automatic game registration
 *
 * Use this in game-specific code to automatically register the game
 * when the library is loaded.
 *
 * Usage:
 * @code
 * // In RTypeGameEngine.cpp:
 * static GameEngineRegistrar<RTypeGameEngine> registrar("rtype");
 * @endcode
 */
template <typename T>
class GameEngineRegistrar {
   public:
    /**
     * @brief Register a game engine type
     *
     * @param gameId Unique identifier for the game
     * @param setAsDefault Whether to set this as the default game
     */
    explicit GameEngineRegistrar(const std::string& gameId,
                                  bool setAsDefault = false) {
        GameEngineFactory::registerGame(
            gameId, [](std::shared_ptr<ECS::Registry> registry) {
                return std::make_unique<T>(std::move(registry));
            });
        if (setAsDefault) {
            GameEngineFactory::setDefaultGame(gameId);
        }
    }
};

}  // namespace rtype::engine

