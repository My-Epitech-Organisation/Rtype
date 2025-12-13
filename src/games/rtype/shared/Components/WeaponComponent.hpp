/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** WeaponComponent - Defines weapon configurations
*/

#pragma once

#include <array>
#include <cstdint>

#include "ProjectileComponent.hpp"

namespace rtype::games::rtype::shared {

/**
 * @struct WeaponConfig
 * @brief Configuration for a single weapon type
 */
struct WeaponConfig {
    ProjectileType projectileType = ProjectileType::BasicBullet;
    int32_t damage = 25;
    float speed = 500.0F;
    float cooldown = 0.25F;
    float lifetime = 3.0F;
    float hitboxWidth = 16.0F;
    float hitboxHeight = 8.0F;
    bool piercing = false;
    int32_t maxHits = 1;
    uint8_t projectileCount = 1;  ///< Number of projectiles per shot
    float spreadAngle = 0.0F;     ///< Angle spread for multi-shot
};

/**
 * @brief Default weapon configurations for variety
 */
namespace WeaponPresets {

/**
 * @brief Basic bullet weapon - fast fire rate, moderate damage
 */
inline constexpr WeaponConfig BasicBullet{
    ProjectileType::BasicBullet,
    25,      // damage
    500.0F,  // speed
    0.2F,    // cooldown
    3.0F,    // lifetime
    33.0F,   // hitbox width
    34.0F,   // hitbox height
    false,   // piercing
    1,       // maxHits
    1,       // projectileCount
    0.0F     // spreadAngle
};

/**
 * @brief Charged shot - slow but powerful
 */
inline constexpr WeaponConfig ChargedShot{
    ProjectileType::ChargedShot,
    100,     // damage
    600.0F,  // speed
    1.0F,    // cooldown
    4.0F,    // lifetime
    33.0F,   // hitbox width
    34.0F,   // hitbox height
    true,    // piercing
    3,       // maxHits
    1,       // projectileCount
    0.0F     // spreadAngle
};

/**
 * @brief Missile - high damage, medium speed
 */
inline constexpr WeaponConfig Missile{
    ProjectileType::Missile,
    75,      // damage
    350.0F,  // speed
    0.5F,    // cooldown
    5.0F,    // lifetime
    33.0F,   // hitbox width
    34.0F,   // hitbox height
    false,   // piercing
    1,       // maxHits
    1,       // projectileCount
    0.0F     // spreadAngle
};

/**
 * @brief Laser beam - very fast, moderate damage, piercing
 */
inline constexpr WeaponConfig LaserBeam{
    ProjectileType::LaserBeam,
    50,      // damage
    800.0F,  // speed
    0.3F,    // cooldown
    2.0F,    // lifetime
    33.0F,   // hitbox width
    34.0F,   // hitbox height
    true,    // piercing
    10,      // maxHits
    1,       // projectileCount
    0.0F     // spreadAngle
};

/**
 * @brief Spread shot - multiple projectiles in a cone
 */
inline constexpr WeaponConfig SpreadShot{
    ProjectileType::SpreadShot,
    15,      // damage per projectile
    450.0F,  // speed
    0.4F,    // cooldown
    2.5F,    // lifetime
    33.0F,   // hitbox width
    34.0F,   // hitbox height
    false,   // piercing
    1,       // maxHits
    5,       // projectileCount
    30.0F    // spreadAngle (degrees)
};

/**
 * @brief Enemy basic bullet
 */
inline constexpr WeaponConfig EnemyBullet{
    ProjectileType::EnemyBullet,
    15,      // damage
    300.0F,  // speed
    1.8F,    // cooldown (slower enemy fire)
    5.0F,    // lifetime
    33.0F,   // hitbox width
    34.0F,   // hitbox height
    false,   // piercing
    1,       // maxHits
    1,       // projectileCount
    0.0F     // spreadAngle
};

/**
 * @brief Heavy enemy bullet
 */
inline constexpr WeaponConfig HeavyBullet{
    ProjectileType::HeavyBullet,
    30,      // damage
    250.0F,  // speed
    1.2F,    // cooldown
    6.0F,    // lifetime
    33.0F,   // hitbox width
    34.0F,   // hitbox height
    false,   // piercing
    1,       // maxHits
    1,       // projectileCount
    0.0F     // spreadAngle
};

}  // namespace WeaponPresets

/**
 * @brief Maximum number of weapon slots per player
 */
inline constexpr std::size_t MAX_WEAPON_SLOTS = 5;

/**
 * @struct WeaponComponent
 * @brief Component storing weapon information for an entity
 *
 * Players can have multiple weapon slots and switch between them.
 */
struct WeaponComponent {
    std::array<WeaponConfig, MAX_WEAPON_SLOTS> weapons = {
        WeaponPresets::BasicBullet, WeaponPresets::Missile,
        WeaponPresets::LaserBeam, WeaponPresets::SpreadShot};
    uint8_t currentSlot = 0;
    uint8_t unlockedSlots = 1;

    WeaponComponent() = default;

    /**
     * @brief Get the currently selected weapon config
     * @return Reference to current weapon config
     */
    [[nodiscard]] const WeaponConfig& getCurrentWeapon() const noexcept {
        return weapons[currentSlot];
    }

    /**
     * @brief Switch to next weapon slot
     */
    void nextWeapon() noexcept {
        currentSlot = static_cast<uint8_t>((currentSlot + 1) % unlockedSlots);
    }

    /**
     * @brief Switch to previous weapon slot
     */
    void previousWeapon() noexcept {
        currentSlot = static_cast<uint8_t>((currentSlot + unlockedSlots - 1) %
                                           unlockedSlots);
    }

    /**
     * @brief Select specific weapon slot
     * @param slot Slot index to select
     */
    void selectWeapon(uint8_t slot) noexcept {
        if (slot < unlockedSlots) {
            currentSlot = slot;
        }
    }

    /**
     * @brief Unlock additional weapon slot
     */
    void unlockSlot() noexcept {
        if (unlockedSlots < MAX_WEAPON_SLOTS) {
            unlockedSlots++;
        }
    }
};

}  // namespace rtype::games::rtype::shared
