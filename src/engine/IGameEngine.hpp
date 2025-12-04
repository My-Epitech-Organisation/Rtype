/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** IGameEngine - Abstract interface for the game engine
*/

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace rtype::engine {

/**
 * @brief Event types that can be emitted by the game engine
 */
enum class GameEventType : uint8_t {
    EntitySpawned = 0,
    EntityDestroyed,
    EntityUpdated
};

/**
 * @brief Data structure for game events
 *
 * Used to communicate entity state changes to external systems (network, etc.)
 */
struct GameEvent {
    GameEventType type;
    uint32_t entityNetworkId;
    float x;
    float y;
    float rotation;
    uint8_t entityType;  // Entity type identifier (e.g., Enemy, Player, etc.)
};

/**
 * @brief Pure interface for the game engine
 *
 * This interface allows the server to interact with the game engine
 * without being coupled to its implementation. The game engine is
 * responsible for managing game state, systems, and entities.
 *
 * Design principles:
 * - Complete decoupling from network/server implementation
 * - Event-based communication for state changes
 * - Deterministic update with fixed delta time
 *
 * @note Concrete game engines should inherit from AGameEngine rather
 * than this interface directly.
 *
 * Example usage:
 * @code
 * auto engine = createGameEngine();
 * engine->setEventCallback([](const GameEvent& event) {
 *     // Handle event (send to network, etc.)
 * });
 * engine->update(deltaTime);
 * @endcode
 */
class IGameEngine {
   public:
    using EventCallback = std::function<void(const GameEvent&)>;

    virtual ~IGameEngine() = default;

    /**
     * @brief Initialize the game engine
     * @return true if initialization succeeded
     */
    virtual bool initialize() = 0;

    /**
     * @brief Update the game state
     * @param deltaTime Time elapsed since last update in seconds
     */
    virtual void update(float deltaTime) = 0;

    /**
     * @brief Shutdown the game engine and release resources
     */
    virtual void shutdown() = 0;

    /**
     * @brief Set the callback for game events
     * @param callback Function to call when events occur
     */
    virtual void setEventCallback(EventCallback callback) = 0;

    /**
     * @brief Get pending events since last call
     * @return Vector of pending events
     */
    virtual std::vector<GameEvent> getPendingEvents() = 0;

    /**
     * @brief Clear all pending events
     */
    virtual void clearPendingEvents() = 0;

    /**
     * @brief Get the current entity count
     * @return Number of active entities
     */
    virtual std::size_t getEntityCount() const = 0;

    /**
     * @brief Check if the engine is running
     * @return true if the engine is initialized and running
     */
    virtual bool isRunning() const = 0;
};

/**
 * @brief Factory function to create a game engine instance
 *
 * This allows the server to create a game engine without knowing
 * the concrete implementation details.
 *
 * @return Unique pointer to a new game engine instance
 */
std::unique_ptr<IGameEngine> createGameEngine();

}  // namespace rtype::engine
