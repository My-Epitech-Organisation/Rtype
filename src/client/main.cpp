/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** Client main entry point
*/

#include <exception>
#include <memory>
#include <string>
#include <vector>

#include <rtype/common.hpp>

#include "ClientApp.hpp"
#include "Graphic/ControllerRumble.hpp"
#include "Logger/Macros.hpp"

/**
 * @brief Configure the argument parser with client options
 * @param parser The argument parser to configure
 * @param config The client configuration to populate
 * @param verbose Flag to enable verbose output
 * @param noColor Flag to disable colored output
 * @return Shared pointer to the configured parser
 */
static std::shared_ptr<rtype::ArgParser> configureParser(
    std::shared_ptr<rtype::ArgParser> parser, ClientApp::Config& config,
    bool& verbose, bool& noColor, rtype::LogCategory& verboseCategories) {
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
              [&verbose, &verboseCategories]() {
                  verbose = true;
                  verboseCategories = rtype::LogCategory::All;
                  return rtype::ParseResult::Success;
              })
        .option("-vc", "--verbose-category", "category",
                "Enable verbose output for specific categories "
                "(main,network,game,ecs,input,audio,graphics,physics,ai,ui). "
                "Can be specified multiple times.",
                [&verbose, &verboseCategories](std::string_view val) {
                    rtype::LogCategory cat = rtype::categoryFromString(val);
                    if (cat == rtype::LogCategory::None) {
                        LOG_ERROR_CAT(rtype::LogCategory::Main,
                                      "Unknown category: " << val);
                        return rtype::ParseResult::Error;
                    }
                    verbose = true;
                    if (verboseCategories == rtype::LogCategory::All) {
                        verboseCategories = cat;
                    } else {
                        verboseCategories |= cat;
                    }
                    return rtype::ParseResult::Success;
                })
        .flag("-nc", "--no-color", "Disable colored console output",
              [&noColor]() {
                  noColor = true;
                  return rtype::ParseResult::Success;
              })
        .option("-s", "--server", "server host",
                "Server hostname or IP address (default: 127.0.0.1)",
                [&config](std::string_view val) {
                    config.defaultServerHost = std::string(val);
                    return rtype::ParseResult::Success;
                })
        .option("-p", "--port", "port", "Server port (1-65535, default: 4242)",
                [&config](std::string_view val) {
                    auto v =
                        rtype::parseNumber<uint16_t>(val, "port", 1, 65535);
                    if (!v.has_value()) return rtype::ParseResult::Error;
                    config.defaultServerPort = v.value();
                    return rtype::ParseResult::Success;
                });
    return parser;
}

auto main(int argc, char** argv) -> int {
    try {
        ClientApp::Config config;
        bool verbose = false;
        bool noColor = false;
        rtype::LogCategory verboseCategories = rtype::LogCategory::All;

        std::vector<std::string_view> args(argv + 1, argv + argc);
        {
            auto parser = std::make_shared<rtype::ArgParser>();
            parser->programName(argv[0]);
            configureParser(parser, config, verbose, noColor,
                            verboseCategories);
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

        if (verbose) {
            logger.setLogLevel(rtype::LogLevel::Debug);
            logger.setEnabledCategories(verboseCategories);
        } else {
            logger.setLogLevel(rtype::LogLevel::Info);
        }

        if (noColor) {
            logger.setColorEnabled(false);
        }

        const auto logFile =
            rtype::Logger::generateLogFilename("client_session");
        if (logger.setLogFile(logFile, false)) {
            LOG_INFO_CAT(rtype::LogCategory::Main,
                         "[Main] Logging to file: " << logFile.string());
        } else {
            LOG_WARNING_CAT(
                rtype::LogCategory::Main,
                "[Main] Failed to open log file: " << logFile.string());
        }

        LOG_INFO_CAT(rtype::LogCategory::Main,
                     "[Main] Starting R-Type client...");
        LOG_DEBUG_CAT(rtype::LogCategory::Main,
                      "[Main] Server: " << config.defaultServerHost << ":"
                                        << config.defaultServerPort);

        ClientApp client(config);
        client.run();

        ControllerRumble::cleanup();
        LOG_INFO_CAT(rtype::LogCategory::Main,
                     "[Main] Client terminated normally");
    } catch (const std::exception& e) {
        LOG_FATAL_CAT(rtype::LogCategory::Main,
                      std::string("Program exited with an error: ") +
                          std::string(e.what()));
        ControllerRumble::cleanup();
        return 1;
    }
    return 0;
}
