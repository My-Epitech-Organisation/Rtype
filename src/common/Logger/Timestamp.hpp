/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Timestamp - Timestamp utilities for logger
*/

#ifndef SRC_COMMON_LOGGER_TIMESTAMP_HPP_
#define SRC_COMMON_LOGGER_TIMESTAMP_HPP_

#include <chrono>
#include <ctime>
#include <format>
#include <string>

#ifdef _WIN32
    #define RTYPE_LOCALTIME(time_ptr, result_ptr) localtime_s(result_ptr, time_ptr)
#else
    #define RTYPE_LOCALTIME(time_ptr, result_ptr) localtime_r(time_ptr, result_ptr)
#endif

namespace rtype {

/**
 * @brief Utility class for timestamp generation
 */
class Timestamp {
 public:
    /**
     * @brief Get current timestamp as formatted string
     * @return Formatted timestamp string (YYYY-MM-DD HH:MM:SS.mmm)
     */
    [[nodiscard]] static std::string now() {
        const auto now = std::chrono::system_clock::now();
        const auto time = std::chrono::system_clock::to_time_t(now);
        const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::tm timeInfo{};
        RTYPE_LOCALTIME(&time, &timeInfo);

        return std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:03d}",
            timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday,
            timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec,
            static_cast<int>(millis.count()));
    }
};

}  // namespace rtype

#endif  // SRC_COMMON_LOGGER_TIMESTAMP_HPP_
