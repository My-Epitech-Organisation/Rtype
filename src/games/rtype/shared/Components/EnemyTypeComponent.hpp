/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** EnemyTypeComponent - Identifies specific enemy variant
*/

#pragma once

#include <cstdint>
#include <string>

namespace rtype::games::rtype::shared {

/**
 * @enum EnemyVariant
 * @brief Enumeration of enemy subtypes for visual/behavior distinction
 */
enum class EnemyVariant : std::uint8_t {
    Basic = 0,     // Basic enemy with straight movement
    Shooter = 1,   // Enemy that shoots at players
    Chaser = 2,    // Enemy that follows players
    Wave = 3,      // Enemy with wave movement pattern
    Patrol = 4,    // Enemy that patrols between points
    Heavy = 5,     // Slow but tanky enemy
    Boss = 6,      // Boss enemy
    Unknown = 255  // Fallback for unrecognized types
};

/**
 * @brief Component to identify the specific variant of an enemy
 *
 * Used by both server and client to distinguish between enemy types
 * for proper sprite selection and behavior identification.
 */
struct EnemyTypeComponent {
    EnemyVariant variant{EnemyVariant::Basic};
    std::string configId;

    EnemyTypeComponent() = default;
    explicit EnemyTypeComponent(EnemyVariant var, std::string id = "")
        : variant(var), configId(std::move(id)) {}

    /**
     * @brief Convert config ID string to enum variant
     */
    static EnemyVariant stringToVariant(const std::string& id) {
        if (id == "basic") return EnemyVariant::Basic;
        if (id == "shooter") return EnemyVariant::Shooter;
        if (id == "chaser") return EnemyVariant::Chaser;
        if (id == "wave") return EnemyVariant::Wave;
        if (id == "patrol") return EnemyVariant::Patrol;
        if (id == "heavy") return EnemyVariant::Heavy;
        if (id == "boss" || id == "boss_1") return EnemyVariant::Boss;
        return EnemyVariant::Unknown;
    }

    /**
     * @brief Convert enum variant to config ID string
     */
    static std::string variantToString(EnemyVariant variant) {
        switch (variant) {
            case EnemyVariant::Basic:
                return "basic";
            case EnemyVariant::Shooter:
                return "shooter";
            case EnemyVariant::Chaser:
                return "chaser";
            case EnemyVariant::Wave:
                return "wave";
            case EnemyVariant::Patrol:
                return "patrol";
            case EnemyVariant::Heavy:
                return "heavy";
            case EnemyVariant::Boss:
                return "boss_1";
            default:
                return "basic";
        }
    }
};

}  // namespace rtype::games::rtype::shared
