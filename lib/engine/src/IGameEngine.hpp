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
#include <optional>
#include <string>
#include <vector>

namespace rtype::engine {

/**
 * @brief Event types that can be emitted by the game engine
 */
enum class GameEventType : uint8_t {
    EntitySpawned = 0,
    EntityDestroyed,
    EntityUpdated,
    EntityHealthChanged,
    PowerUpApplied,
    GameOver,
    BossPhaseChanged,
    BossDefeated,
    BossAttack,
    WeakPointDestroyed,
    LevelComplete,
    ScoreChanged
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
    uint8_t entityType;
    uint8_t subType = 0;
    float velocityX = 0.0F;
    float velocityY = 0.0F;
    int32_t healthCurrent{0};
    int32_t healthMax{0};
    int32_t damage{0};
    float duration{0.0F};
    uint8_t bossPhase{0};
    uint8_t bossPhaseCount{0};
    float attackAngle{0.0F};
    float attackProgress{0.0F};
    uint32_t parentNetworkId{0};
    int32_t score{0};
};

/**
 * @brief Result of processing a game event
 *
 * Contains information needed by the server to handle the event
 * (e.g., register/unregister entities from network system)
 */
struct ProcessedEvent {
    GameEventType type;
    uint32_t networkId;
    uint8_t networkEntityType;
    uint8_t subType{0};
    float x;
    float y;
    float vx;
    float vy;
    float duration;
    bool valid;
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
 * - Game-agnostic: no player/projectile-specific methods
 *
 * @note Concrete game engines should inherit from AGameEngine rather
 * than this interface directly.
 *
 * Example usage:
 * @code
 * auto registry = std::make_shared<ECS::Registry>();
 * auto engine = GameEngineFactory::create("rtype", registry);
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

    /**
     * @brief Get the game identifier
     * @return Game identifier string (e.g., "rtype", "spaceinvaders")
     */
    [[nodiscard]] virtual std::string getGameId() const = 0;

    /**
     * @brief Load a specific level/map from file
     * @param filepath The path to the level configuration file
     * @return true if loaded successfully
     */
    virtual bool loadLevelFromFile(const std::string& filepath) = 0;

    /**
     * @brief Process a game event and return network-ready data
     *
     * This method allows each game engine to implement its own event
     * processing logic, including finding entities by network ID and
     * mapping game types to network types.
     *
     * @param event The game event to process
     * @return ProcessedEvent with network-ready data
     */
    virtual ProcessedEvent processEvent(const GameEvent& event) = 0;

    /**
     * @brief Sync entity positions and mark them for network broadcast
     *
     * Called each frame to prepare entity position updates for network
     * synchronization. The callback is invoked for each entity that needs
     * to be synced.
     *
     * @param callback Function called with (networkId, x, y, vx, vy) for each entity
     */
    virtual void syncEntityPositions(
        std::function<void(uint32_t, float, float, float, float)> callback) = 0;
};

}  // namespace rtype::engine

// Forward declaration for ECS::Registry
namespace ECS {
class Registry;
}

namespace rtype::engine {

/**
 * @brief Factory function to create a game engine instance
 *
 * This allows the server to create a game engine without knowing
 * the concrete implementation details. Uses the default registered game.
 *
 * @param registry Shared pointer to the ECS registry
 * @return Unique pointer to a new game engine instance
 */
std::unique_ptr<IGameEngine> createGameEngine(std::shared_ptr<ECS::Registry> registry);

}  // namespace rtype::engine
