/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** WeakPointComponent - Destructible boss weak point component
*/

#pragma once

#include <cstdint>
#include <string>

#include <rtype/ecs.hpp>

namespace rtype::games::rtype::shared {

/**
 * @enum WeakPointType
 * @brief Types of weak points with different behaviors
 */
enum class WeakPointType : uint8_t {
    Generic = 0,
    Head,
    Tail,
    Core,
    Arm,
    Cannon,
    Engine,
    Shield
};

/**
 * @struct WeakPointComponent
 * @brief Component for destructible boss weak points
 *
 * Weak points are child entities attached to a boss that have their own
 * health pool and hitbox. Destroying them can provide bonus score,
 * reduce boss capabilities, or expose vulnerabilities.
 *
 * Transform is relative to parent boss entity.
 *
 * Usage:
 * @code
 * WeakPointComponent wp;
 * wp.parentBossEntity = bossEntity;
 * wp.parentBossNetworkId = bossNetId;
 * wp.type = WeakPointType::Tail;
 * wp.localOffsetX = -100.0F;
 * wp.localOffsetY = 50.0F;
 * @endcode
 */
struct WeakPointComponent {
    ECS::Entity parentBossEntity{0, 0};
    uint32_t parentBossNetworkId = 0;
    WeakPointType type = WeakPointType::Generic;
    std::string weakPointId;
    float localOffsetX = 0.0F;
    float localOffsetY = 0.0F;
    float localRotation = 0.0F;

    int32_t segmentIndex = -1;
    int32_t bonusScore = 500;
    int32_t damageToParent = 0;
    float damageMultiplier = 1.0F;
    bool destroyed = false;
    bool critical = false;
    bool disablesBossAttack = false;
    std::string disabledAttackPattern;
    bool exposesCore = false;

    /**
     * @brief Check if this weak point is still attached to parent
     * @return true if parent is valid and weak point not destroyed
     */
    [[nodiscard]] bool isActive() const noexcept {
        return !destroyed && parentBossNetworkId != 0;
    }

    /**
     * @brief Mark weak point as destroyed
     */
    void destroy() noexcept { destroyed = true; }

    /**
     * @brief Get the effective damage multiplier
     * @return Damage multiplier (critical points take more damage)
     */
    [[nodiscard]] float getEffectiveDamageMultiplier() const noexcept {
        return critical ? damageMultiplier * 2.0F : damageMultiplier;
    }
};

/**
 * @struct WeakPointTag
 * @brief Tag component for quick filtering of weak point entities
 */
struct WeakPointTag {};

/**
 * @brief Convert WeakPointType to string
 * @param type The type to convert
 * @return String representation
 */
[[nodiscard]] const char* weakPointTypeToString(WeakPointType type) noexcept;

/**
 * @brief Convert string to WeakPointType
 * @param str The string to convert
 * @return Corresponding type, or Generic if not recognized
 */
[[nodiscard]] WeakPointType stringToWeakPointType(
    const std::string& str) noexcept;

}  // namespace rtype::games::rtype::shared
