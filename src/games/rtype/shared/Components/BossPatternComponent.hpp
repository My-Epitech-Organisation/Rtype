/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** BossPatternComponent - Boss attack pattern management component
*/

#pragma once

#include <cstdint>
#include <deque>
#include <string>
#include <vector>

#include "BossComponent.hpp"

namespace rtype::games::rtype::shared {

/**
 * @struct AttackPatternConfig
 * @brief Configuration for a single attack pattern execution
 */
struct AttackPatternConfig {
    BossAttackPattern pattern = BossAttackPattern::None;
    float duration = 2.0F;
    float cooldown = 1.0F;
    float projectileSpeed = 400.0F;
    int32_t projectileCount = 8;
    int32_t damage = 25;
    float spreadAngle = 45.0F;
    float rotationSpeed = 90.0F;
    std::string minionType;
    int32_t minionCount = 3;
    float telegraphDuration = 0.5F;
    bool requiresTarget = false;

    /**
     * @brief Create a CircularShot pattern configuration
     * @param bulletCount Number of bullets in the circle
     * @param bulletSpeed Speed of bullets
     * @param bulletDamage Damage per bullet
     * @return Configured pattern
     */
    [[nodiscard]] static AttackPatternConfig createCircularShot(
        int32_t bulletCount = 12, float bulletSpeed = 350.0F,
        int32_t bulletDamage = 15) noexcept;

    /**
     * @brief Create a SpreadFan pattern configuration
     * @param bulletCount Number of bullets in the fan
     * @param angle Total spread angle in degrees
     * @param bulletSpeed Speed of bullets
     * @return Configured pattern
     */
    [[nodiscard]] static AttackPatternConfig createSpreadFan(
        int32_t bulletCount = 5, float angle = 60.0F,
        float bulletSpeed = 400.0F) noexcept;

    /**
     * @brief Create a LaserSweep pattern configuration
     * @param sweepDuration Duration of the sweep
     * @param sweepAngle Total sweep angle
     * @param laserDamage Damage per tick
     * @return Configured pattern
     */
    [[nodiscard]] static AttackPatternConfig createLaserSweep(
        float sweepDuration = 3.0F, float sweepAngle = 120.0F,
        int32_t laserDamage = 30) noexcept;

    /**
     * @brief Create a MinionSpawn pattern configuration
     * @param minionTypeId ID of the minion enemy to spawn
     * @param count Number of minions to spawn
     * @return Configured pattern
     */
    [[nodiscard]] static AttackPatternConfig createMinionSpawn(
        const std::string& minionTypeId = "basic", int32_t count = 4) noexcept;

    /**
     * @brief Create a TailSweep pattern configuration
     * @param sweepDuration Duration of the sweep
     * @param sweepDamage Contact damage
     * @return Configured pattern
     */
    [[nodiscard]] static AttackPatternConfig createTailSweep(
        float sweepDuration = 2.0F, int32_t sweepDamage = 40) noexcept;
};

/**
 * @enum PatternExecutionState
 * @brief Current state of a pattern being executed
 */
enum class PatternExecutionState : uint8_t {
    Idle = 0,
    Telegraph,
    Executing,
    Cooldown
};

/**
 * @struct BossPatternComponent
 * @brief Component managing boss attack pattern execution
 *
 * Handles pattern queuing, execution timing, and cooldown management.
 * Works with BossAttackSystem to execute patterns.
 *
 * Usage:
 * @code
 * BossPatternComponent patterns;
 * patterns.patternQueue.push_back(AttackPatternConfig::createCircularShot());
 * patterns.patternQueue.push_back(AttackPatternConfig::createSpreadFan());
 * patterns.cyclical = true;
 * @endcode
 */
struct BossPatternComponent {
    std::vector<AttackPatternConfig> phasePatterns;
    std::deque<AttackPatternConfig> patternQueue;
    AttackPatternConfig currentPattern;
    PatternExecutionState state = PatternExecutionState::Idle;
    float stateTimer = 0.0F;
    float globalCooldown = 0.0F;
    float patternProgress = 0.0F;
    float targetX = 0.0F;
    float targetY = 0.0F;
    bool cyclical = true;
    bool enabled = true;
    float telegraphAngle = 0.0F;
    int32_t projectilesFired = 0;
    float lastFireTime =
        0.0F;  ///< Entity-specific fire timer for continuous attacks

    /**
     * @brief Check if a pattern is currently being executed
     * @return true if in Telegraph or Executing state
     */
    [[nodiscard]] bool isExecuting() const noexcept {
        return state == PatternExecutionState::Telegraph ||
               state == PatternExecutionState::Executing;
    }

    /**
     * @brief Check if ready to start a new pattern
     * @return true if idle and has patterns queued
     */
    [[nodiscard]] bool canStartPattern() const noexcept {
        return enabled && state == PatternExecutionState::Idle &&
               globalCooldown <= 0.0F && !patternQueue.empty();
    }

    /**
     * @brief Start the next pattern in the queue
     */
    void startNextPattern() noexcept;

    /**
     * @brief Advance to execution state after telegraph
     */
    void startExecution() noexcept {
        state = PatternExecutionState::Executing;
        stateTimer = currentPattern.duration;
    }

    /**
     * @brief Complete current pattern and enter cooldown
     */
    void completePattern() noexcept {
        state = PatternExecutionState::Cooldown;
        stateTimer = currentPattern.cooldown;
        globalCooldown = currentPattern.cooldown * 0.5F;
    }

    /**
     * @brief Reset to idle state after cooldown
     */
    void resetToIdle() noexcept {
        state = PatternExecutionState::Idle;
        stateTimer = 0.0F;
        patternProgress = 0.0F;
        projectilesFired = 0;
    }

    /**
     * @brief Add patterns for a specific phase
     * @param patterns Vector of pattern configs to add
     */
    void setPhasePatterns(const std::vector<AttackPatternConfig>& patterns);

    /**
     * @brief Clear all patterns and reset state
     */
    void clear() noexcept {
        phasePatterns.clear();
        patternQueue.clear();
        state = PatternExecutionState::Idle;
        stateTimer = 0.0F;
        globalCooldown = 0.0F;
        patternProgress = 0.0F;
    }
};

}  // namespace rtype::games::rtype::shared
