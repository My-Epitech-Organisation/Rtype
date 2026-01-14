/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** LobbyManager - Manages multiple lobby instances
*/

#pragma once

#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include "Lobby.hpp"

namespace rtype::server {

/**
 * @brief Manages multiple lobby instances and discovery service
 *
 * The LobbyManager creates and manages N lobby instances, each running
 * on a separate port. It also runs a discovery service on the base port
 * that allows clients to query available lobbies.
 */
class LobbyManager {
   public:
    /**
     * @brief Configuration for the lobby manager
     */
    struct Config {
        std::uint16_t basePort{4242};  ///< discovery service
        std::uint32_t instanceCount{1};
        std::uint32_t maxPlayers{4};
        std::uint32_t tickRate{60};
        std::string configPath{"config/server"};
        std::chrono::seconds emptyTimeout{300};  ///< Timeout for empty lobbies
        std::uint32_t maxInstances{16};          ///< Maximum allowed instances
    };

    /**
     * @brief Construct a new Lobby Manager
     *
     * @param config Manager configuration
     */
    explicit LobbyManager(const Config& config);

    /**
     * @brief Destroy the Lobby Manager
     *
     * Automatically stops all lobbies and cleanup threads
     */
    ~LobbyManager();

    LobbyManager(const LobbyManager&) = delete;
    LobbyManager& operator=(const LobbyManager&) = delete;
    LobbyManager(LobbyManager&&) = delete;
    LobbyManager& operator=(LobbyManager&&) = delete;

    /**
     * @brief Start all lobbies and the discovery service
     *
     * @return true if all started successfully, false otherwise
     */
    bool start();

    /**
     * @brief Stop all lobbies and the discovery service
     */
    void stop();

    /**
     * @brief Check if the lobby manager is running
     *
     * @return true if running, false otherwise
     */
    bool isRunning() const { return running_; }

    /**
     * @brief Run the manager (blocking call)
     *
     * Keeps the manager running until stop() is called.
     * Handles lobby lifecycle and cleanup.
     */
    void run();

    /**
     * @brief Get lobby information for all active lobbies
     *
     * @return std::vector<LobbyInfo> List of lobby information
     */
    struct LobbyInfo {
        std::string code;
        std::uint16_t port;
        std::uint32_t playerCount;
        std::uint32_t maxPlayers;
        bool isActive;
        std::string levelId;
    };

    std::vector<LobbyInfo> getActiveLobbyList() const;

    /**
     * @brief Verify that a lobby code is valid for a specific port
     *
     * @param code The lobby code to verify
     * @param port The port to check against
     * @return true if code is valid for that port, false otherwise
     */
    bool verifyLobbyCode(const std::string& code, std::uint16_t port) const;

    /**
     * @brief Find a lobby by its code
     *
     * @param code The lobby code to search for
     * @return Lobby* Pointer to the lobby, or nullptr if not found
     */
    Lobby* findLobbyByCode(const std::string& code) const;

    /**
     * @brief Get all lobby instances for metrics aggregation
     *
     * @return std::vector<Lobby*> Pointers to all lobbies
     */
    std::vector<Lobby*> getAllLobbies() const;

    /**
     * @brief Create a new lobby dynamically
     *
     * @param isPrivate If true, generates alphanumeric code; if false, uses
     * next numeric code
     * @param levelId Optional specific level to load
     * @return std::string The lobby code, or empty string on failure
     */
    std::string createLobby(bool isPrivate = true,
                            const std::string& levelId = "");

    /**
     * @brief Get list of available levels from config
     * @return List of level identifiers
     */
    std::vector<std::string> getAvailableLevels() const;

    /**
     * @brief Delete a lobby by code
     *
     * @param code The lobby code to delete
     * @return bool True if deleted, false if not found
     */
    bool deleteLobby(const std::string& code);

    /**
     * @brief Get the shared ban manager used across all lobbies
     *
     * @return std::shared_ptr<BanManager> Shared ban manager instance
     */
    std::shared_ptr<BanManager> getBanManager() { return banManager_; }

   private:
    /**
     * @brief Generate a unique 6-character alphanumeric lobby code
     *
     * @return std::string The generated code
     */
    std::string generateLobbyCode();

    /**
     * @brief Cleanup thread function
     *
     * Periodically checks for lobbies that have been empty too long
     */
    void cleanupLoop();

    /**
     * @brief Discovery service thread function
     *
     * Runs the discovery server that handles C_REQUEST_LOBBIES
     */
    void discoveryLoop();

    Config config_;  ///< Manager configuration
    std::vector<std::unique_ptr<Lobby>> lobbies_;
    std::map<std::string, Lobby*> lobbyByCode_;

    mutable std::mutex lobbiesMutex_;

    std::atomic<bool> running_{false};
    std::unique_ptr<std::thread> cleanupThread_;
    std::unique_ptr<std::thread> discoveryThread_;

    class LobbyDiscoveryServer* discoveryServer_{nullptr};

    std::mt19937 rng_;
    std::uniform_int_distribution<> charDist_;  ///< For code generation

    std::shared_ptr<BanManager>
        banManager_;  ///< Shared ban manager for all lobbies
};

}  // namespace rtype::server
