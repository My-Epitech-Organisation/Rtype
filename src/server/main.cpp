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
#include "lobby/LobbyManager.hpp"

/**
 * @brief Signal handler for graceful shutdown and config reload
 * @param signal The signal received
 */
static void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        LOG_INFO_CAT(rtype::LogCategory::Main,
                     "\n[Main] Received shutdown signal");
        ServerSignals::shutdown()->store(true);
    }
#ifndef _WIN32
    if (signal == SIGHUP) {
        LOG_INFO_CAT(rtype::LogCategory::Main,
                     "\n[Main] Received SIGHUP - config reload requested");
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
        .flag("-v", "--verbose",
              "Enable verbose debug output for all categories",
              [config]() {
                  config->verbose = true;
                  config->verboseCategories = rtype::LogCategory::All;
                  return rtype::ParseResult::Success;
              })
        .option("-vc", "--verbose-category", "category",
                "Enable verbose output for specific categories "
                "(main,network,game,ecs,input,audio,graphics,physics,ai,ui). "
                "Can be specified multiple times.",
                [config](std::string_view val) {
                    rtype::LogCategory cat = rtype::categoryFromString(val);
                    if (cat == rtype::LogCategory::None) {
                        LOG_ERROR_CAT(rtype::LogCategory::Main,
                                      "Unknown category: " << val);
                        return rtype::ParseResult::Error;
                    }
                    config->verbose = true;
                    if (config->verboseCategories == rtype::LogCategory::All) {
                        config->verboseCategories = cat;
                    } else {
                        config->verboseCategories |= cat;
                    }
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
                })
        .option("-i", "--instances", "n",
                "Number of lobby instances (1-16, default: 1)",
                [config](std::string_view val) {
                    auto v =
                        rtype::parseNumber<uint32_t>(val, "instances", 1, 16);
                    if (!v.has_value()) return rtype::ParseResult::Error;
                    config->instanceCount = v.value();
                    return rtype::ParseResult::Success;
                })
        .option("", "--lobby-timeout", "seconds",
                "Empty lobby timeout in seconds (default: 300)",
                [config](std::string_view val) {
                    auto v = rtype::parseNumber<uint32_t>(val, "lobby-timeout",
                                                          10, 3600);
                    if (!v.has_value()) return rtype::ParseResult::Error;
                    config->lobbyTimeout = v.value();
                    return rtype::ParseResult::Success;
                });
    return parser;
}

/**
 * @brief Print the server startup banner with configuration
 * @param config The server configuration to display
 */
static void printBanner(const ServerConfig& config) {
    LOG_INFO_CAT(
        rtype::LogCategory::Main,
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
            << std::format("  Instances:   {}\n", config.instanceCount)
            << std::format("  Lobby Timeout: {} seconds\n", config.lobbyTimeout)
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
    // TODO(Clem): Implement hot reload in main loop when reloadConfigFlag
    (void)reloadConfigFlag;

    if (config.instanceCount >= 1) {
        LOG_INFO_CAT(rtype::LogCategory::Main,
                     "[Main] Starting lobby manager with "
                         << config.instanceCount << " instance(s)");

        rtype::server::LobbyManager::Config managerConfig;
        managerConfig.basePort = config.port;
        managerConfig.instanceCount = config.instanceCount;
        managerConfig.maxPlayers = config.maxPlayers;
        managerConfig.tickRate = config.tickRate;
        managerConfig.configPath = config.configPath;
        managerConfig.emptyTimeout = std::chrono::seconds(config.lobbyTimeout);
        managerConfig.maxInstances = 16;

        try {
            rtype::server::LobbyManager manager(managerConfig);

            if (!manager.start()) {
                LOG_ERROR_CAT(rtype::LogCategory::Main,
                              "[Main] Failed to start lobby manager");
                return 1;
            }

            while (!shutdownFlag->load() && manager.isRunning()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            LOG_INFO_CAT(
                rtype::LogCategory::Main,
                "[Main] Shutdown signal received, stopping lobbies...");

            manager.stop();

            LOG_INFO_CAT(rtype::LogCategory::Main,
                         "[Main] Lobby manager terminated.");
            return 0;
        } catch (const std::exception& e) {
            LOG_ERROR_CAT(rtype::LogCategory::Main,
                          "[Main] Lobby manager error: " << e.what());
            return 1;
        }
    }
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
            logger.setEnabledCategories(config->verboseCategories);
        } else {
            logger.setLogLevel(rtype::LogLevel::Info);
        }

        if (config->noColor) {
            logger.setColorEnabled(false);
        }
        const auto logFile =
            rtype::Logger::generateLogFilename("server_session");
        if (logger.setLogFile(logFile, false)) {
            LOG_INFO_CAT(rtype::LogCategory::Main,
                         "[Main] Logging to file: " << logFile.string());
        } else {
            LOG_WARNING_CAT(
                rtype::LogCategory::Main,
                "[Main] Failed to open log file: " << logFile.string());
        }

        printBanner(*config);
        setupSignalHandlers();
        return runServer(*config, ServerSignals::shutdown(),
                         ServerSignals::reloadConfig());
    } catch (const std::exception& e) {
        LOG_FATAL_CAT(rtype::LogCategory::Main,
                      "[Main] Fatal error: " << std::string(e.what()));
        return 1;
    } catch (...) {
        LOG_FATAL_CAT(rtype::LogCategory::Main,
                      "[Main] Unknown fatal error occurred");
        return 1;
    }
}
