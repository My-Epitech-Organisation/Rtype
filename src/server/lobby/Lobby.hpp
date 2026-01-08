/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** Lobby - Wrapper for a single game lobby instance
*/

#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <thread>

#include "server/serverApp/ServerApp.hpp"

namespace rtype::server {

class LobbyManager;

/**
 * @brief Represents a single game lobby instance
 *
 * Each Lobby wraps a ServerApp instance and runs it in a dedicated thread.
 * Lobbies are identified by a unique code and listen on a specific port.
 */
class Lobby {
   public:
    /**
     * @brief Configuration for a lobby instance
     */
    struct Config {
        std::uint16_t port{0};  ///< Port for this lobby
        std::uint32_t maxPlayers{4};
        std::uint32_t tickRate{60};
        std::string configPath{"config/server"};
        std::chrono::seconds emptyTimeout{
            300};  ///< Time to keep empty lobby alive
    };

    /**
     * @brief Construct a new Lobby
     *
     * @param code Unique 6-character alphanumeric lobby code
     * @param config Lobby configuration
     * @param manager Pointer to the lobby manager (optional)
     * @param banManager Shared ban manager for connection enforcement
     */
    Lobby(const std::string& code, const Config& config,
          LobbyManager* manager = nullptr,
          std::shared_ptr<BanManager> banManager = nullptr);

    /**
     * @brief Destroy the Lobby
     *
     * Automatically stops the lobby and joins the thread
     */
    ~Lobby();

    // Delete copy/move to prevent thread ownership issues
    Lobby(const Lobby&) = delete;
    Lobby& operator=(const Lobby&) = delete;
    Lobby(Lobby&&) = delete;
    Lobby& operator=(Lobby&&) = delete;

    /**
     * @brief Start the lobby server
     *
     * Spawns a dedicated thread running the ServerApp game loop
     *
     * @return true if started successfully, false otherwise
     */
    bool start();

    /**
     * @brief Stop the lobby server
     *
     * Signals the server to shutdown and waits for the thread to finish
     */
    void stop();

    /**
     * @brief Check if the lobby is currently running
     *
     * @return true if running, false otherwise
     */
    bool isRunning() const;

    /**
     * @brief Get the lobby code
     *
     * @return const std::string& The unique lobby code
     */
    const std::string& getCode() const { return code_; }

    /**
     * @brief Get the port this lobby is listening on
     *
     * @return std::uint16_t The port number (0 if not started)
     */
    std::uint16_t getPort() const { return actualPort_; }

    /**
     * @brief Get the number of connected players
     *
     * @return std::uint32_t Current player count
     */
    std::uint32_t getPlayerCount() const;

    /**
     * @brief Get the maximum number of players
     *
     * @return std::uint32_t Maximum player count
     */
    std::uint32_t getMaxPlayers() const { return config_.maxPlayers; }

    /**
     * @brief Check if the lobby is empty
     *
     * @return true if no players are connected, false otherwise
     */
    bool isEmpty() const;

    /**
     * @brief Get the time since the lobby became empty
     *
     * @return std::chrono::seconds Time since empty (0 if not empty)
     */
    std::chrono::seconds getTimeSinceEmpty() const;

    /**
     * @brief Get the lobby's ServerApp for metrics access
     *
     * @return ServerApp* Pointer to the server app, or nullptr if not running
     */
    ServerApp* getServerApp() const { return serverApp_.get(); }

    /**
     * @brief Update the last activity timestamp
     *
     * Called when players join/leave to track empty timeout
     */
    void updateActivity();

   private:
    /**
     * @brief Main thread function running the server loop
     */
    void run();

    std::string code_;
    Config config_;
    std::uint16_t actualPort_{0};  ///< Actual port after binding

    std::shared_ptr<std::atomic<bool>> shutdownFlagPtr_;
    std::unique_ptr<ServerApp> serverApp_;
    std::unique_ptr<std::thread> thread_;

    std::atomic<bool> running_{false};
    std::chrono::steady_clock::time_point lastActivity_;
    mutable std::mutex activityMutex_;
    LobbyManager* lobbyManager_{nullptr};
    std::shared_ptr<BanManager> banManager_;
};

}  // namespace rtype::server
