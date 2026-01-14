/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** BossComponent - Multi-phase boss component with behavior configuration
*/

#pragma once

#include <cstdint>
#include <deque>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace rtype::games::rtype::shared {

/**
 * @enum BossAttackPattern
 * @brief Available attack patterns for boss phases
 */
enum class BossAttackPattern : uint8_t {
    None = 0,
    CircularShot,
    SpreadFan,
    LaserSweep,
    MinionSpawn,
    TailSweep,
    ChargeAttack,
    HomingMissile,
    GroundPound
};

/**
 * @struct BossPhase
 * @brief Defines a single phase of a boss fight
 *
 * Each phase activates when boss health drops at or below the threshold.
 * Phases should be ordered from highest to lowest threshold.
 */
struct BossPhase {
    float healthThreshold = 1.0F;
    BossAttackPattern primaryPattern = BossAttackPattern::None;
    BossAttackPattern secondaryPattern = BossAttackPattern::None;
    float speedMultiplier = 1.0F;
    float attackSpeedMultiplier = 1.0F;
    float damageMultiplier = 1.0F;
    std::string phaseName;
    uint8_t colorR = 255;
    uint8_t colorG = 255;
    uint8_t colorB = 255;

    /**
     * @brief Check if this phase should activate at given health ratio
     * @param healthRatio Current health / max health (0.0 - 1.0)
     * @return true if health is at or below threshold
     */
    [[nodiscard]] bool shouldActivate(float healthRatio) const noexcept {
        return healthRatio <= healthThreshold;
    }
};

/**
 * @enum BossType
 * @brief Enumeration of boss types
 */
enum class BossType : uint8_t {
    Generic = 0,
    Serpent,
    Scorpion,
    Battleship,
    Hive
};

/**
 * @struct BossComponent
 * @brief Component for boss entities with multi-phase behavior
 *
 * Manages boss phases, attack patterns, and state transitions.
 * Works with BossPhaseSystem to handle phase changes based on health.
 *
 * Usage:
 * @code
 * BossComponent boss;
 * boss.bossType = BossType::Serpent;
 * boss.phases.push_back({0.75F, BossAttackPattern::SpreadFan, ...});
 * boss.phases.push_back({0.50F, BossAttackPattern::CircularShot, ...});
 * boss.phases.push_back({0.25F, BossAttackPattern::LaserSweep, ...});
 * @endcode
 */
struct BossComponent {
    BossType bossType = BossType::Generic;
    std::string bossId;
    std::vector<BossPhase> phases;
    std::size_t currentPhaseIndex = 0;
    bool phaseTransitionActive = false;
    float phaseTransitionTimer = 0.0F;
    float phaseTransitionDuration = 1.0F;
    float invulnerabilityTimer = 0.0F;
    int32_t scoreValue = 5000;
    bool defeated = false;
    bool levelCompleteTrigger = true;

    float movementTimer = 0.0F;
    float baseX = 0.0F;
    float baseY = 0.0F;
    float amplitude = 150.0F;
    float frequency = 0.5F;

    static constexpr std::size_t MAX_POSITION_HISTORY = 500;
    static constexpr float SEGMENT_SPACING = 100.0F;
    static constexpr float MIN_RECORD_DISTANCE = 3.0F;
    std::deque<std::pair<float, float>> positionHistory;

    /**
     * @brief Record current position in history for serpent movement
     * Only records if moved at least MIN_RECORD_DISTANCE from last position
     * @param x Current X position
     * @param y Current Y position
     */
    void recordPosition(float x, float y) {
        if (!positionHistory.empty()) {
            float dx = x - positionHistory.front().first;
            float dy = y - positionHistory.front().second;
            float distSq = dx * dx + dy * dy;
            if (distSq < MIN_RECORD_DISTANCE * MIN_RECORD_DISTANCE) {
                return;
            }
        }
        positionHistory.push_front({x, y});
        if (positionHistory.size() > MAX_POSITION_HISTORY) {
            positionHistory.pop_back();
        }
    }

    /**
     * @brief Get position for a segment at given index (0 = head)
     * @param segmentIndex Index of the segment (0 = closest to head)
     * @return Position pair, or head position if history too short
     */
    [[nodiscard]] std::pair<float, float> getSegmentPosition(
        std::size_t segmentIndex) const noexcept {
        if (segmentIndex == 0) {
            if (!positionHistory.empty()) {
                return positionHistory.front();
            }
            return {baseX, baseY};
        }
        std::size_t historyIndex = segmentIndex * 15;
        if (historyIndex < positionHistory.size()) {
            return positionHistory[historyIndex];
        }
        if (!positionHistory.empty()) {
            auto lastPos = positionHistory.back();
            float extraOffset =
                static_cast<float>(segmentIndex) * SEGMENT_SPACING * 0.5F;
            return {lastPos.first - extraOffset, lastPos.second};
        }
        return {baseX - static_cast<float>(segmentIndex) * SEGMENT_SPACING,
                baseY};
    }

