/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Logger - Implementation
*/

#include "Logger.hpp"

namespace rtype {

std::filesystem::path Logger::generateLogFilename(
    const std::string& prefix, const std::filesystem::path& directory) {
    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);

    std::tm timeInfo{};
    RTYPE_LOCALTIME(&time, &timeInfo);

    const std::string filename = std::format(
        "{}_{:04d}-{:02d}-{:02d}_{:02d}-{:02d}-{:02d}.log",
        prefix,
        timeInfo.tm_year + 1900,
        timeInfo.tm_mon + 1,
        timeInfo.tm_mday,
        timeInfo.tm_hour,
        timeInfo.tm_min,
        timeInfo.tm_sec);

    if (!std::filesystem::exists(directory)) {
        std::filesystem::create_directories(directory);
    }

    return directory / filename;
}

bool Logger::setLogFile(const std::filesystem::path& filepath, bool append) {
    std::lock_guard<std::mutex> lock(_mutex);
    return _fileWriter.open(filepath, append);
}

void Logger::log(LogLevel level, const std::string& msg,
                 LogCategory category) {
    std::lock_guard<std::mutex> lock(_mutex);

    if (level < _logLevel) {
        return;
    }

    if (!rtype::isCategoryEnabled(_enabledCategories, category)) {
        return;
    }

    const std::string formattedMsg =
        std::format("[{}] [{}] {}", Timestamp::now(), toString(level), msg);

    const std::string coloredMsg =
        std::format("{}[{}] [{}] {}{}",
                    ColorFormatter::getColor(level),
                    Timestamp::now(),
                    toString(level),
                    msg,
                    ColorFormatter::getReset());

    std::ostream& consoleStream =
        (level >= LogLevel::Warning) ? std::cerr : std::cout;
    consoleStream << coloredMsg << '\n';

    _fileWriter.write(formattedMsg);
}

}  // namespace rtype
