/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** BossPhaseSystem - Implementation
*/

#include "BossPhaseSystem.hpp"

#include <cmath>
#include <utility>

#include <rtype/network/Protocol.hpp>

#include "../../../shared/Components/BossComponent.hpp"
#include "../../../shared/Components/BossPatternComponent.hpp"
#include "../../../shared/Components/HealthComponent.hpp"
#include "../../../shared/Components/NetworkIdComponent.hpp"
#include "../../../shared/Components/Tags.hpp"
#include "../../../shared/Components/TransformComponent.hpp"
#include "../../../shared/Components/VelocityComponent.hpp"
#include "Logger/Macros.hpp"

namespace rtype::games::rtype::server {

using shared::BossComponent;
using shared::BossPatternComponent;
using shared::BossPhase;
using shared::BossTag;
using shared::DestroyTag;
using shared::HealthComponent;
using shared::NetworkIdComponent;
using shared::TransformComponent;
using shared::VelocityComponent;

namespace {
constexpr float PI = 3.14159265358979323846F;
constexpr float TWO_PI = PI * 2.0F;
}  // namespace

BossPhaseSystem::BossPhaseSystem(EventEmitter emitter)
    : ASystem("BossPhaseSystem"), _emitEvent(std::move(emitter)) {}

void BossPhaseSystem::update(ECS::Registry& registry, float deltaTime) {
    updatePhaseTransitions(registry, deltaTime);
    updateBossMovement(registry, deltaTime);

    auto view = registry.view<BossComponent, BossTag, HealthComponent>();

    view.each([this, &registry, deltaTime](
                  ECS::Entity entity, BossComponent& boss,
                  const BossTag& /*tag*/, HealthComponent& health) {
        if (boss.defeated) {
            return;
        }

        if (boss.invulnerabilityTimer > 0.0F) {
            boss.invulnerabilityTimer -= deltaTime;
        }

        if (!health.isAlive()) {
            checkBossDefeated(registry, entity);
            return;
        }

        if (boss.phaseTransitionActive) {
            return;
        }

        float healthRatio =
            static_cast<float>(health.current) / static_cast<float>(health.max);

        auto newPhaseOpt = boss.checkPhaseTransition(healthRatio);
        if (newPhaseOpt.has_value()) {
            handlePhaseTransition(registry, entity, *newPhaseOpt);
        }
    });
}

void BossPhaseSystem::handlePhaseTransition(ECS::Registry& registry,
                                            ECS::Entity entity,
                                            std::size_t newPhaseIndex) {
    auto& boss = registry.getComponent<BossComponent>(entity);

    std::size_t oldPhase = boss.currentPhaseIndex;
    boss.transitionToPhase(newPhaseIndex);
    boss.invulnerabilityTimer = boss.phaseTransitionDuration;

    const auto* phase = boss.getCurrentPhase();
    if (phase == nullptr) {
        return;
    }

    LOG_INFO("[BossPhaseSystem] Boss transitioning from phase "
             << oldPhase << " to phase " << newPhaseIndex << " ("
             << phase->phaseName << ")");

    if (registry.hasComponent<BossPatternComponent>(entity)) {
        auto& patterns = registry.getComponent<BossPatternComponent>(entity);
        patterns.clear();

        if (phase->primaryPattern != shared::BossAttackPattern::None) {
            shared::AttackPatternConfig primaryConfig;
            primaryConfig.pattern = phase->primaryPattern;
            primaryConfig.cooldown *= (1.0F / phase->attackSpeedMultiplier);
            primaryConfig.damage =
                static_cast<int32_t>(static_cast<float>(primaryConfig.damage) *
                                     phase->damageMultiplier);
            patterns.patternQueue.push_back(primaryConfig);
        }

        if (phase->secondaryPattern != shared::BossAttackPattern::None) {
            shared::AttackPatternConfig secondaryConfig;
            secondaryConfig.pattern = phase->secondaryPattern;
            secondaryConfig.cooldown *= (1.0F / phase->attackSpeedMultiplier);
            patterns.patternQueue.push_back(secondaryConfig);
        }

        patterns.cyclical = true;
    }

    if (_emitEvent && registry.hasComponent<NetworkIdComponent>(entity)) {
        const auto& netId = registry.getComponent<NetworkIdComponent>(entity);
        if (netId.isValid()) {
            engine::GameEvent event{};
            event.type = engine::GameEventType::BossPhaseChanged;
            event.entityNetworkId = netId.networkId;
            event.bossPhase = static_cast<uint8_t>(newPhaseIndex);
            event.bossPhaseCount = static_cast<uint8_t>(boss.getPhaseCount());
            _emitEvent(event);

            LOG_INFO(
                "[BossPhaseSystem] Emitted BossPhaseChanged event for "
                "boss networkId="
                << netId.networkId << " phase=" << newPhaseIndex);
        }
    }
}

void BossPhaseSystem::updatePhaseTransitions(ECS::Registry& registry,
                                             float deltaTime) {
    auto view = registry.view<BossComponent, BossTag>();

    view.each([deltaTime](ECS::Entity /*entity*/, BossComponent& boss,
                          const BossTag& /*tag*/) {
        if (!boss.phaseTransitionActive) {
            return;
        }

        boss.phaseTransitionTimer += deltaTime;
        if (boss.phaseTransitionTimer >= boss.phaseTransitionDuration) {
            boss.phaseTransitionActive = false;
            boss.phaseTransitionTimer = 0.0F;
        }
    });
}

void BossPhaseSystem::updateBossMovement(ECS::Registry& registry,
                                         float deltaTime) {
    auto view = registry.view<BossComponent, BossTag, TransformComponent,
                              VelocityComponent>();

    view.each([deltaTime](ECS::Entity /*entity*/, BossComponent& boss,
                          const BossTag& /*tag*/, TransformComponent& transform,
                          VelocityComponent& velocity) {
        if (boss.defeated) {
            velocity.vx = 0.0F;
            velocity.vy = 0.0F;
            return;
        }

        if (boss.baseY == 0.0F) {
            boss.baseY = transform.y;
            boss.baseX = transform.x;
        }

        const BossPhase* phase = boss.getCurrentPhase();
        float speedMult = (phase != nullptr) ? phase->speedMultiplier : 1.0F;

        boss.movementTimer += deltaTime;

        float horizontalAmplitude = 250.0F;
        float verticalAmplitude = boss.amplitude * 1.5F;
        float freqX = boss.frequency * 0.6F;
        float freqY = boss.frequency * 1.2F;

        float targetX = boss.baseX + horizontalAmplitude *
                                         std::sin(freqX * boss.movementTimer);
        float targetY = boss.baseY + verticalAmplitude *
                                         std::sin(freqY * boss.movementTimer);

        float minX = 1280.0F * 0.5F;
        if (targetX < minX) {
            targetX = minX;
        }

        velocity.vx = (targetX - transform.x) * 3.0F * speedMult;
        velocity.vy = (targetY - transform.y) * 3.0F * speedMult;
    });
}

void BossPhaseSystem::checkBossDefeated(ECS::Registry& registry,
                                        ECS::Entity entity) {
    auto& boss = registry.getComponent<BossComponent>(entity);

    if (boss.defeated) {
        return;
    }

    boss.defeated = true;

    LOG_INFO("[BossPhaseSystem] Boss defeated! Type="
             << shared::bossTypeToString(boss.bossType)
             << " Score=" << boss.scoreValue);

    if (_emitEvent && registry.hasComponent<NetworkIdComponent>(entity)) {
        const auto& netId = registry.getComponent<NetworkIdComponent>(entity);
        if (netId.isValid()) {
            engine::GameEvent scoreEvent{};
            scoreEvent.type = engine::GameEventType::ScoreChanged;
            scoreEvent.entityNetworkId = netId.networkId;
            scoreEvent.score = boss.scoreValue;
            _emitEvent(scoreEvent);

            engine::GameEvent defeatEvent{};
            defeatEvent.type = engine::GameEventType::BossDefeated;
            defeatEvent.entityNetworkId = netId.networkId;
            _emitEvent(defeatEvent);

            if (boss.levelCompleteTrigger) {
                engine::GameEvent levelEvent{};
                levelEvent.type = engine::GameEventType::LevelComplete;
                _emitEvent(levelEvent);

                LOG_INFO("[BossPhaseSystem] Level complete triggered!");
            }
        }
    }

    if (!registry.hasComponent<DestroyTag>(entity)) {
        registry.emplaceComponent<DestroyTag>(entity);
    }
}

}  // namespace rtype::games::rtype::server
