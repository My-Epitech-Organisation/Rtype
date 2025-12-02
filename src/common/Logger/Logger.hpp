/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Logger - Main logger class
*/

#ifndef SRC_COMMON_LOGGER_LOGGER_HPP_
#define SRC_COMMON_LOGGER_LOGGER_HPP_

#include <format>
#include <functional>
#include <iostream>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>

#include "FileWriter.hpp"
#include "LogLevel.hpp"
#include "Timestamp.hpp"

namespace rtype {

/**
 * @brief Thread-safe logger with configurable levels and file output
 *
 * Provides synchronized logging with timestamps across multiple threads.
 * Uses RAII lock_guard for automatic mutex management.
 *
 * Features:
 * - Configurable log levels
 * - Optional file output
 * - Thread-safe operations
 * - Timestamps with millisecond precision
 * - Uses C++20 std::format for formatting
 *
 * @note For unit testing, use Logger::setInstance() to inject a mock logger
 */
class Logger {
   public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the logger instance
     */
    static Logger& instance() {
        if (_customInstance.has_value()) {
            return _customInstance->get();
        }
        static Logger logger;
        return logger;
    }

    /**
     * @brief Set a custom logger instance (useful for testing)
     * @param logger Reference to custom logger instance
     */
    static void setInstance(Logger& logger) {
        _customInstance = std::ref(logger);
    }

    /**
     * @brief Reset to default singleton instance
     */
    static void resetInstance() { _customInstance.reset(); }

    Logger() = default;
    virtual ~Logger() = default;

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    /**
     * @brief Set the minimum log level
     * @param level Minimum level to log (messages below this level are ignored)
     */
    void setLogLevel(LogLevel level) noexcept {
        std::lock_guard<std::mutex> lock(_mutex);
        _logLevel = level;
    }

    /**
     * @brief Get the current log level
     * @return Current minimum log level
     */
    [[nodiscard]] LogLevel getLogLevel() const noexcept { return _logLevel; }

    /**
     * @brief Enable file logging
     * @param filepath Path to the log file
     * @param append If true, append to existing file; otherwise overwrite
     * @return true if file was opened successfully
     */
    bool setLogFile(const std::filesystem::path& filepath, bool append = true) {
        std::lock_guard<std::mutex> lock(_mutex);
        return _fileWriter.open(filepath, append);
    }

    /**
     * @brief Close the log file
     */
    void closeFile() {
        std::lock_guard<std::mutex> lock(_mutex);
        _fileWriter.close();
    }

    /**
     * @brief Check if file logging is enabled
     * @return true if logging to file
     */
    [[nodiscard]] bool isFileLoggingEnabled() const noexcept {
        return _fileWriter.isOpen();
    }

    /**
     * @brief Log a debug message
     * @param msg The message to log
     */
    virtual void debug(const std::string& msg) { log(LogLevel::Debug, msg); }

    /**
     * @brief Log an info message
     * @param msg The message to log
     */
    virtual void info(const std::string& msg) { log(LogLevel::Info, msg); }

    /**
     * @brief Log a warning message
     * @param msg The message to log
     */
    virtual void warning(const std::string& msg) {
        log(LogLevel::Warning, msg);
    }

    /**
     * @brief Log an error message
     * @param msg The message to log
     */
    virtual void error(const std::string& msg) { log(LogLevel::Error, msg); }

   protected:
    mutable std::mutex _mutex;
    LogLevel _logLevel{LogLevel::Debug};

   private:
    static inline std::optional<std::reference_wrapper<Logger>> _customInstance{
        std::nullopt};
    FileWriter _fileWriter;

    /**
     * @brief Internal logging function with level check and mutex protection
     * @param level Log level for this message
     * @param msg Message to log
     */
    void log(LogLevel level, const std::string& msg) {
        std::lock_guard<std::mutex> lock(_mutex);

        if (level < _logLevel) {
            return;
        }
        const std::string formattedMsg =
            std::format("[{}] [{}] {}", Timestamp::now(), toString(level), msg);

        std::ostream& consoleStream =
            (level >= LogLevel::Warning) ? std::cerr : std::cout;
        consoleStream << formattedMsg << '\n';

        _fileWriter.write(formattedMsg);
    }
};

}  // namespace rtype

#endif  // SRC_COMMON_LOGGER_LOGGER_HPP_
