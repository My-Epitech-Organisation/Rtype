/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** PowerUpComponent - Pickup data and active power-up state
*/

#pragma once

#include <cstdint>

namespace rtype::games::rtype::shared {

/**
 * @enum PowerUpType
 * @brief Types of gameplay power-ups
 */
enum class PowerUpType : uint8_t {
    None = 0,
    SpeedBoost,
    Shield,
    RapidFire,
    DoubleDamage,
    HealthBoost,
    ForcePod
};

/**
 * @struct PowerUpComponent
 * @brief Component carried by pickup entities
 */
struct PowerUpComponent {
    PowerUpType type{PowerUpType::None};
    float duration{8.0F};
    float magnitude{1.0F};
};

/**
 * @struct ActivePowerUpComponent
 * @brief Component applied to players while a power-up is active
 */
struct ActivePowerUpComponent {
    PowerUpType type{PowerUpType::None};
    float remainingTime{0.0F};
    float speedMultiplier{1.0F};
    float fireRateMultiplier{1.0F};
    float damageMultiplier{1.0F};
    bool shieldActive{false};
    float originalCooldown{0.0F};
    bool hasOriginalCooldown{false};
};

}  // namespace rtype::games::rtype::shared
