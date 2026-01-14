/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** WeakPointSystem - Implementation
*/

#include "WeakPointSystem.hpp"

#include <cmath>
#include <utility>

#include "../../../shared/Components/BossComponent.hpp"
#include "../../../shared/Components/BossPatternComponent.hpp"
#include "../../../shared/Components/HealthComponent.hpp"
#include "../../../shared/Components/NetworkIdComponent.hpp"
#include "../../../shared/Components/Tags.hpp"
#include "../../../shared/Components/TransformComponent.hpp"
#include "../../../shared/Components/WeakPointComponent.hpp"
#include "Logger/Macros.hpp"

namespace rtype::games::rtype::server {

using shared::BossComponent;
using shared::BossPatternComponent;
using shared::BossTag;
using shared::DestroyTag;
using shared::HealthComponent;
using shared::NetworkIdComponent;
using shared::TransformComponent;
using shared::WeakPointComponent;
using shared::WeakPointTag;

WeakPointSystem::WeakPointSystem(EventEmitter emitter)
    : ASystem("WeakPointSystem"), _emitEvent(std::move(emitter)) {}

void WeakPointSystem::update(ECS::Registry& registry, float /*deltaTime*/) {
    syncWeakPointPositions(registry);
    handleWeakPointDestruction(registry);
}

void WeakPointSystem::syncWeakPointPositions(ECS::Registry& registry) {
    auto view =
        registry.view<WeakPointComponent, WeakPointTag, TransformComponent>();

    view.each([&registry](ECS::Entity /*entity*/, WeakPointComponent& weakPoint,
                          const WeakPointTag& /*tag*/,
                          TransformComponent& transform) {
        if (weakPoint.destroyed) {
            return;
        }

        ECS::Entity parent = weakPoint.parentBossEntity;
        if (!registry.isAlive(parent)) {
            return;
        }

        if (!registry.hasComponent<TransformComponent>(parent)) {
            return;
        }

        const auto& parentTransform =
            registry.getComponent<TransformComponent>(parent);

        if (weakPoint.segmentIndex > 0 &&
            registry.hasComponent<BossComponent>(parent)) {
            const auto& boss = registry.getComponent<BossComponent>(parent);

            auto [histX, histY] = boss.getSegmentPosition(
                static_cast<std::size_t>(weakPoint.segmentIndex));

            transform.x = histX;
            transform.y = histY;
        } else {
            transform.x = parentTransform.x + weakPoint.localOffsetX;
            transform.y = parentTransform.y + weakPoint.localOffsetY;
            transform.rotation =
                parentTransform.rotation + weakPoint.localRotation;
        }
    });
}

void WeakPointSystem::handleWeakPointDestruction(ECS::Registry& registry) {
    auto view =
        registry.view<WeakPointComponent, WeakPointTag, HealthComponent>();

    view.each([this, &registry](
                  ECS::Entity entity, WeakPointComponent& weakPoint,
                  const WeakPointTag& /*tag*/, HealthComponent& health) {
        if (weakPoint.destroyed) {
            return;
        }

        if (health.isAlive()) {
            return;
        }

        weakPoint.destroy();

        LOG_INFO("[WeakPointSystem] Weak point destroyed: "
                 << weakPoint.weakPointId
                 << " Type=" << shared::weakPointTypeToString(weakPoint.type));

        if (_emitEvent && registry.hasComponent<NetworkIdComponent>(entity)) {
            const auto& netId =
                registry.getComponent<NetworkIdComponent>(entity);
            if (netId.isValid()) {
                engine::GameEvent scoreEvent{};
                scoreEvent.type = engine::GameEventType::ScoreChanged;
                scoreEvent.entityNetworkId = netId.networkId;
                scoreEvent.score = weakPoint.bonusScore;
                _emitEvent(scoreEvent);

                engine::GameEvent destroyEvent{};
                destroyEvent.type = engine::GameEventType::WeakPointDestroyed;
                destroyEvent.entityNetworkId = netId.networkId;
                destroyEvent.parentNetworkId = weakPoint.parentBossNetworkId;
                _emitEvent(destroyEvent);

                LOG_INFO(
                    "[WeakPointSystem] Emitted WeakPointDestroyed event "
                    "and bonus score "
                    << weakPoint.bonusScore);
            }
        }

        if (weakPoint.damageToParent > 0) {
            applyParentDamage(registry, entity);
        }

        if (weakPoint.disablesBossAttack) {
            disableBossPattern(registry, entity);
        }

        if (!registry.hasComponent<DestroyTag>(entity)) {
            registry.emplaceComponent<DestroyTag>(entity);
        }
    });
}

void WeakPointSystem::applyParentDamage(ECS::Registry& registry,
                                        ECS::Entity weakPoint) {
    if (!registry.hasComponent<WeakPointComponent>(weakPoint)) {
        return;
    }

    const auto& wp = registry.getComponent<WeakPointComponent>(weakPoint);
    ECS::Entity parent = wp.parentBossEntity;

    if (!registry.isAlive(parent)) {
        return;
    }

    if (!registry.hasComponent<HealthComponent>(parent)) {
        return;
    }

    auto& parentHealth = registry.getComponent<HealthComponent>(parent);
    parentHealth.takeDamage(wp.damageToParent);

    LOG_INFO("[WeakPointSystem] Applied " << wp.damageToParent
                                          << " damage to parent boss");

    if (_emitEvent && registry.hasComponent<NetworkIdComponent>(parent)) {
        const auto& netId = registry.getComponent<NetworkIdComponent>(parent);
        if (netId.isValid()) {
            engine::GameEvent event{};
            event.type = engine::GameEventType::EntityHealthChanged;
            event.entityNetworkId = netId.networkId;
            event.healthCurrent = parentHealth.current;
            event.healthMax = parentHealth.max;
            event.damage = wp.damageToParent;
            _emitEvent(event);
        }
    }
}

void WeakPointSystem::disableBossPattern(ECS::Registry& registry,
                                         ECS::Entity weakPoint) {
    if (!registry.hasComponent<WeakPointComponent>(weakPoint)) {
        return;
    }

    const auto& wp = registry.getComponent<WeakPointComponent>(weakPoint);
    ECS::Entity parent = wp.parentBossEntity;

    if (!registry.isAlive(parent)) {
        return;
    }

    if (!registry.hasComponent<BossPatternComponent>(parent)) {
        return;
    }

    auto& patterns = registry.getComponent<BossPatternComponent>(parent);

    if (!wp.disabledAttackPattern.empty()) {
        shared::BossAttackPattern patternToDisable =
            shared::stringToBossAttackPattern(wp.disabledAttackPattern);

        auto& queue = patterns.patternQueue;
        queue.erase(std::remove_if(queue.begin(), queue.end(),
                                   [patternToDisable](
                                       const shared::AttackPatternConfig& cfg) {
                                       return cfg.pattern == patternToDisable;
                                   }),
                    queue.end());

        LOG_INFO("[WeakPointSystem] Disabled boss attack pattern: "
                 << wp.disabledAttackPattern);
    }
}

}  // namespace rtype::games::rtype::server
