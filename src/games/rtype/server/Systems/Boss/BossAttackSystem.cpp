/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** BossAttackSystem - Implementation
*/

#include "BossAttackSystem.hpp"

#include <cmath>
#include <limits>
#include <utility>

#include "../../../shared/Components/BossComponent.hpp"
#include "../../../shared/Components/NetworkIdComponent.hpp"
#include "../../../shared/Components/Tags.hpp"
#include "../../../shared/Components/TransformComponent.hpp"
#include "Logger/Macros.hpp"

namespace rtype::games::rtype::server {

using shared::BossComponent;
using shared::BossPatternComponent;
using shared::BossTag;
using shared::NetworkIdComponent;
using shared::PatternExecutionState;
using shared::PlayerTag;
using shared::TransformComponent;

namespace {
constexpr float PI = 3.14159265358979323846F;
constexpr float DEG_TO_RAD = PI / 180.0F;
}  // namespace

BossAttackSystem::BossAttackSystem(EventEmitter emitter,
                                   ProjectileSpawner projSpawner,
                                   MinionSpawner minionSpawner)
    : ASystem("BossAttackSystem"),
      _emitEvent(std::move(emitter)),
      _spawnProjectile(std::move(projSpawner)),
      _spawnMinion(std::move(minionSpawner)) {}

void BossAttackSystem::update(ECS::Registry& registry, float deltaTime) {
    auto view = registry.view<BossComponent, BossTag, BossPatternComponent,
                              TransformComponent>();

    view.each([this, &registry, deltaTime](
                  ECS::Entity entity, BossComponent& boss,
                  const BossTag& /*tag*/, BossPatternComponent& patterns,
                  const TransformComponent& transform) {
        if (boss.defeated || boss.phaseTransitionActive) {
            return;
        }

        if (!patterns.enabled) {
            return;
        }

        patterns.globalCooldown =
            std::max(0.0F, patterns.globalCooldown - deltaTime);

        updatePatternState(registry, patterns, deltaTime);

        if (patterns.canStartPattern()) {
            findNearestPlayer(registry, transform.x, transform.y,
                              patterns.targetX, patterns.targetY);
            patterns.startNextPattern();
        }

        if (!patterns.isExecuting()) {
            return;
        }

        switch (patterns.currentPattern.pattern) {
            case shared::BossAttackPattern::CircularShot:
                executeCircularShot(registry, entity, patterns, deltaTime);
                break;
            case shared::BossAttackPattern::SpreadFan:
                executeSpreadFan(registry, entity, patterns, deltaTime);
                break;
            case shared::BossAttackPattern::LaserSweep:
                executeLaserSweep(registry, entity, patterns, deltaTime);
                break;
            case shared::BossAttackPattern::MinionSpawn:
                executeMinionSpawn(registry, entity, patterns, deltaTime);
                break;
            case shared::BossAttackPattern::TailSweep:
                executeTailSweep(registry, entity, patterns, deltaTime);
                break;
            default:
                break;
        }
    });
}

void BossAttackSystem::updatePatternState(ECS::Registry& /*registry*/,
                                          BossPatternComponent& patterns,
                                          float deltaTime) {
    if (patterns.state == PatternExecutionState::Idle ||
        patterns.state == PatternExecutionState::Cooldown) {
        patterns.stateTimer -= deltaTime;
        if (patterns.stateTimer <= 0.0F) {
            patterns.resetToIdle();
        }
        return;
    }

    if (patterns.state == PatternExecutionState::Telegraph) {
        patterns.stateTimer -= deltaTime;
        if (patterns.stateTimer <= 0.0F) {
            patterns.startExecution();
        }
        return;
    }

    if (patterns.state == PatternExecutionState::Executing) {
        patterns.stateTimer -= deltaTime;
        patterns.patternProgress += deltaTime;
        if (patterns.stateTimer <= 0.0F) {
            patterns.completePattern();
        }
    }
}

void BossAttackSystem::executeCircularShot(ECS::Registry& registry,
                                           ECS::Entity boss,
                                           BossPatternComponent& patterns,
                                           float /*deltaTime*/) {
    if (patterns.state != PatternExecutionState::Executing) {
        return;
    }

    if (patterns.projectilesFired > 0) {
        return;
    }

    if (!registry.hasComponent<TransformComponent>(boss)) {
        return;
    }

    const auto& transform = registry.getComponent<TransformComponent>(boss);
    const auto& config = patterns.currentPattern;

    uint32_t ownerNetId = 0;
    if (registry.hasComponent<NetworkIdComponent>(boss)) {
        ownerNetId = registry.getComponent<NetworkIdComponent>(boss).networkId;
    }

    float angleStep = 360.0F / static_cast<float>(config.projectileCount);
    for (int32_t i = 0; i < config.projectileCount; ++i) {
        float angle = static_cast<float>(i) * angleStep * DEG_TO_RAD;
        float vx = std::cos(angle) * config.projectileSpeed;
        float vy = std::sin(angle) * config.projectileSpeed;

        if (_spawnProjectile) {
            _spawnProjectile(registry, transform.x, transform.y, vx, vy,
                             config.damage, ownerNetId);
        }
    }

    patterns.projectilesFired = config.projectileCount;
    LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                  "[BossAttackSystem] Executed CircularShot with "
                      << config.projectileCount << " projectiles");
}

