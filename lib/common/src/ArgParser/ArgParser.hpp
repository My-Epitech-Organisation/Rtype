/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ArgParser - Command-line argument parser
*/

#ifndef SRC_COMMON_ARGPARSER_ARGPARSER_HPP_
#define SRC_COMMON_ARGPARSER_ARGPARSER_HPP_

// Prevent Windows.h from defining min/max macros
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <algorithm>
#include <format>
#include <utility>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../Logger/Macros.hpp"
#include "Option.hpp"
#include "ParseResult.hpp"

namespace rtype {

/**
 * @brief Simple command line argument parser
 *
 * Uses shared_ptr for handlers to avoid duplication and enable safe sharing.
 * Provides fluent API for easy configuration.
 *
 * Supports:
 * - Flag options (--help, -h)
 * - Value options (--port 4242, -p 4242)
 * - Positional arguments (config.toml)
 *
 * Handler return values:
 * - For flags: return ParseResult::Exit for --help, ParseResult::Success to
 * continue
 * - For options: return ParseResult::Error on validation failure,
 * ParseResult::Success otherwise
 * - For positional: return ParseResult::Error on validation failure,
 * ParseResult::Success otherwise
 */
class ArgParser {
   public:
    /**
     * @brief Destructor - clears handler maps to avoid keeping lambdas alive
     */
    ~ArgParser() { clear(); }

    /**
     * @brief Clear all handlers and options
     */
    void clear() {
        _flagHandlers.clear();
        _valueHandlers.clear();
        _positionalHandlers.clear();
        _options.clear();
        _positionalArgs.clear();
    }

    /**
     * @brief Add a flag option (no argument)
     * @param shortOpt Short option (e.g., "-h")
     * @param longOpt Long option (e.g., "--help")
     * @param description Description for help message
     * @param handler Function to call when flag is found (returns ParseResult)
     * @return Reference to this parser for chaining
     */
    ArgParser& flag(std::string_view shortOpt, std::string_view longOpt,
                    std::string_view description,
                    std::function<ParseResult()> handler) {
        if (isDuplicate(shortOpt, longOpt)) {
            LOG_WARNING(
                std::format("Duplicate option: {}/{}", shortOpt, longOpt));
            return *this;
        }
        _options.push_back({std::string(shortOpt), std::string(longOpt),
                            std::string(description), false, ""});
        auto sharedHandler =
            std::make_shared<std::function<ParseResult()>>(std::move(handler));
        _flagHandlers[std::string(shortOpt)] = sharedHandler;
        _flagHandlers[std::string(longOpt)] = sharedHandler;
        return *this;
    }

    /**
     * @brief Add an option with argument
     * @param shortOpt Short option (e.g., "-p")
     * @param longOpt Long option (e.g., "--port")
     * @param argName Name of the argument (e.g., "port")
     * @param description Description for help message
     * @param handler Function to call with the argument value (returns
     * ParseResult)
     * @return Reference to this parser for chaining
     */
    ArgParser& option(std::string_view shortOpt, std::string_view longOpt,
                      std::string_view argName, std::string_view description,
                      std::function<ParseResult(std::string_view)> handler) {
        if (isDuplicate(shortOpt, longOpt)) {
            LOG_WARNING(
                std::format("Duplicate option: {}/{}", shortOpt, longOpt));
            return *this;
        }
        _options.push_back({std::string(shortOpt), std::string(longOpt),
                            std::string(description), true,
                            std::string(argName)});
        auto sharedHandler =
            std::make_shared<std::function<ParseResult(std::string_view)>>(
                std::move(handler));
        _valueHandlers[std::string(shortOpt)] = sharedHandler;
        _valueHandlers[std::string(longOpt)] = sharedHandler;
        return *this;
    }

    /**
     * @brief Add a positional argument
     * @param name Name of the argument (e.g., "config")
     * @param description Description for help message
     * @param handler Function to call with the argument value (returns
     * ParseResult)
     * @param required Whether this argument is required (default: true)
     * @return Reference to this parser for chaining
     */
    ArgParser& positional(std::string_view name, std::string_view description,
                          std::function<ParseResult(std::string_view)> handler,
                          bool required = true) {
        _positionalArgs.push_back(
            {std::string(name), std::string(description), required});
        _positionalHandlers.push_back(
            std::make_shared<std::function<ParseResult(std::string_view)>>(
                std::move(handler)));
        return *this;
    }

    /**
     * @brief Parse command line arguments
     * @param args Vector of argument strings (excluding program name)
     * @return ParseResult::Success if parsing succeeded,
     *         ParseResult::Exit for early exit (e.g., --help),
     *         ParseResult::Error on error
     */
    [[nodiscard]] ParseResult parse(
        const std::vector<std::string_view>& args) const {
        std::vector<std::string_view> positionalValues;
        auto iter = args.begin();

        while (iter != args.end()) {
            const std::string key(*iter);
            if (!key.empty() && key[0] == '-') {
                auto [result, consumed] = parseOption(key, iter, args.end());
                if (result != ParseResult::Success) {
                    return result;
                }
                iter += consumed;
            } else {
                positionalValues.push_back(*iter);
            }
            iter++;
        }
        return processPositionalArgs(positionalValues);
    }