    /**
     * @brief Get the current phase configuration
     * @return Pointer to current phase, nullptr if no phases defined
     */
    [[nodiscard]] const BossPhase* getCurrentPhase() const noexcept {
        if (phases.empty() || currentPhaseIndex >= phases.size()) {
            return nullptr;
        }
        return &phases[currentPhaseIndex];
    }

    /**
     * @brief Get the current phase (mutable)
     * @return Pointer to current phase, nullptr if no phases defined
     */
    [[nodiscard]] BossPhase* getCurrentPhase() noexcept {
        if (phases.empty() || currentPhaseIndex >= phases.size()) {
            return nullptr;
        }
        return &phases[currentPhaseIndex];
    }

    /**
     * @brief Check if boss should transition to a new phase
     * @param healthRatio Current health / max health (0.0 - 1.0)
     * @return Index of the phase to transition to, or nullopt if no change
     */
    [[nodiscard]] std::optional<std::size_t> checkPhaseTransition(
        float healthRatio) const noexcept {
        for (std::size_t i = currentPhaseIndex + 1; i < phases.size(); ++i) {
            if (phases[i].shouldActivate(healthRatio)) {
                return i;
            }
        }
        return std::nullopt;
    }

    /**
     * @brief Transition to a new phase
     * @param newPhaseIndex Index of the phase to transition to
     */
    void transitionToPhase(std::size_t newPhaseIndex) noexcept {
        if (newPhaseIndex < phases.size()) {
            currentPhaseIndex = newPhaseIndex;
            phaseTransitionActive = true;
            phaseTransitionTimer = 0.0F;
        }
    }

    /**
     * @brief Check if boss is in any phase
     * @return true if phases are defined and valid
     */
    [[nodiscard]] bool hasPhases() const noexcept { return !phases.empty(); }

    /**
     * @brief Get total number of phases
     * @return Phase count
     */
    [[nodiscard]] std::size_t getPhaseCount() const noexcept {
        return phases.size();
    }

    /**
     * @brief Check if boss is currently invulnerable
     * @return true if invulnerability timer is active
     */
    [[nodiscard]] bool isInvulnerable() const noexcept {
        return invulnerabilityTimer > 0.0F || phaseTransitionActive;
    }
};

/**
 * @struct BossTag
 * @brief Tag component to identify boss entities for quick filtering
 */
struct BossTag {};

/**
 * @brief Convert BossAttackPattern enum to string
 * @param pattern The pattern to convert
 * @return String representation
 */
[[nodiscard]] inline const char* bossAttackPatternToString(
    BossAttackPattern pattern) noexcept {
    switch (pattern) {
        case BossAttackPattern::None:
            return "None";
        case BossAttackPattern::CircularShot:
            return "CircularShot";
        case BossAttackPattern::SpreadFan:
            return "SpreadFan";
        case BossAttackPattern::LaserSweep:
            return "LaserSweep";
        case BossAttackPattern::MinionSpawn:
            return "MinionSpawn";
        case BossAttackPattern::TailSweep:
            return "TailSweep";
        case BossAttackPattern::ChargeAttack:
            return "ChargeAttack";
        case BossAttackPattern::HomingMissile:
            return "HomingMissile";
        case BossAttackPattern::GroundPound:
            return "GroundPound";
        default:
            return "Unknown";
    }
}

/**
 * @brief Convert BossType enum to string
 * @param type The boss type to convert
 * @return String representation
 */
[[nodiscard]] inline const char* bossTypeToString(BossType type) noexcept {
    switch (type) {
        case BossType::Generic:
            return "Generic";
        case BossType::Serpent:
            return "Serpent";
        case BossType::Scorpion:
            return "Scorpion";
        case BossType::Battleship:
            return "Battleship";
        case BossType::Hive:
            return "Hive";
        default:
            return "Unknown";
    }
}

/**
 * @brief Convert string to BossAttackPattern enum
 * @param str The string to convert
 * @return Corresponding pattern, or None if not recognized
 */
[[nodiscard]] inline BossAttackPattern stringToBossAttackPattern(
    const std::string& str) noexcept {
    if (str == "circular_shot") return BossAttackPattern::CircularShot;
    if (str == "spread_fan") return BossAttackPattern::SpreadFan;
    if (str == "laser_sweep") return BossAttackPattern::LaserSweep;
    if (str == "minion_spawn") return BossAttackPattern::MinionSpawn;
    if (str == "tail_sweep") return BossAttackPattern::TailSweep;
    if (str == "charge_attack") return BossAttackPattern::ChargeAttack;
    if (str == "homing_missile") return BossAttackPattern::HomingMissile;
    if (str == "ground_pound") return BossAttackPattern::GroundPound;
    return BossAttackPattern::None;
}

/**
 * @brief Convert string to BossType enum
 * @param str The string to convert
 * @return Corresponding type, or Generic if not recognized
 */
[[nodiscard]] inline BossType stringToBossType(
    const std::string& str) noexcept {
    if (str == "serpent") return BossType::Serpent;
    if (str == "scorpion") return BossType::Scorpion;
    if (str == "battleship") return BossType::Battleship;
    if (str == "hive") return BossType::Hive;
    return BossType::Generic;
}

}  // namespace rtype::games::rtype::shared
