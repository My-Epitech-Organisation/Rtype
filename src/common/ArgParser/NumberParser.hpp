/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** NumberParser - Numeric parsing utilities
*/

#ifndef SRC_COMMON_ARGPARSER_NUMBERPARSER_HPP_
#define SRC_COMMON_ARGPARSER_NUMBERPARSER_HPP_

#include <cstdint>
#include <format>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "../Logger/Macros.hpp"

namespace rtype {

/**
 * @brief Parse a numeric value from a string with range validation
 * @tparam T The numeric type to parse
 * @param str The string to parse
 * @param name The name of the argument (for error messages)
 * @param minVal Minimum valid value
 * @param maxVal Maximum valid value
 * @return The parsed value, or std::nullopt on failure
 */
template <typename T>
[[nodiscard]] std::optional<T> parseNumber(
    std::string_view str, std::string_view name,
    T minVal = std::numeric_limits<T>::min(),
    T maxVal = std::numeric_limits<T>::max()) noexcept {
    try {
        const std::string input(str);
        std::size_t pos = 0;
        const int64_t value = std::stoll(input, &pos);

        if (pos != input.size()) {
            LOG_ERROR(std::format("Invalid {}: '{}' is not a valid number",
                                  name, str));
            return std::nullopt;
        }
        if (value < static_cast<int64_t>(minVal) ||
            value > static_cast<int64_t>(maxVal)) {
            LOG_ERROR(std::format("Invalid {}: must be between {} and {}", name,
                                  minVal, maxVal));
            return std::nullopt;
        }
        return static_cast<T>(value);
    } catch (const std::invalid_argument&) {
        LOG_ERROR(
            std::format("Invalid {}: '{}' is not a valid number", name, str));
        return std::nullopt;
    } catch (const std::out_of_range&) {
        LOG_ERROR(std::format("Invalid {}: value out of range", name));
        return std::nullopt;
    }
}

}  // namespace rtype

#endif  // SRC_COMMON_ARGPARSER_NUMBERPARSER_HPP_
