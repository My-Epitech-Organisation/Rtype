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
 */
#define LOG_TO_STRING(msg) \
    (static_cast<std::ostringstream&&>(std::ostringstream() << msg)).str()

/**
 * @brief Debug logging macro - only prints in debug builds
 *
 * This macro is disabled when NDEBUG is defined (Release builds).
 * Use for verbose debugging information that shouldn't appear in production.
 * Thread-safe with timestamp.
 *
 * @param msg The message to log (supports << chaining)
 */
#ifdef NDEBUG
#define LOG_DEBUG(msg) ((void)0)
#else
#define LOG_DEBUG(msg) rtype::Logger::instance().debug(LOG_TO_STRING(msg))
#endif

/**
 * @brief Info logging macro - always prints to stdout
 *
 * Use for important operational messages that should always be visible.
 * Thread-safe with timestamp.
 *
 * @param msg The message to log (supports << chaining)
 */
#define LOG_INFO(msg) rtype::Logger::instance().info(LOG_TO_STRING(msg))

/**
 * @brief Warning logging macro - always prints to stderr
 *
 * Use for warning messages that indicate potential issues.
 * Thread-safe with timestamp.
 *
 * @param msg The message to log (supports << chaining)
 */
#define LOG_WARNING(msg) rtype::Logger::instance().warning(LOG_TO_STRING(msg))

/**
 * @brief Error logging macro - always prints to stderr
 *
 * Use for error messages that indicate failures.
 * Thread-safe with timestamp.
 *
 * @param msg The message to log (supports << chaining)
 */
#define LOG_ERROR(msg) rtype::Logger::instance().error(LOG_TO_STRING(msg))

#endif  // SRC_COMMON_LOGGER_MACROS_HPP_
