/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Macros - Logging macros for convenient usage
*/

#ifndef SRC_COMMON_LOGGER_MACROS_HPP_
#define SRC_COMMON_LOGGER_MACROS_HPP_

#include <sstream>

#include "Logger.hpp"

/**
 * @brief Helper macro to convert stream expressions to string
 *
 * Uses variadic macro to properly handle expressions containing commas
 * (e.g., template arguments, initializer lists).
 *
 * @note The message expression is evaluated exactly once.
 * @note Expressions with unparenthesized commas (e.g., std::pair{1, 2})
 *       are supported thanks to the variadic macro.
 */
#define LOG_TO_STRING(...)                                                   \
    (static_cast<std::ostringstream&&>(std::ostringstream() << __VA_ARGS__)) \
        .str()

/**
 * @brief Debug logging macro - only prints in debug builds
 *
 * Use for verbose debugging information that shouldn't appear in production.
 * Thread-safe with timestamp.
 *
 * @param ... The message to log (supports << chaining and expressions with
 * commas)
 */
#define LOG_DEBUG(...) \
    ::rtype::Logger::instance().debug(LOG_TO_STRING(__VA_ARGS__))

/**
 * @brief Info logging macro - always prints to stdout
 *
 * Use for important operational messages that should always be visible.
 * Thread-safe with timestamp.
 *
 * @param ... The message to log (supports << chaining and expressions with
 * commas)
 */
#define LOG_INFO(...) \
    ::rtype::Logger::instance().info(LOG_TO_STRING(__VA_ARGS__))

/**
 * @brief Warning logging macro - always prints to stderr
 *
 * Use for warning messages that indicate potential issues.
 * Thread-safe with timestamp.
 *
 * @param ... The message to log (supports << chaining and expressions with
 * commas)
 */
#define LOG_WARNING(...) \
    ::rtype::Logger::instance().warning(LOG_TO_STRING(__VA_ARGS__))

/**
 * @brief Error logging macro - always prints to stderr
 *
 * Use for error messages that indicate failures.
 * Thread-safe with timestamp.
 *
 * @param ... The message to log (supports << chaining and expressions with
 * commas)
 */
#define LOG_ERROR(...) \
    ::rtype::Logger::instance().error(LOG_TO_STRING(__VA_ARGS__))

/**
 * @brief Fatal error logging macro - always prints to stderr
 *
 * Use for critical error messages that indicate fatal failures.
 * Thread-safe with timestamp.
 *
 * @param ... The message to log (supports << chaining and expressions with
 * commas)
 */
#define LOG_FATAL(...) \
    ::rtype::Logger::instance().fatal(LOG_TO_STRING(__VA_ARGS__))

#endif  // SRC_COMMON_LOGGER_MACROS_HPP_
