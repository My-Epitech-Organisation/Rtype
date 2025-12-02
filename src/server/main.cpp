/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Server main entry point
*/

#include "main.hpp"

#include <atomic>
#include <csignal>
#include <exception>
#include <format>
#include <iostream>

#include "../common/ArgParser.hpp"
#include "ServerApp.hpp"

/**
 * @brief Signal handler for graceful shutdown and config reload
 * @param signal The signal received
 */
static void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\n[Main] Received shutdown signal" << std::endl;
        ServerSignals::shutdown() = true;
    }
#ifndef _WIN32
    if (signal == SIGHUP) {
        std::cout << "\n[Main] Received SIGHUP - config reload requested"
                  << std::endl;
        ServerSignals::reloadConfig() = true;
    }
#endif
}

/**
 * @brief Setup signal handlers for the server
 */
static void setupSignalHandlers() {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
#ifndef _WIN32
    std::signal(SIGHUP, signalHandler);
#endif
}

/**
 * @brief Configure the argument parser with server options
 * @param parser The argument parser to configure
 * @param config The server configuration to populate
 * @return Reference to the configured parser
 */
static rtype::ArgParser& configureParser(rtype::ArgParser& parser,
                                         ServerConfig& config) {
    parser
        .flag("-h", "--help", "Show this help message",
              [&parser]() {
                  parser.printUsage();
                  return rtype::ParseResult::Exit;
              })
        .flag("-v", "--verbose", "Enable verbose debug output",
              [&config]() {
                  config.verbose = true;
                  return rtype::ParseResult::Success;
              })
        .option("-p", "--port", "port", "Server port (1-65535, default: 4242)",
                [&config](std::string_view val) {
                    auto v =
                        rtype::parseNumber<uint16_t>(val, "port", 1, 65535);
                    if (!v.has_value()) return rtype::ParseResult::Error;
                    config.port = v.value();
                    return rtype::ParseResult::Success;
                })
        .option(
            "-m", "--max-players", "n", "Maximum players (1-256, default: 4)",
            [&config](std::string_view val) {
                auto v = rtype::parseNumber<size_t>(val, "max-players", 1, 256);
                if (!v.has_value()) return rtype::ParseResult::Error;
                config.maxPlayers = v.value();
                return rtype::ParseResult::Success;
            })
        .option("-t", "--tick-rate", "hz",
                "Tick rate in Hz (1-1000, default: 60)",
                [&config](std::string_view val) {
                    auto v =
                        rtype::parseNumber<uint32_t>(val, "tick-rate", 1, 1000);
                    if (!v.has_value()) return rtype::ParseResult::Error;
                    config.tickRate = v.value();
                    return rtype::ParseResult::Success;
                });
    return parser;
}

/**
 * @brief Print the server startup banner with configuration
 * @param config The server configuration to display
 */
static void printBanner(const ServerConfig& config) {
    std::cout << "==================================\n"
              << "    R-Type Server\n"
              << "==================================\n"
              << std::format("  Port:        {}\n", config.port)
              << std::format("  Max Players: {}\n", config.maxPlayers)
              << std::format("  Tick Rate:   {} Hz\n", config.tickRate)
              << std::format("  Verbose:     {}\n",
                             config.verbose ? "yes" : "no")
              << "==================================" << std::endl;
}

/**
 * @brief Run the server with the given configuration
 * @param config The server configuration
 * @param shutdownFlag Reference to the shutdown flag
 * @param reloadConfigFlag Reference to the reload config flag
 * @return Exit code (0 for success, 1 for failure)
 */
static int runServer(const ServerConfig& config,
                     std::atomic<bool>& shutdownFlag,
                     std::atomic<bool>& reloadConfigFlag) {
    rtype::server::ServerApp server(
        config.port, config.maxPlayers, config.tickRate, shutdownFlag,
        rtype::server::ServerApp::DEFAULT_CLIENT_TIMEOUT_SECONDS,
        config.verbose);

    // Check for config reload requests during runtime
    // TODO(Clem): Implement hot reload when reloadConfigFlag becomes true
    (void)reloadConfigFlag;  // Suppress unused warning until implemented

    if (!server.run()) {
        std::cerr << "[Main] Server failed to start." << std::endl;
        return 1;
    }

    std::cout << "[Main] Server terminated." << std::endl;
    return 0;
}

int main(int argc, char** argv) {
    try {
        ServerConfig config;
        std::vector<std::string_view> args(argv + 1, argv + argc);
        rtype::ArgParser parser;

        parser.programName(argv[0]);
        configureParser(parser, config);
        rtype::ParseResult parseResult = parser.parse(args);
        if (parseResult == rtype::ParseResult::Error) {
            return 1;
        }
        if (parseResult == rtype::ParseResult::Exit) {
            return 0;
        }
        printBanner(config);
        setupSignalHandlers();
        return runServer(config, ServerSignals::shutdown(),
                         ServerSignals::reloadConfig());
    } catch (const std::exception& e) {
        std::cerr << "[Main] Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "[Main] Unknown fatal error occurred" << std::endl;
        return 1;
    }
}