void BossAttackSystem::executeSpreadFan(ECS::Registry& registry,
                                        ECS::Entity boss,
                                        BossPatternComponent& patterns,
                                        float /*deltaTime*/) {
    if (patterns.state != PatternExecutionState::Executing) {
        return;
    }

    if (patterns.projectilesFired > 0) {
        return;
    }

    if (!registry.hasComponent<TransformComponent>(boss)) {
        return;
    }

    const auto& transform = registry.getComponent<TransformComponent>(boss);
    const auto& config = patterns.currentPattern;

    uint32_t ownerNetId = 0;
    if (registry.hasComponent<NetworkIdComponent>(boss)) {
        ownerNetId = registry.getComponent<NetworkIdComponent>(boss).networkId;
    }

    float dx = patterns.targetX - transform.x;
    float dy = patterns.targetY - transform.y;
    float baseAngle = std::atan2(dy, dx);

    float halfSpread = config.spreadAngle * 0.5F * DEG_TO_RAD;
    float angleStep = (config.projectileCount > 1)
                          ? (config.spreadAngle * DEG_TO_RAD) /
                                static_cast<float>(config.projectileCount - 1)
                          : 0.0F;

    for (int32_t i = 0; i < config.projectileCount; ++i) {
        float angle =
            baseAngle - halfSpread + static_cast<float>(i) * angleStep;
        float vx = std::cos(angle) * config.projectileSpeed;
        float vy = std::sin(angle) * config.projectileSpeed;

        if (_spawnProjectile) {
            _spawnProjectile(registry, transform.x, transform.y, vx, vy,
                             config.damage, ownerNetId);
        }
    }

    patterns.projectilesFired = config.projectileCount;
    LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                  "[BossAttackSystem] Executed SpreadFan with "
                      << config.projectileCount << " projectiles");
}

