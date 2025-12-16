/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** PlayerSpawner - Implementation
*/

#include "PlayerSpawner.hpp"

#include <rtype/common.hpp>

#include "games/rtype/shared/Components/BoundingBoxComponent.hpp"
#include "games/rtype/shared/Components/CooldownComponent.hpp"
#include "games/rtype/shared/Components/HealthComponent.hpp"
#include "games/rtype/shared/Components/NetworkIdComponent.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"
#include "games/rtype/shared/Components/Tags.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"
#include "games/rtype/shared/Components/VelocityComponent.hpp"
#include "games/rtype/shared/Components/WeaponComponent.hpp"
#include "network/ServerNetworkSystem.hpp"

namespace rtype::server {

using Position = rtype::games::rtype::shared::TransformComponent;
using TransformComponent = rtype::games::rtype::shared::TransformComponent;
using Velocity = rtype::games::rtype::shared::VelocityComponent;
using ShootCooldown = rtype::games::rtype::shared::ShootCooldownComponent;
using Weapon = rtype::games::rtype::shared::WeaponComponent;
using BoundingBox = rtype::games::rtype::shared::BoundingBoxComponent;
using PlayerTag = rtype::games::rtype::shared::PlayerTag;
using NetworkIdComponent = rtype::games::rtype::shared::NetworkIdComponent;
using Health = rtype::games::rtype::shared::HealthComponent;
using EntityType = ServerNetworkSystem::EntityType;

PlayerSpawner::PlayerSpawner(std::shared_ptr<ECS::Registry> registry,
                             ServerNetworkSystem* networkSystem,
                             const SpawnConfig& config)
    : _registry(std::move(registry)),
      _networkSystem(networkSystem),
      _config(config) {}

PlayerSpawnResult PlayerSpawner::spawnPlayer(std::uint32_t userId,
                                             size_t playerIndex) {
    PlayerSpawnResult result;
    result.networkId = userId;
    result.x = _config.baseX;
    result.y =
        _config.baseY + static_cast<float>(playerIndex) * _config.yOffset;

    ECS::Entity playerEntity = _registry->spawnEntity();
    result.entity = playerEntity;

    _registry->emplaceComponent<Position>(playerEntity, result.x, result.y);
    _registry->emplaceComponent<TransformComponent>(playerEntity, result.x,
                                                    result.y, 0.0F);
    _registry->emplaceComponent<Velocity>(playerEntity, 0.0F, 0.0F);

    _registry->emplaceComponent<ShootCooldown>(playerEntity,
                                               _config.shootCooldown);
    Weapon weapon{};
    weapon.weapons[0] = rtype::games::rtype::shared::WeaponPresets::LaserBeam;
    weapon.currentSlot = 0;
    weapon.unlockedSlots = 1;
    _registry->emplaceComponent<Weapon>(playerEntity, weapon);

    _registry->emplaceComponent<BoundingBox>(playerEntity, _config.playerWidth,
                                             _config.playerHeight);

    _registry->emplaceComponent<PlayerTag>(playerEntity);
    _registry->emplaceComponent<Health>(playerEntity, _config.playerLives,
                                        _config.playerLives);
    _registry->emplaceComponent<NetworkIdComponent>(playerEntity,
                                                    result.networkId);

    if (_networkSystem) {
        _networkSystem->registerNetworkedEntity(playerEntity, result.networkId,
                                                EntityType::Player, result.x,
                                                result.y);
        _networkSystem->updateEntityHealth(
            result.networkId, _config.playerLives, _config.playerLives);
        _networkSystem->setPlayerEntity(userId, playerEntity);
    }

    result.success = true;
    LOG_INFO("[PlayerSpawner] Spawned player for userId="
             << userId << " networkId=" << result.networkId << " pos=("
             << result.x << ", " << result.y << ")");

    return result;
}

bool PlayerSpawner::destroyPlayer(std::uint32_t userId) {
    if (!_networkSystem) {
        return false;
    }

    auto entityOpt = _networkSystem->getPlayerEntity(userId);
    if (!entityOpt.has_value()) {
        return false;
    }

    ECS::Entity playerEntity = *entityOpt;
    _networkSystem->unregisterNetworkedEntity(playerEntity);
    _registry->killEntity(playerEntity);

    LOG_INFO("[PlayerSpawner] Destroyed player entity for userId=" << userId);
    return true;
}

std::optional<ECS::Entity> PlayerSpawner::getPlayerEntity(
    std::uint32_t userId) const {
    if (!_networkSystem) {
        return std::nullopt;
    }
    return _networkSystem->getPlayerEntity(userId);
}

}  // namespace rtype::server
