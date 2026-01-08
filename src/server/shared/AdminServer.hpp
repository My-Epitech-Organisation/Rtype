/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** AdminServer - HTTP server for web-based admin panel
*/

#ifndef SRC_SERVER_SHARED_ADMINSERVER_HPP_
#define SRC_SERVER_SHARED_ADMINSERVER_HPP_

#include <memory>
#include <string>
#include <thread>

#include "BanManager.hpp"
#include "ServerMetrics.hpp"

namespace rtype::server {

class ServerApp;
class LobbyManager;

/**
 * @brief Web-based admin panel HTTP server
 *
 * Serves:
 * - REST API endpoints for server management and monitoring
 * - Web dashboard frontend (Vue.js)
 * - Localhost-only or token-authenticated access
 */
class AdminServer {
   public:
    /**
     * @brief Configuration for admin server
     */
    struct Config {
        uint16_t port{8080};
        bool enabled{true};
        std::string token;
        bool localhostOnly{true};
    };

    /**
     * @brief Construct admin server
     * @param config Admin server configuration
     * @param serverApp Reference to ServerApp for metrics/control
     * @param lobbyManager Reference to LobbyManager for lobby management
     */
    explicit AdminServer(const Config& config, ServerApp* serverApp,
                         LobbyManager* lobbyManager);

    ~AdminServer();

    AdminServer(const AdminServer&) = delete;
    AdminServer& operator=(const AdminServer&) = delete;
    AdminServer(AdminServer&&) = delete;
    AdminServer& operator=(AdminServer&&) = delete;

    /**
     * @brief Start the admin server (non-blocking)
     * @return true if started successfully
     */
    [[nodiscard]] bool start();

    /**
     * @brief Stop the admin server
     */
    void stop();

    /**
     * @brief Check if server is running
     * @return true if server is running
     */
    [[nodiscard]] bool isRunning() const noexcept;

   private:
    void setupRoutes();
    void runServer() noexcept;

    void registerAdminPageRoutes(void* server);
    void registerMetricsRoutes(void* server);
    void registerLobbyRoutes(void* server);
    void registerBanRoutes(void* server);

    std::string buildMetricsJson() const;

    Config _config;
    ServerApp* _serverApp;
    LobbyManager* _lobbyManager;
    void* _httpServer;  // opaque pointer to httplib::Server
    std::thread _serverThread;
    std::atomic<bool> _running{false};
};

}  // namespace rtype::server

#endif  // SRC_SERVER_SHARED_ADMINSERVER_HPP_
