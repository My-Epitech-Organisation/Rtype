/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ClientApp.hpp
*/

#ifndef SRC_CLIENT_CLIENTAPP_HPP_
#define SRC_CLIENT_CLIENTAPP_HPP_

#include <memory>
#include <string>

#include <rtype/ecs.hpp>

#include "Graphic/Graphic.hpp"
#include "network/ClientNetworkSystem.hpp"
#include "network/NetworkClient.hpp"

/**
 * @brief Main client application class
 *
 * Manages the lifecycle of:
 * - ECS Registry (shared with all subsystems)
 * - NetworkClient (handles server communication)
 * - ClientNetworkSystem (bridges network events to ECS)
 * - Graphic (window, rendering, scenes)
 *
 * Usage:
 * @code
 * ClientApp app;
 * app.run();  // Blocks until window closes
 * @endcode
 */
class ClientApp {
   public:
    /**
     * @brief Configuration for ClientApp
     */
    struct Config {
        std::string defaultServerHost;
        std::uint16_t defaultServerPort;

        Config() : defaultServerHost("127.0.0.1"), defaultServerPort(4242) {}
    };

    /**
     * @brief Construct the client application
     * @param config Optional configuration
     */
    explicit ClientApp(const Config& config = Config{});
    ~ClientApp() = default;

    /**
     * @brief Run the main application loop
     *
     * Initializes all subsystems and runs until window is closed.
     */
    void run();

    /**
     * @brief Get the ECS registry
     * @return Shared pointer to the registry
     */
    [[nodiscard]] std::shared_ptr<ECS::Registry> getRegistry() const {
        return _registry;
    }

    /**
     * @brief Get the network client
     * @return Shared pointer to the network client
     */
    [[nodiscard]] std::shared_ptr<rtype::client::NetworkClient>
    getNetworkClient() const {
        return _networkClient;
    }

    /**
     * @brief Get the client network system
     * @return Shared pointer to the network system
     */
    [[nodiscard]] std::shared_ptr<rtype::client::ClientNetworkSystem>
    getNetworkSystem() const {
        return _networkSystem;
    }

    ClientApp(const ClientApp&) = delete;
    ClientApp& operator=(const ClientApp&) = delete;
    ClientApp(ClientApp&&) = delete;
    ClientApp& operator=(ClientApp&&) = delete;

   private:
    Config _config;

    /// @brief ECS registry - central data store for all entities/components
    std::shared_ptr<ECS::Registry> _registry;

    /// @brief Network client - handles raw network I/O
    std::shared_ptr<rtype::client::NetworkClient> _networkClient;

    /// @brief Network system - bridges network events to ECS
    std::shared_ptr<rtype::client::ClientNetworkSystem> _networkSystem;

    /// @brief Graphics system (created after network to receive dependencies)
    std::unique_ptr<Graphic> _graphic;
};

#endif  // SRC_CLIENT_CLIENTAPP_HPP_
