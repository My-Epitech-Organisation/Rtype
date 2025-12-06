/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** main
*/

#ifndef SRC_SERVER_MAIN_HPP_
#define SRC_SERVER_MAIN_HPP_

#include <atomic>
#include <csignal>
#include <exception>
#include <format>
#include <iostream>
#include <memory>
#include <string>

/**
 * @brief Encapsulates server signal flags for thread-safe access
 *
 * This class provides a singleton-like access to signal flags used
 * by the signal handler to communicate with the main server loop.
 */
class ServerSignals {
   public:
    static std::shared_ptr<std::atomic<bool>> shutdown() noexcept {
        static std::shared_ptr<std::atomic<bool>> flag =
            std::make_shared<std::atomic<bool>>(false);
        return flag;
    }

    static std::shared_ptr<std::atomic<bool>> reloadConfig() noexcept {
        static std::shared_ptr<std::atomic<bool>> flag =
            std::make_shared<std::atomic<bool>>(false);
        return flag;
    }

    ServerSignals() = delete;
};

/**
 * @brief Server configuration structure
 */
struct ServerConfig {
    uint16_t port = 4242;
    size_t maxPlayers = 4;
    uint32_t tickRate = 60;
    bool verbose = false;
    std::string configPath = "config/server";

    bool portOverride = false;
    bool maxPlayersOverride = false;
    bool tickRateOverride = false;
};

#endif  // SRC_SERVER_MAIN_HPP_
