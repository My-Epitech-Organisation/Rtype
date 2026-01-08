/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** PowerUpTypeComponent - Power-up variant identification for network sync
*/

#pragma once

#include <cstdint>
#include <string>

namespace rtype::games::rtype::shared {

/**
 * @enum PowerUpVariant
 * @brief Power-up type variants for network protocol and visual distinction
 */
enum class PowerUpVariant : uint8_t {
    SpeedBoost = 0,
    Shield = 1,
    RapidFire = 2,
    DoubleDamage = 3,
    HealthBoost = 4,
    WeaponUpgrade = 5,
    ExtraLife = 6,
    Unknown = 255
};

/**
 * @struct PowerUpTypeComponent
 * @brief Component to identify power-up variant across server and client
 */
struct PowerUpTypeComponent {
    PowerUpVariant variant{PowerUpVariant::Unknown};

    /**
     * @brief Convert config ID string to enum variant
     */
    static PowerUpVariant stringToVariant(const std::string& id) {
        if (id == "speed_boost") return PowerUpVariant::SpeedBoost;
        if (id == "shield") return PowerUpVariant::Shield;
        if (id == "rapid_fire") return PowerUpVariant::RapidFire;
        if (id == "double_damage") return PowerUpVariant::DoubleDamage;
        if (id == "health_small" || id == "health_large")
            return PowerUpVariant::HealthBoost;
        if (id == "weapon_upgrade") return PowerUpVariant::WeaponUpgrade;
        if (id == "extra_life") return PowerUpVariant::ExtraLife;
        return PowerUpVariant::Unknown;
    }

    /**
     * @brief Convert enum variant to config ID string
     */
    static std::string variantToString(PowerUpVariant variant) {
        switch (variant) {
            case PowerUpVariant::SpeedBoost:
                return "speed_boost";
            case PowerUpVariant::Shield:
                return "shield";
            case PowerUpVariant::RapidFire:
                return "rapid_fire";
            case PowerUpVariant::DoubleDamage:
                return "double_damage";
            case PowerUpVariant::HealthBoost:
                return "health_small";
            case PowerUpVariant::WeaponUpgrade:
                return "weapon_upgrade";
            case PowerUpVariant::ExtraLife:
                return "extra_life";
            default:
                return "health_small";
        }
    }
};

}  // namespace rtype::games::rtype::shared
