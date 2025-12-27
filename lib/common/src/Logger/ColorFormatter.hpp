/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ColorFormatter - ANSI color codes for console output
*/

#ifndef SRC_COMMON_LOGGER_COLORFORMATTER_HPP_
#define SRC_COMMON_LOGGER_COLORFORMATTER_HPP_

#include <string_view>

#include "LogLevel.hpp"

namespace rtype {

/**
 * @brief ANSI color codes for terminal output
 */
namespace AnsiColors {
constexpr std::string_view RESET = "\033[0m";
constexpr std::string_view CYAN = "\033[36m";
constexpr std::string_view GREEN = "\033[32m";
constexpr std::string_view YELLOW = "\033[33m";
constexpr std::string_view RED = "\033[31m";
constexpr std::string_view BRIGHT_RED = "\033[91m";
}  // namespace AnsiColors

/**
 * @brief Utility class for colorizing log output
 *
 * Provides ANSI color codes for different log levels.
 * Colors are disabled on Windows by default unless explicitly enabled.
 */
class ColorFormatter {
   public:
    /**
     * @brief Check if colors are currently enabled
     * @return true if colors are enabled
     */
    static bool isEnabled() noexcept { return _enabled; }

    /**
     * @brief Enable or disable colored output
     * @param enabled true to enable colors, false to disable
     */
    static void setEnabled(bool enabled) noexcept { _enabled = enabled; }

    /**
     * @brief Get the ANSI color code for a log level
     * @param level The log level
     * @return ANSI color code string (empty if colors disabled)
     */
    [[nodiscard]] static std::string_view getColor(LogLevel level) noexcept {
        if (!_enabled) {
            return "";
        }

        switch (level) {
            case LogLevel::Debug:
                return AnsiColors::CYAN;
            case LogLevel::Info:
                return AnsiColors::GREEN;
            case LogLevel::Warning:
                return AnsiColors::YELLOW;
            case LogLevel::Error:
                return AnsiColors::RED;
            case LogLevel::Fatal:
                return AnsiColors::BRIGHT_RED;
            case LogLevel::None:
                return "";
        }
        return "";
    }

    /**
     * @brief Get the ANSI reset code
     * @return ANSI reset code (empty if colors disabled)
     */
    [[nodiscard]] static std::string_view getReset() noexcept {
        return _enabled ? AnsiColors::RESET : "";
    }

   private:
#ifdef _WIN32
    static inline bool _enabled = false;  // Disabled by default on Windows
#else
    static inline bool _enabled = true;  // Enabled by default on Unix/Linux
#endif
};

}  // namespace rtype

#endif  // SRC_COMMON_LOGGER_COLORFORMATTER_HPP_
