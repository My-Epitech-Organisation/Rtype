/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** SpawnerSystem - Server enemy spawning implementation
*/

#include "SpawnerSystem.hpp"

#include "../../../shared/Components.hpp"

namespace rtype::games::rtype::server {

using shared::AIBehavior;
using shared::AIComponent;
using shared::BoundingBoxComponent;
using shared::BydosSlaveTag;
using shared::EnemyTag;
using shared::EntityType;
using shared::HealthComponent;
using shared::NetworkIdComponent;
using shared::TransformComponent;
using shared::VelocityComponent;

SpawnerSystem::SpawnerSystem(EventEmitter emitter, SpawnerConfig config)
    : _emitEvent(std::move(emitter)),
      _config(config),
      _rng(std::random_device {}()),
      _spawnTimeDist(config.minSpawnInterval, config.maxSpawnInterval),
      _spawnYDist(config.minSpawnY, config.maxSpawnY) {
    generateNextSpawnTime();
}

void SpawnerSystem::update(ECS::Registry& registry, float deltaTime) {
    _spawnTimer += deltaTime;

    if (_spawnTimer >= _nextSpawnTime && _enemyCount < _config.maxEnemies) {
        spawnBydosSlave(registry);
        _spawnTimer = 0.0F;
        generateNextSpawnTime();
    }
}

void SpawnerSystem::spawnBydosSlave(ECS::Registry& registry) {
    ECS::Entity enemy = registry.spawnEntity();
    float spawnY = _spawnYDist(_rng);

    registry.emplaceComponent<TransformComponent>(enemy, _config.spawnX, spawnY,
                                                  0.0F);
    registry.emplaceComponent<VelocityComponent>(
        enemy, -_config.bydosSlaveSpeed, 0.0F);
    registry.emplaceComponent<AIComponent>(enemy, AIBehavior::MoveLeft,
                                           _config.bydosSlaveSpeed);
    constexpr int BYDOS_SLAVE_HEALTH = 10;
    registry.emplaceComponent<HealthComponent>(enemy, BYDOS_SLAVE_HEALTH,
                                               BYDOS_SLAVE_HEALTH);

    constexpr float BYDOS_SLAVE_SIZE = 32.0F;
    registry.emplaceComponent<BoundingBoxComponent>(enemy, BYDOS_SLAVE_SIZE,
                                                    BYDOS_SLAVE_SIZE);

    uint32_t networkId = _nextNetworkId++;
    registry.emplaceComponent<NetworkIdComponent>(enemy, networkId);

    registry.emplaceComponent<EnemyTag>(enemy);
    registry.emplaceComponent<BydosSlaveTag>(enemy);

    _enemyCount++;

    engine::GameEvent event{};
    event.type = engine::GameEventType::EntitySpawned;
    event.entityNetworkId = networkId;
    event.x = _config.spawnX;
    event.y = spawnY;
    event.rotation = 0.0F;
    event.entityType = static_cast<uint8_t>(EntityType::Enemy);
    _emitEvent(event);
}

void SpawnerSystem::generateNextSpawnTime() {
    _nextSpawnTime = _spawnTimeDist(_rng);
}

}  // namespace rtype::games::rtype::server
