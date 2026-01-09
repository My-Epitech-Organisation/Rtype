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
#include <string>
#include <utility>

namespace rtype::server {

/**
 * @brief Server game state
 */
enum class GameState { WaitingForPlayers, Playing, Paused, GameOver };

/**
 * @brief Convert GameState to string for logging
 */
[[nodiscard]] inline const std::string toString(GameState state) noexcept {
    switch (state) {
        case GameState::WaitingForPlayers:
            return "WaitingForPlayers";
        case GameState::Playing:
            return "Playing";
        case GameState::Paused:
            return "Paused";
        case GameState::GameOver:
            return "GameOver";
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
     * @brief Check if game is over
     */
    [[nodiscard]] bool isGameOver() const noexcept {
        return _state == GameState::GameOver;
    }

    /**
     * @brief Mark a player as ready
     * @param userId The user ID
     * @return true if player was newly marked ready
     */
    bool playerReady(std::uint32_t userId);

    /**
     * @brief Mark a player as not ready
     * @param userId The user ID
     * @return true if player was marked not ready (was previously ready)
     */
    bool playerNotReady(std::uint32_t userId);

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
     * @brief Set callback invoked when the countdown starts. Receives the
     * countdown duration in seconds.
     */
    void setOnCountdownStarted(std::function<void(float)> callback) {
        _onCountdownStartedCallback = std::move(callback);
    }

    /**
     * @brief Set callback invoked when a previously-started countdown is
     * cancelled (e.g., a player becomes unready).
     */
    void setOnCountdownCancelled(std::function<void()> callback) {
        _onCountdownCancelledCallback = std::move(callback);
    }

    /**
     * @brief Set the default countdown duration used when auto-starting
     */
    void setDefaultCountdown(float seconds) noexcept {
        _defaultCountdown = seconds;
    }

    /**
     * @brief Get the default countdown duration
     */
    [[nodiscard]] float getDefaultCountdown() const noexcept {
        return _defaultCountdown;
    }

    /**
     * @brief Set callback for when a player's ready state changes
     */
    void setOnPlayerReadyStateChanged(
        std::function<void(std::uint32_t userId, bool isReady)> callback) {
        _onPlayerReadyStateChangedCallback = std::move(callback);
    }

    /**
     * @brief Get whether a countdown is currently active (for tests/inspection)
     */
    [[nodiscard]] bool isCountdownActive() const noexcept {
        return _countdownActive;
    }

    /**
     * @brief Get remaining countdown time in seconds
     */
    [[nodiscard]] float getCountdownRemaining() const noexcept {
        return _countdownRemaining;
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

    /**
     * @brief Update countdown timer (call each frame)
     * @param deltaTime Time since last update in seconds
     */
    void update(float deltaTime);

    /**
     * @brief Set the total number of connected players
     * @param count Number of players currently connected
     */
    void setConnectedPlayerCount(size_t count) noexcept {
        _connectedPlayerCount = count;
        checkAutoStart();
    }

    /**
     * @brief Get the total number of connected players
     */
    [[nodiscard]] size_t getConnectedPlayerCount() const noexcept {
        return _connectedPlayerCount;
    }

   private:
    /**
     * @brief Check if game should auto-start
     */
    void checkAutoStart();

    GameState _state{GameState::WaitingForPlayers};
    std::set<std::uint32_t> _readyPlayers;
    size_t _minPlayersToStart;
    size_t _connectedPlayerCount{0};
    StateChangeCallback _stateChangeCallback;

    std::function<void(std::uint32_t userId, bool isReady)>
        _onPlayerReadyStateChangedCallback;

    bool _countdownActive{false};
    float _countdownRemaining{0.0f};

    static constexpr float DEFAULT_COUNTDOWN = 3.0f;
    float _defaultCountdown{DEFAULT_COUNTDOWN};
    std::function<void(float)> _onCountdownStartedCallback;
    std::function<void()> _onCountdownCancelledCallback;
};

}  // namespace rtype::server

#endif  // SRC_SERVER_SERVERAPP_GAME_GAMESTATEMANAGER_GAMESTATEMANAGER_HPP_
