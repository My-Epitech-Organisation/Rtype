/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** EnemyHealthBarSystem.cpp
*/

#include "EnemyHealthBarSystem.hpp"

#include <algorithm>

#include "../../shared/Components.hpp"
#include "../AllComponents.hpp"
#include "../GraphicsConstants.hpp"
#include "Logger/Macros.hpp"

namespace rc = ::rtype::games::rtype::client;
namespace rs = ::rtype::games::rtype::shared;

namespace rtype::games::rtype::client {

EnemyHealthBarSystem::EnemyHealthBarSystem(
    std::shared_ptr<ECS::Registry> registry)
    : ::rtype::engine::ASystem("EnemyHealthBarSystem"),
      _registry(std::move(registry)) {}

void EnemyHealthBarSystem::update(ECS::Registry& registry,
                                  float /*deltaTime*/) {
    auto enemyView =
        registry
            .view<rs::EnemyTag, rs::HealthComponent, rs::TransformComponent>();

    std::size_t enemyCount = 0;
    enemyView.each([&enemyCount](auto, const auto&, const auto&, const auto&) {
        enemyCount++;
    });

    static bool loggedOnce = false;
    if (!loggedOnce || enemyCount > 0) {
        LOG_INFO("[EnemyHealthBarSystem] Found " << enemyCount
                                                 << " enemies with health");
        loggedOnce = true;
    }

    enemyView.each([this, &registry](auto entity, const auto& /*enemyTag*/,
                                     const auto& health,
                                     const auto& /*transform*/) {
        if (registry.hasComponent<rs::DestroyTag>(entity)) {
            removeHealthBar(entity, registry);
            return;
        }
        if (_healthBarBgs.find(entity.id) == _healthBarBgs.end()) {
            createHealthBar(entity, registry);
        }
        updateHealthBar(entity, registry);
    });

    std::vector<std::uint32_t> toRemove;
    for (const auto& [enemyId, barEntity] : _healthBarBgs) {
        ECS::Entity enemy{enemyId};
        if (!registry.isAlive(enemy) ||
            !registry.hasComponent<rs::EnemyTag>(enemy)) {
            toRemove.push_back(enemyId);
        }
    }
    for (auto enemyId : toRemove) {
        removeHealthBar(ECS::Entity{enemyId}, registry);
    }
}

void EnemyHealthBarSystem::createHealthBar(ECS::Entity enemy,
                                           ECS::Registry& registry) {
    LOG_INFO("[EnemyHealthBarSystem] Creating health bar for enemy "
             << enemy.id);

    auto bg = registry.spawnEntity();
    registry.emplaceComponent<rs::TransformComponent>(bg, 0.0f, 0.0f);
    registry.emplaceComponent<Rectangle>(
        bg, std::pair<float, float>{HEALTH_BAR_WIDTH, HEALTH_BAR_HEIGHT},
        ::rtype::display::Color(50, 50, 50, 200),
        ::rtype::display::Color(50, 50, 50, 200));
    registry.emplaceComponent<ZIndex>(bg, 2);
    registry.emplaceComponent<GameTag>(bg);
    auto fill = registry.spawnEntity();
    registry.emplaceComponent<rs::TransformComponent>(fill, 0.0f, 0.0f);
    registry.emplaceComponent<Rectangle>(
        fill, std::pair<float, float>{HEALTH_BAR_WIDTH, HEALTH_BAR_HEIGHT},
        ::rtype::display::Color(0, 200, 0, 220),
        ::rtype::display::Color(0, 200, 0, 220));
    registry.emplaceComponent<ZIndex>(fill, 3);
    registry.emplaceComponent<GameTag>(fill);
    _healthBarBgs[enemy.id] = bg;
    _healthBarFills[enemy.id] = fill;
    LOG_INFO("[EnemyHealthBarSystem] Health bar created: bg="
             << bg.id << " fill=" << fill.id);
}

void EnemyHealthBarSystem::updateHealthBar(ECS::Entity enemy,
                                           ECS::Registry& registry) {
    auto bgIt = _healthBarBgs.find(enemy.id);
    auto fillIt = _healthBarFills.find(enemy.id);

    if (bgIt == _healthBarBgs.end() || fillIt == _healthBarFills.end()) {
        return;
    }

    auto bgEntity = bgIt->second;
    auto fillEntity = fillIt->second;
    if (!registry.isAlive(bgEntity) || !registry.isAlive(fillEntity)) {
        removeHealthBar(enemy, registry);
        return;
    }
    if (!registry.hasComponent<rs::TransformComponent>(enemy) ||
        !registry.hasComponent<rs::HealthComponent>(enemy)) {
        return;
    }

    const auto& enemyPos = registry.getComponent<rs::TransformComponent>(enemy);
    const auto& health = registry.getComponent<rs::HealthComponent>(enemy);
    float barX = enemyPos.x - (HEALTH_BAR_WIDTH / 2.0f);
    float barY = enemyPos.y + HEALTH_BAR_OFFSET_Y;
    auto& bgPos = registry.getComponent<rs::TransformComponent>(bgEntity);
    bgPos.x = barX;
    bgPos.y = barY;
    auto& fillPos = registry.getComponent<rs::TransformComponent>(fillEntity);
    fillPos.x = barX;
    fillPos.y = barY;
    auto& fillRect = registry.getComponent<Rectangle>(fillEntity);
    float healthRatio =
        health.max > 0
            ? std::max(0.0f, std::min(1.0f, static_cast<float>(health.current) /
                                                static_cast<float>(health.max)))
            : 0.0f;
    fillRect.size.first = HEALTH_BAR_WIDTH * healthRatio;

    if (healthRatio > 0.6f) {
        fillRect.currentColor = ::rtype::display::Color(0, 200, 0, 220);
    } else if (healthRatio > 0.3f) {
        fillRect.currentColor = ::rtype::display::Color(200, 200, 0, 220);
    } else {
        fillRect.currentColor = ::rtype::display::Color(200, 0, 0, 220);
    }
}

void EnemyHealthBarSystem::removeHealthBar(ECS::Entity enemy,
                                           ECS::Registry& registry) {
    auto bgIt = _healthBarBgs.find(enemy.id);
    auto fillIt = _healthBarFills.find(enemy.id);

    if (bgIt != _healthBarBgs.end()) {
        auto bgEntity = bgIt->second;
        if (registry.isAlive(bgEntity)) {
            registry.emplaceComponent<rs::DestroyTag>(bgEntity);
        }
        _healthBarBgs.erase(bgIt);
    }

    if (fillIt != _healthBarFills.end()) {
        auto fillEntity = fillIt->second;
        if (registry.isAlive(fillEntity)) {
            registry.emplaceComponent<rs::DestroyTag>(fillEntity);
        }
        _healthBarFills.erase(fillIt);
    }

    LOG_DEBUG_CAT(
        ::rtype::LogCategory::UI,
        "[EnemyHealthBarSystem] Removed health bar for enemy " << enemy.id);
}

}  // namespace rtype::games::rtype::client