void BossAttackSystem::executeLaserSweep(ECS::Registry& registry,
                                         ECS::Entity boss,
                                         BossPatternComponent& patterns,
                                         float deltaTime) {
    if (patterns.state != PatternExecutionState::Executing) {
        return;
    }

    if (!registry.hasComponent<TransformComponent>(boss)) {
        return;
    }

    const auto& transform = registry.getComponent<TransformComponent>(boss);
    const auto& config = patterns.currentPattern;

    uint32_t ownerNetId = 0;
    if (registry.hasComponent<NetworkIdComponent>(boss)) {
        ownerNetId = registry.getComponent<NetworkIdComponent>(boss).networkId;
    }

    float sweepProgress = patterns.patternProgress / config.duration;
    float baseAngle = PI;
    float currentAngle = baseAngle + (-config.spreadAngle * 0.5F +
                                      config.spreadAngle * sweepProgress) *
                                         DEG_TO_RAD;

    patterns.telegraphAngle = currentAngle;

    static float lastFireTime = 0.0F;
    lastFireTime += deltaTime;
    constexpr float fireInterval = 0.1F;

    if (lastFireTime >= fireInterval) {
        lastFireTime = 0.0F;

        float vx = std::cos(currentAngle) * config.projectileSpeed;
        float vy = std::sin(currentAngle) * config.projectileSpeed;

        if (_spawnProjectile) {
            _spawnProjectile(registry, transform.x, transform.y, vx, vy,
                             config.damage, ownerNetId);
        }
        patterns.projectilesFired++;
    }
}

void BossAttackSystem::executeMinionSpawn(ECS::Registry& registry,
                                          ECS::Entity boss,
                                          BossPatternComponent& patterns,
                                          float /*deltaTime*/) {
    if (patterns.state != PatternExecutionState::Executing) {
        return;
    }

    if (patterns.projectilesFired > 0) {
        return;
    }

    if (!registry.hasComponent<TransformComponent>(boss)) {
        return;
    }

    const auto& transform = registry.getComponent<TransformComponent>(boss);
    const auto& config = patterns.currentPattern;

    for (int32_t i = 0; i < config.minionCount; ++i) {
        float offsetY = static_cast<float>(i - config.minionCount / 2) * 50.0F;

        if (_spawnMinion) {
            _spawnMinion(registry, config.minionType, transform.x - 50.0F,
                         transform.y + offsetY);
        }
    }

    patterns.projectilesFired = config.minionCount;
    LOG_INFO("[BossAttackSystem] Spawned "
             << config.minionCount << " minions of type " << config.minionType);
}

void BossAttackSystem::executeTailSweep(ECS::Registry& registry,
                                        ECS::Entity boss,
                                        BossPatternComponent& patterns,
                                        float /*deltaTime*/) {
    if (patterns.state != PatternExecutionState::Executing) {
        return;
    }

    if (!registry.hasComponent<TransformComponent>(boss)) {
        return;
    }

    const auto& config = patterns.currentPattern;

    float sweepProgress = patterns.patternProgress / config.duration;

    patterns.telegraphAngle =
        (-config.spreadAngle * 0.5F + config.spreadAngle * sweepProgress) *
        DEG_TO_RAD;

    if (_emitEvent) {
        uint32_t ownerNetId = 0;
        if (registry.hasComponent<NetworkIdComponent>(boss)) {
            ownerNetId =
                registry.getComponent<NetworkIdComponent>(boss).networkId;
        }

        engine::GameEvent event{};
        event.type = engine::GameEventType::BossAttack;
        event.entityNetworkId = ownerNetId;
        event.attackAngle = patterns.telegraphAngle;
        event.attackProgress = sweepProgress;
        _emitEvent(event);
    }
}

void BossAttackSystem::findNearestPlayer(ECS::Registry& registry, float bossX,
                                         float bossY, float& targetX,
                                         float& targetY) {
    float bestDist2 = std::numeric_limits<float>::max();
    targetX = bossX - 300.0F;
    targetY = bossY;

    auto playerView = registry.view<PlayerTag, TransformComponent>();
    playerView.each([bossX, bossY, &targetX, &targetY, &bestDist2](
                        ECS::Entity /*entity*/, const PlayerTag& /*tag*/,
                        const TransformComponent& transform) {
        float dx = transform.x - bossX;
        float dy = transform.y - bossY;
        float dist2 = dx * dx + dy * dy;

        if (dist2 < bestDist2) {
            bestDist2 = dist2;
            targetX = transform.x;
            targetY = transform.y;
        }
    });
}

}  // namespace rtype::games::rtype::server
