/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** LogLevel - Log level enumeration and utilities
*/

#ifndef SRC_COMMON_LOGGER_LOGLEVEL_HPP_
#define SRC_COMMON_LOGGER_LOGLEVEL_HPP_

#include <string_view>

namespace rtype {

/**
 * @brief Log level enumeration for configurable logging
 */
enum class LogLevel {
  Debug = 0,   ///< Verbose debugging information
  Info = 1,    ///< Informational messages
  Warning = 2, ///< Warning messages
  Error = 3,   ///< Error messages
  None = 4     ///< Disable all logging
};

/**
 * @brief Convert LogLevel to string representation
 * @param level The log level
 * @return String representation
 */
[[nodiscard]] inline constexpr std::string_view
toString(LogLevel level) noexcept {
  switch (level) {
  case LogLevel::Debug:
    return "DEBUG";
  case LogLevel::Info:
    return "INFO";
  case LogLevel::Warning:
    return "WARNING";
  case LogLevel::Error:
    return "ERROR";
  case LogLevel::None:
    return "NONE";
  }
  return "UNKNOWN";
}

} // namespace rtype

#endif // SRC_COMMON_LOGGER_LOGLEVEL_HPP_
