/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** GameSession - Game state management
*/

#ifndef SRC_SERVER_SERVERAPP_GAME_GAMESESSION_GAMESESSION_HPP_
#define SRC_SERVER_SERVERAPP_GAME_GAMESESSION_GAMESESSION_HPP_

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <set>

#include <rtype/ecs.hpp>
#include <rtype/engine.hpp>

#include "IEntitySpawner.hpp"
#include "IGameConfig.hpp"

namespace rtype::server {

// Forward declarations
class ServerNetworkSystem;

/**
 * @brief Server game state
 *
 * Controls what the server does at each tick:
 * - WaitingForPlayers: Server accepts connections but doesn't run gameplay
 * - Playing: Full game simulation running
 * - Paused: Game paused (all clients disconnected during game)
 */
enum class GameState {
    WaitingForPlayers,
    Playing,
    Paused
};

/**
 * @brief Converts GameState to string representation
 */
[[nodiscard]] constexpr const char* toString(GameState state) noexcept {
    switch (state) {
        case GameState::WaitingForPlayers:
            return "WaitingForPlayers";
        case GameState::Playing:
            return "Playing";
        case GameState::Paused:
            return "Paused";
        default:
            return "Unknown";
    }
}

/**
 * @brief Configuration for GameSession
 */
struct GameSessionConfig {
    std::size_t minPlayersToStart = 1;
    bool verbose = false;
};

/**
 * @brief Manages the game state and player lifecycle
 *
 * This class encapsulates:
 * - Game state transitions (Waiting -> Playing -> Paused)
 * - Player spawning/despawning via IEntitySpawner
 * - Ready player tracking
 * - Game engine updates
 *
 * Usage:
 * @code
 * GameSession session(registry, networkSystem, entitySpawner, gameEngine,
 * config);
 *
 * // When a client connects:
 * session.handleClientConnected(userId);
 *
 * // When a client sends input:
 * session.handleClientInput(userId, inputMask);
 *
 * // In game loop:
 * session.update(deltaTime);
 * @endcode
 */
class GameSession {
   public:
    /**
     * @brief Callback type for state transitions
     */
    using StateChangeCallback =
        std::function<void(GameState oldState, GameState newState)>;

    /**
     * @brief Construct a GameSession
     *
     * @param registry Shared pointer to the ECS registry
     * @param networkSystem Shared pointer to the network system
     * @param entitySpawner Unique pointer to the entity spawner (takes
     * ownership)
     * @param gameEngine Shared pointer to the game engine
     * @param gameConfig Shared pointer to the game config (can be null)
     * @param config Session configuration
     */
    GameSession(std::shared_ptr<ECS::Registry> registry,
                std::shared_ptr<ServerNetworkSystem> networkSystem,
                std::unique_ptr<IEntitySpawner> entitySpawner,
                std::shared_ptr<engine::IGameEngine> gameEngine,
                std::shared_ptr<const IGameConfig> gameConfig,
                const GameSessionConfig& config = {});

    ~GameSession() = default;

    GameSession(const GameSession&) = delete;
    GameSession& operator=(const GameSession&) = delete;
    GameSession(GameSession&&) = delete;
    GameSession& operator=(GameSession&&) = delete;

    /**
     * @brief Update the game session
     *
     * Updates game engine and player movements if in Playing state.
     *
     * @param deltaTime Time since last update in seconds
     */
    void update(float deltaTime);

    /**
     * @brief Handle a new client connection
     *
     * Spawns a player entity for the client.
     *
     * @param userId The user ID of the connected client
     */
    void handleClientConnected(std::uint32_t userId);

    /**
     * @brief Handle a client disconnection
     *
     * Removes the player entity and updates game state if needed.
     *
     * @param userId The user ID of the disconnected client
     */
    void handleClientDisconnected(std::uint32_t userId);

    /**
     * @brief Handle client input
     *
     * Processes movement and shooting inputs.
     *
     * @param userId The user ID of the client
     * @param inputMask The input bitmask
     * @param entity The player entity (if known)
     */
    void handleClientInput(std::uint32_t userId, std::uint8_t inputMask,
                           std::optional<ECS::Entity> entity);

    /**
     * @brief Signal that a player is ready
     *
     * @param userId The user ID that signaled ready
     */
    void playerReady(std::uint32_t userId);

    /**
     * @brief Get the current game state
     *
     * @return Current game state
     */
    [[nodiscard]] GameState getState() const noexcept { return _state; }

    /**
     * @brief Check if the game is actively playing
     *
     * @return true if state is Playing
     */
    [[nodiscard]] bool isPlaying() const noexcept {
        return _state == GameState::Playing;
    }

    /**
     * @brief Get the number of ready players
     *
     * @return Count of players who signaled ready
     */
    [[nodiscard]] std::size_t getReadyPlayerCount() const noexcept {
        return _readyPlayers.size();
    }

    /**
     * @brief Set callback for state changes
     *
     * @param callback Function to call on state change
     */
    void setStateChangeCallback(StateChangeCallback callback) {
        _stateChangeCallback = std::move(callback);
    }

    /**
     * @brief Process pending game events
     *
     * Processes events from the game engine and routes them to the network.
     */
    void processGameEvents();

    /**
     * @brief Synchronize entity positions with the network
     */
    void syncEntityPositions();

   private:
    /**
     * @brief Transition to a new game state
     */
    void transitionToState(GameState newState);

    /**
     * @brief Check if game should start (enough ready players)
     */
    void checkGameStart();

    /**
     * @brief Update player movement based on velocity
     */
    void updatePlayerMovement(float deltaTime);

    std::shared_ptr<ECS::Registry> _registry;
    std::shared_ptr<ServerNetworkSystem> _networkSystem;
    std::unique_ptr<IEntitySpawner> _entitySpawner;
    std::shared_ptr<engine::IGameEngine> _gameEngine;
    std::shared_ptr<const IGameConfig> _gameConfig;
    GameSessionConfig _config;

    GameState _state{GameState::WaitingForPlayers};
    std::set<std::uint32_t> _readyPlayers;
    StateChangeCallback _stateChangeCallback;
};

}  // namespace rtype::server

#endif  // SRC_SERVER_SERVERAPP_GAME_GAMESESSION_GAMESESSION_HPP_
