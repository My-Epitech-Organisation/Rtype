/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** GameStateManager - Manages game state transitions
*/

#ifndef SRC_SERVER_SERVERAPP_GAME_GAMESTATEMANAGER_GAMESTATEMANAGER_HPP_
#define SRC_SERVER_SERVERAPP_GAME_GAMESTATEMANAGER_GAMESTATEMANAGER_HPP_

#include <cstdint>
#include <functional>
#include <set>

namespace rtype::server {

/**
 * @brief Server game state
 */
enum class GameState { WaitingForPlayers, Playing, Paused };

/**
 * @brief Convert GameState to string for logging
 */
[[nodiscard]] inline const char* toString(GameState state) noexcept {
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
 * @brief Callback type for state transitions
 */
using StateChangeCallback =
    std::function<void(GameState oldState, GameState newState)>;

/**
 * @brief Manages game state and player readiness
 *
 * Handles:
 * - State transitions (WaitingForPlayers -> Playing -> Paused)
 * - Player ready tracking
 * - Auto-start when enough players are ready
 */
class GameStateManager {
   public:
    /**
     * @brief Default minimum players to start a game
     */
    static constexpr size_t DEFAULT_MIN_PLAYERS = 1;

    /**
     * @brief Construct a new GameStateManager
     * @param minPlayersToStart Minimum ready players required to start
     */
    explicit GameStateManager(size_t minPlayersToStart = DEFAULT_MIN_PLAYERS);

    ~GameStateManager() = default;
    GameStateManager(const GameStateManager&) = delete;
    GameStateManager& operator=(const GameStateManager&) = delete;
    GameStateManager(GameStateManager&&) = default;
    GameStateManager& operator=(GameStateManager&&) = default;

    /**
     * @brief Get current game state
     */
    [[nodiscard]] GameState getState() const noexcept { return _state; }

    /**
     * @brief Check if game is actively playing
     */
    [[nodiscard]] bool isPlaying() const noexcept {
        return _state == GameState::Playing;
    }

    /**
     * @brief Check if waiting for players
     */
    [[nodiscard]] bool isWaiting() const noexcept {
        return _state == GameState::WaitingForPlayers;
    }

    /**
     * @brief Check if game is paused
     */
    [[nodiscard]] bool isPaused() const noexcept {
        return _state == GameState::Paused;
    }

    /**
     * @brief Mark a player as ready
     * @param userId The user ID
     * @return true if player was newly marked ready
     */
    bool playerReady(std::uint32_t userId);

    /**
     * @brief Remove a player from ready set
     * @param userId The user ID
     */
    void playerLeft(std::uint32_t userId);

    /**
     * @brief Get number of ready players
     */
    [[nodiscard]] size_t getReadyPlayerCount() const noexcept {
        return _readyPlayers.size();
    }

    /**
     * @brief Check if a player is ready
     */
    [[nodiscard]] bool isPlayerReady(std::uint32_t userId) const noexcept {
        return _readyPlayers.count(userId) > 0;
    }

    /**
     * @brief Get the set of ready players
     */
    [[nodiscard]] const std::set<std::uint32_t>& getReadyPlayers()
        const noexcept {
        return _readyPlayers;
    }

    /**
     * @brief Transition to a new state
     * @param newState The target state
     */
    void transitionTo(GameState newState);

    /**
     * @brief Set callback for state changes
     */
    void setStateChangeCallback(StateChangeCallback callback) {
        _stateChangeCallback = std::move(callback);
    }

    /**
     * @brief Force transition to Playing state (for testing)
     */
    void forceStart();

    /**
     * @brief Force transition to Paused state
     */
    void pause();

    /**
     * @brief Reset to WaitingForPlayers and clear ready players
     */
    void reset();

   private:
    /**
     * @brief Check if game should auto-start
     */
    void checkAutoStart();

    GameState _state{GameState::WaitingForPlayers};
    std::set<std::uint32_t> _readyPlayers;
    size_t _minPlayersToStart;
    StateChangeCallback _stateChangeCallback;
};

}  // namespace rtype::server

#endif  // SRC_SERVER_SERVERAPP_GAME_GAMESTATEMANAGER_GAMESTATEMANAGER_HPP_
