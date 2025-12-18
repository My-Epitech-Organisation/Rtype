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
#include <memory>
#include <vector>

#include <rtype/common.hpp>

#include "Logger/Macros.hpp"
#include "ServerApp.hpp"
#include "games/rtype/server/GameEngine.hpp"
#include "games/rtype/server/RTypeEntitySpawner.hpp"
#include "games/rtype/server/RTypeGameConfig.hpp"

/**
 * @brief Signal handler for graceful shutdown and config reload
 * @param signal The signal received
 */
static void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        LOG_INFO("\n[Main] Received shutdown signal");
        ServerSignals::shutdown()->store(true);
    }
#ifndef _WIN32
    if (signal == SIGHUP) {
        LOG_INFO("\n[Main] Received SIGHUP - config reload requested");
        ServerSignals::reloadConfig()->store(true);
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
 * @return Shared pointer to the configured parser
 */
static std::shared_ptr<rtype::ArgParser> configureParser(
    std::shared_ptr<rtype::ArgParser> parser,
    std::shared_ptr<ServerConfig> config) {
    (*parser)
        .flag("-h", "--help", "Show this help message",
              [weak_parser = std::weak_ptr<rtype::ArgParser>(parser)]() {
                  if (auto p = weak_parser.lock()) {
                      p->printUsage();
                  }
                  return rtype::ParseResult::Exit;
              })
        .flag("-v", "--verbose", "Enable verbose debug output",
              [config]() {
                  config->verbose = true;
                  return rtype::ParseResult::Success;
              })
        .flag("-nc", "--no-color", "Disable colored console output",
              [config]() {
                  config->noColor = true;
                  return rtype::ParseResult::Success;
              })
        .option("-c", "--config", "path",
                "Path to configuration directory (default: config/server)",
                [config](std::string_view val) {
                    config->configPath = std::string(val);
                    return rtype::ParseResult::Success;
                })
        .option(
            "-p", "--port", "port", "Server port (1-65535, overrides config)",
            [config](std::string_view val) {
                auto v = rtype::parseNumber<uint16_t>(val, "port", 1, 65535);
                if (!v.has_value()) return rtype::ParseResult::Error;
                config->port = v.value();
                config->portOverride = true;
                return rtype::ParseResult::Success;
            })
        .option("-m", "--max-players", "n",
                "Maximum players (1-256, overrides config)",
                [config](std::string_view val) {
                    auto v =
                        rtype::parseNumber<size_t>(val, "max-players", 1, 256);
                    if (!v.has_value()) return rtype::ParseResult::Error;
                    config->maxPlayers = v.value();
                    config->maxPlayersOverride = true;
                    return rtype::ParseResult::Success;
                })
        .option("-t", "--tick-rate", "hz",
                "Tick rate in Hz (1-1000, overrides config)",
                [config](std::string_view val) {
                    auto v =
                        rtype::parseNumber<uint32_t>(val, "tick-rate", 1, 1000);
                    if (!v.has_value()) return rtype::ParseResult::Error;
                    config->tickRate = v.value();
                    config->tickRateOverride = true;
                    return rtype::ParseResult::Success;
                });
    return parser;
}

/**
 * @brief Print the server startup banner with configuration
 * @param config The server configuration to display
 */
static void printBanner(const ServerConfig& config) {
    LOG_INFO(
        "\n==================================\n"
        << "    R-Type Server\n"
        << "==================================\n"
        << std::format("  Config Dir:  {}\n", config.configPath)
        << std::format("  Port:        {}{}\n", config.port,
                       config.portOverride ? " (override)" : "")
        << std::format("  Max Players: {}{}\n", config.maxPlayers,
                       config.maxPlayersOverride ? " (override)" : "")
        << std::format("  Tick Rate:   {} Hz{}\n", config.tickRate,
                       config.tickRateOverride ? " (override)" : "")
        << std::format("  Verbose:     {}\n", config.verbose ? "yes" : "no")
        << "==================================");
}

/**
 * @brief Run the server with the given configuration
 * @param config The server configuration
 * @param shutdownFlag Shared pointer to the shutdown flag
 * @param reloadConfigFlag Shared pointer to the reload config flag
 * @return Exit code (0 for success, 1 for failure)
 */
static int runServer(const ServerConfig& config,
                     std::shared_ptr<std::atomic<bool>> shutdownFlag,
                     std::shared_ptr<std::atomic<bool>> reloadConfigFlag) {
    auto gameConfig = rtype::games::rtype::server::createRTypeGameConfig();

    if (!gameConfig->initialize(config.configPath)) {
        LOG_ERROR("[Main] Failed to initialize game configuration: "
                  << gameConfig->getLastError());
        return 1;
    }

    rtype::server::ServerApp server(std::move(gameConfig), shutdownFlag,
                                    config.verbose);

    // TODO(Clem): Implement hot reload in main loop when reloadConfigFlag
    (void)reloadConfigFlag;

    if (!server.run()) {
        LOG_ERROR("[Main] Server failed to start.");
        return 1;
    }

    LOG_INFO("[Main] Server terminated.");
    return 0;
}

int main(int argc, char** argv) {
    rtype::games::rtype::server::registerRTypeGameEngine();
    rtype::games::rtype::server::registerRTypeEntitySpawner();

    try {
        auto config = std::make_shared<ServerConfig>();
        std::vector<std::string_view> args(argv + 1, argv + argc);
        {
            auto parser = std::make_shared<rtype::ArgParser>();
            parser->programName(argv[0]);
            configureParser(parser, config);
            rtype::ParseResult parseResult = parser->parse(args);
            if (parseResult == rtype::ParseResult::Error) {
                return 1;
            }
            if (parseResult == rtype::ParseResult::Exit) {
                return 0;
            }
            parser->clear();
        }

        auto& logger = rtype::Logger::instance();

        if (config->verbose) {
            logger.setLogLevel(rtype::LogLevel::Debug);
        } else {
            logger.setLogLevel(rtype::LogLevel::Info);
        }

        if (config->noColor) {
            logger.setColorEnabled(false);
        }
        const auto logFile =
            rtype::Logger::generateLogFilename("server_session");
        if (logger.setLogFile(logFile, false)) {
            LOG_INFO("[Main] Logging to file: " << logFile.string());
        } else {
            LOG_WARNING("[Main] Failed to open log file: " << logFile.string());
        }

        printBanner(*config);
        setupSignalHandlers();
        return runServer(*config, ServerSignals::shutdown(),
                         ServerSignals::reloadConfig());
    } catch (const std::exception& e) {
        LOG_FATAL("[Main] Fatal error: " << std::string(e.what()));
        return 1;
    } catch (...) {
        LOG_FATAL("[Main] Unknown fatal error occurred");
        return 1;
    }
}
