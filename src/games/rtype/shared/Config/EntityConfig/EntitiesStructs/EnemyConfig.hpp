/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** EnemyConfig - Enemy entity configuration structure
*/

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "../../../Components/AIComponent.hpp"
#include "../../../Components/BossComponent.hpp"
#include "BossAnimationConfig.hpp"

namespace rtype::games::rtype::shared {

/**
 * @struct BossPhaseConfig
 * @brief Configuration for a boss phase loaded from TOML
 */
struct BossPhaseConfig {
    float healthThreshold = 1.0F;
    std::string name;
    std::string primaryPattern;
    std::string secondaryPattern;
    float speedMultiplier = 1.0F;
    float attackSpeedMultiplier = 1.0F;
    float damageMultiplier = 1.0F;
    uint8_t colorR = 255;
    uint8_t colorG = 255;
    uint8_t colorB = 255;
};

/**
 * @struct WeakPointConfig
 * @brief Configuration for a boss weak point loaded from TOML
 */
struct WeakPointConfig {
    std::string id;
    std::string type;
    float offsetX = 0.0F;
    float offsetY = 0.0F;
    int32_t health = 100;
    float hitboxWidth = 32.0F;
    float hitboxHeight = 32.0F;
    int32_t bonusScore = 500;
    int32_t damageToParent = 0;
    bool critical = false;
    std::string disablesAttack;
    int32_t segmentIndex = -1;
    BossPartAnimationConfig animation;
};

/**
 * @struct EnemyConfig
 * @brief Configuration for an enemy type loaded from config files
 */
struct EnemyConfig {
    std::string id;
    std::string name;
    std::string spriteSheet;

    int32_t health = 100;
    int32_t damage = 10;
    int32_t scoreValue = 100;

    AIBehavior behavior = AIBehavior::MoveLeft;
    float speed = 100.0F;

    float hitboxWidth = 32.0F;
    float hitboxHeight = 32.0F;

    bool canShoot = false;
    float fireRate = 1.0F;
    std::string projectileType;

    uint8_t colorR = 255;
    uint8_t colorG = 255;
    uint8_t colorB = 255;
    uint8_t colorA = 255;

    bool isBoss = false;
    std::string bossType;
    bool levelCompleteTrigger = true;
    float phaseTransitionDuration = 1.0F;
    float invulnerabilityDuration = 1.0F;
    std::vector<BossPhaseConfig> phases;
    std::vector<WeakPointConfig> weakPoints;
    BossAnimationConfig animationConfig;

    /**
     * @brief Validate the enemy configuration
     * @return true if configuration is valid
     */
    [[nodiscard]] bool isValid() const noexcept {
        return !id.empty() && health > 0 &&
               (speed >= 0 || behavior == AIBehavior::Stationary);
    }

    /**
     * @brief Check if this enemy is a boss
     * @return true if boss configuration is present
     */
    [[nodiscard]] bool hasBossConfig() const noexcept {
        return isBoss && !phases.empty();
    }
};

}  // namespace rtype::games::rtype::shared