    /**
     * @brief Set the program name for usage message
     * @param name Program name to display
     * @return Reference to this parser for chaining
     */
    ArgParser& programName(std::string_view name) {
        _programName = name;
        return *this;
    }

    /**
     * @brief Print usage message with properly aligned options
     */
    void printUsage() const {
        printUsageLine();
        printOptions();
        printPositionalArgs();
        std::cout << std::flush;
    }

   private:
    /**
     * @brief Parse a single option
     * @return Pair of (ParseResult, number of additional arguments consumed)
     */
    [[nodiscard]] std::pair<ParseResult, int> parseOption(
        const std::string& key,
        std::vector<std::string_view>::const_iterator iter,
        std::vector<std::string_view>::const_iterator end) const {
        if (auto handlerIt = _flagHandlers.find(key);
            handlerIt != _flagHandlers.end()) {
            return {(*handlerIt->second)(), 0};
        }
        if (auto handlerIt = _valueHandlers.find(key);
            handlerIt != _valueHandlers.end()) {
            iter++;
            if (iter == end) {
                LOG_ERROR(std::format("Option {} requires an argument", key));
                return {ParseResult::Error, 0};
            }
            return {(*handlerIt->second)(*iter), 1};
        }
        LOG_ERROR(std::format("Unknown option: {}", key));
        printUsage();
        return {ParseResult::Error, 0};
    }

    /**
     * @brief Process positional arguments
     */
    [[nodiscard]] ParseResult processPositionalArgs(
        const std::vector<std::string_view>& positionalValues) const {
        for (std::size_t i = 0; i < _positionalArgs.size(); ++i) {
            if (i < positionalValues.size()) {
                ParseResult result =
                    (*_positionalHandlers[i])(positionalValues[i]);
                if (result != ParseResult::Success) {
                    return result;
                }
            } else if (_positionalArgs[i].required) {
                LOG_ERROR(std::format("Missing required argument: {}",
                                      _positionalArgs[i].name));
                printUsage();
                return ParseResult::Error;
            }
        }
        if (positionalValues.size() > _positionalArgs.size() &&
            !_positionalArgs.empty()) {
            LOG_WARNING(std::format(
                "Extra positional arguments ignored (got {}, expected {})",
                positionalValues.size(), _positionalArgs.size()));
        }
        return ParseResult::Success;
    }

    /**
     * @brief Print the usage line
     */
    void printUsageLine() const {
        std::cout << "Usage: " << _programName << " [options]";
        for (const auto& posArg : _positionalArgs) {
            if (posArg.required) {
                std::cout << " <" << posArg.name << ">";
            } else {
                std::cout << " [" << posArg.name << "]";
            }
        }
        std::cout << "\n";
    }

    /**
     * @brief Print options section
     */
    void printOptions() const {
        if (_options.empty()) {
            return;
        }
        std::cout << "Options:\n";
        std::size_t maxWidth = 0;
        for (const auto& opt : _options) {
            std::size_t width = opt.shortOpt.size() + opt.longOpt.size() + 2;
            if (opt.hasArg) {
                width += opt.argName.size() + 3;
            }
            maxWidth = (std::max)(maxWidth, width);
        }
        for (const auto& opt : _options) {
            std::ostringstream optStr;
            optStr << "  " << opt.shortOpt << ", " << opt.longOpt;
            if (opt.hasArg) {
                optStr << " <" << opt.argName << ">";
            }
            std::cout << std::left << std::setw(static_cast<int>(maxWidth + 4))
                      << optStr.str() << opt.description << '\n';
        }
    }

    /**
     * @brief Print positional arguments section
     */
    void printPositionalArgs() const {
        if (_positionalArgs.empty()) {
            return;
        }
        std::cout << "Arguments:\n";
        std::size_t maxWidth = 0;
        for (const auto& posArg : _positionalArgs) {
            maxWidth = (std::max)(maxWidth, posArg.name.size());
        }
        for (const auto& posArg : _positionalArgs) {
            std::cout << "  " << std::left
                      << std::setw(static_cast<int>(maxWidth + 4))
                      << posArg.name << posArg.description;
            if (!posArg.required) {
                std::cout << " (optional)";
            }
            std::cout << '\n';
        }
    }

    /**
     * @brief Check if an option is already registered
     * @param shortOpt Short option to check
     * @param longOpt Long option to check
     * @return true if either option already exists
     */
    [[nodiscard]] bool isDuplicate(std::string_view shortOpt,
                                   std::string_view longOpt) const {
        const std::string shortKey(shortOpt);
        const std::string longKey(longOpt);
        return _flagHandlers.contains(shortKey) ||
               _flagHandlers.contains(longKey) ||
               _valueHandlers.contains(shortKey) ||
               _valueHandlers.contains(longKey);
    }

    std::string _programName = "program";
    std::vector<Option> _options;
    std::vector<PositionalArg> _positionalArgs;
    std::unordered_map<
        std::string,
        std::shared_ptr<std::function<ParseResult(std::string_view)>>>
        _valueHandlers;
    std::unordered_map<std::string,
                       std::shared_ptr<std::function<ParseResult()>>>
        _flagHandlers;
    std::vector<std::shared_ptr<std::function<ParseResult(std::string_view)>>>
        _positionalHandlers;
};

}  // namespace rtype

#endif  // SRC_COMMON_ARGPARSER_ARGPARSER_HPP_
