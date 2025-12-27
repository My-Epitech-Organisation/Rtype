/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** LogCategory - Category-based filtering for logs
*/

#ifndef SRC_COMMON_LOGGER_LOGCATEGORY_HPP_
#define SRC_COMMON_LOGGER_LOGCATEGORY_HPP_

#include <bitset>
#include <string_view>
#include <cstdint>

namespace rtype {

/**
 * @brief Log categories for fine-grained filtering
 *
 * Categories can be combined with bitwise OR to enable multiple categories.
 * Use LogCategory::All to enable all categories.
 */
enum class LogCategory : uint32_t {
    None = 0,
    Main = 1 << 0,        // Main application flow
    Network = 1 << 1,     // Network operations (send/receive packets)
    GameEngine = 1 << 2,  // Game engine logic
    ECS = 1 << 3,         // Entity Component System
    Input = 1 << 4,       // Input handling
    Audio = 1 << 5,       // Audio system
    Graphics = 1 << 6,    // Graphics/rendering
    Physics = 1 << 7,     // Physics system
    AI = 1 << 8,          // AI logic
    UI = 1 << 9,          // User interface
    All = 0xFFFFFFFF      // All categories enabled
};

/**
 * @brief Bitwise OR operator for combining categories
 */
constexpr LogCategory operator|(LogCategory a, LogCategory b) noexcept {
    return static_cast<LogCategory>(static_cast<uint32_t>(a) |
                                    static_cast<uint32_t>(b));
}

/**
 * @brief Bitwise AND operator for checking categories
 */
constexpr LogCategory operator&(LogCategory a, LogCategory b) noexcept {
    return static_cast<LogCategory>(static_cast<uint32_t>(a) &
                                    static_cast<uint32_t>(b));
}

/**
 * @brief Bitwise OR assignment operator
 */
constexpr LogCategory& operator|=(LogCategory& a, LogCategory b) noexcept {
    a = a | b;
    return a;
}

/**
 * @brief Check if a category is enabled in a category mask
 */
constexpr bool isCategoryEnabled(LogCategory mask,
                                 LogCategory category) noexcept {
    return static_cast<uint32_t>(mask & category) != 0;
}

/**
 * @brief Convert LogCategory to string representation
 * @param category The log category
 * @return String representation
 */
[[nodiscard]] inline constexpr std::string_view toString(
    LogCategory category) noexcept {
    switch (category) {
        case LogCategory::None:
            return "None";
        case LogCategory::Main:
            return "Main";
        case LogCategory::Network:
            return "Network";
        case LogCategory::GameEngine:
            return "GameEngine";
        case LogCategory::ECS:
            return "ECS";
        case LogCategory::Input:
            return "Input";
        case LogCategory::Audio:
            return "Audio";
        case LogCategory::Graphics:
            return "Graphics";
        case LogCategory::Physics:
            return "Physics";
        case LogCategory::AI:
            return "AI";
        case LogCategory::UI:
            return "UI";
        case LogCategory::All:
            return "All";
    }
    return "Unknown";
}

/**
 * @brief Parse category from string
 * @param str String representation of category
 * @return LogCategory enum value, or LogCategory::None if not found
 */
[[nodiscard]] inline LogCategory categoryFromString(
    std::string_view str) noexcept {
    if (str == "all" || str == "All" || str == "ALL")
        return LogCategory::All;
    if (str == "main" || str == "Main" || str == "MAIN")
        return LogCategory::Main;
    if (str == "network" || str == "Network" || str == "NETWORK")
        return LogCategory::Network;
    if (str == "gameengine" || str == "GameEngine" || str == "GAMEENGINE" ||
        str == "game" || str == "Game" || str == "GAME")
        return LogCategory::GameEngine;
    if (str == "ecs" || str == "ECS")
        return LogCategory::ECS;
    if (str == "input" || str == "Input" || str == "INPUT")
        return LogCategory::Input;
    if (str == "audio" || str == "Audio" || str == "AUDIO")
        return LogCategory::Audio;
    if (str == "graphics" || str == "Graphics" || str == "GRAPHICS" ||
        str == "gfx" || str == "GFX")
        return LogCategory::Graphics;
    if (str == "physics" || str == "Physics" || str == "PHYSICS")
        return LogCategory::Physics;
    if (str == "ai" || str == "AI")
        return LogCategory::AI;
    if (str == "ui" || str == "UI")
        return LogCategory::UI;
    return LogCategory::None;
}

}  // namespace rtype

#endif  // SRC_COMMON_LOGGER_LOGCATEGORY_HPP_
