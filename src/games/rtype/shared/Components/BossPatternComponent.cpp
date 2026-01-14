/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** BossPatternComponent - Implementation of boss pattern component methods
*/

#include "BossPatternComponent.hpp"

namespace rtype::games::rtype::shared {

AttackPatternConfig AttackPatternConfig::createCircularShot(
    int32_t bulletCount, float bulletSpeed, int32_t bulletDamage) noexcept {
    AttackPatternConfig config;
    config.pattern = BossAttackPattern::CircularShot;
    config.projectileCount = bulletCount;
    config.projectileSpeed = bulletSpeed;
    config.damage = bulletDamage;
    config.duration = 0.5F;
    config.cooldown = 2.0F;
    return config;
}

AttackPatternConfig AttackPatternConfig::createSpreadFan(
    int32_t bulletCount, float angle, float bulletSpeed) noexcept {
    AttackPatternConfig config;
    config.pattern = BossAttackPattern::SpreadFan;
    config.projectileCount = bulletCount;
    config.spreadAngle = angle;
    config.projectileSpeed = bulletSpeed;
    config.damage = 20;
    config.duration = 0.3F;
    config.cooldown = 1.5F;
    config.requiresTarget = true;
    return config;
}

AttackPatternConfig AttackPatternConfig::createLaserSweep(
    float sweepDuration, float sweepAngle, int32_t laserDamage) noexcept {
    AttackPatternConfig config;
    config.pattern = BossAttackPattern::LaserSweep;
    config.duration = sweepDuration;
    config.spreadAngle = sweepAngle;
    config.damage = laserDamage;
    config.cooldown = 5.0F;
    config.telegraphDuration = 1.0F;
    config.rotationSpeed = sweepAngle / sweepDuration;
    return config;
}

AttackPatternConfig AttackPatternConfig::createMinionSpawn(
    const std::string& minionTypeId, int32_t count) noexcept {
    AttackPatternConfig config;
    config.pattern = BossAttackPattern::MinionSpawn;
    config.minionType = minionTypeId;
    config.minionCount = count;
    config.duration = 1.0F;
    config.cooldown = 8.0F;
    config.telegraphDuration = 0.8F;
    return config;
}

AttackPatternConfig AttackPatternConfig::createTailSweep(
    float sweepDuration, int32_t sweepDamage) noexcept {
    AttackPatternConfig config;
    config.pattern = BossAttackPattern::TailSweep;
    config.duration = sweepDuration;
    config.damage = sweepDamage;
    config.cooldown = 4.0F;
    config.spreadAngle = 180.0F;
    config.telegraphDuration = 0.5F;
    return config;
}

void BossPatternComponent::startNextPattern() noexcept {
    if (patternQueue.empty()) return;

    currentPattern = patternQueue.front();
    patternQueue.pop_front();

    if (cyclical) {
        patternQueue.push_back(currentPattern);
    }

    if (currentPattern.telegraphDuration > 0.0F) {
        state = PatternExecutionState::Telegraph;
        stateTimer = currentPattern.telegraphDuration;
    } else {
        state = PatternExecutionState::Executing;
        stateTimer = currentPattern.duration;
    }
    patternProgress = 0.0F;
    projectilesFired = 0;
}

void BossPatternComponent::setPhasePatterns(
    const std::vector<AttackPatternConfig>& patterns) {
    phasePatterns = patterns;
    patternQueue.clear();
    for (const auto& p : phasePatterns) {
        patternQueue.push_back(p);
    }
}

}  // namespace rtype::games::rtype::shared
