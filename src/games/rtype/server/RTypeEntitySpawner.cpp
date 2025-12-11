/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** RTypeEntitySpawner - Implementation
*/

#include "RTypeEntitySpawner.hpp"

#include "../shared/Components/BoundingBoxComponent.hpp"
#include "../shared/Components/CooldownComponent.hpp"
#include "../shared/Components/HealthComponent.hpp"
#include "../shared/Components/NetworkIdComponent.hpp"
#include "../shared/Components/PositionComponent.hpp"
#include "../shared/Components/Tags.hpp"
#include "../shared/Components/TransformComponent.hpp"
#include "../shared/Components/VelocityComponent.hpp"
#include "../shared/Components/WeaponComponent.hpp"
#include "GameEngine.hpp"
#include "server/serverApp/game/entitySpawnerFactory/EntitySpawnerFactory.hpp"

namespace rtype::games::rtype::server {

RTypeEntitySpawner::RTypeEntitySpawner(
    std::shared_ptr<ECS::Registry> registry,
    ::rtype::server::ServerNetworkSystem& networkSystem,
    GameEngineOpt gameEngine,
    GameConfigOpt gameConfig)
    : _registry(std::move(registry)),
      _networkSystem(networkSystem),
      _gameEngine(gameEngine),
      _gameConfig(gameConfig) {}

::rtype::server::PlayerSpawnResult RTypeEntitySpawner::spawnPlayer(
    const ::rtype::server::PlayerSpawnConfig& config) {
    using shared::BoundingBoxComponent;
    using shared::HealthComponent;
    using shared::NetworkIdComponent;
    using shared::PlayerTag;
    using shared::Position;
    using shared::ShootCooldownComponent;
    using shared::TransformComponent;
    using shared::VelocityComponent;
    using shared::WeaponComponent;
    using EntityType = ::rtype::server::ServerNetworkSystem::EntityType;

    ::rtype::server::PlayerSpawnResult result{};

    if (!_registry) {
        return result;
    }

    ECS::Entity playerEntity = _registry->spawnEntity();
    float spawnX = kSpawnBaseX;
    float spawnY =
        kSpawnBaseY + static_cast<float>(config.playerIndex) * kSpawnYOffset;

    _registry->emplaceComponent<Position>(playerEntity, spawnX, spawnY);
    _registry->emplaceComponent<TransformComponent>(playerEntity, spawnX,
                                                    spawnY, 0.0F);
    _registry->emplaceComponent<VelocityComponent>(playerEntity, 0.0F, 0.0F);

    _registry->emplaceComponent<ShootCooldownComponent>(playerEntity,
                                                        kShootCooldown);
    WeaponComponent weapon{};
    weapon.weapons[0] = shared::WeaponPresets::LaserBeam;
    weapon.currentSlot = 0;
    weapon.unlockedSlots = 1;
    _registry->emplaceComponent<WeaponComponent>(playerEntity, weapon);

    _registry->emplaceComponent<BoundingBoxComponent>(
        playerEntity, kPlayerWidth, kPlayerHeight);
    _registry->emplaceComponent<PlayerTag>(playerEntity);
    _registry->emplaceComponent<HealthComponent>(
        playerEntity, kDefaultPlayerLives, kDefaultPlayerLives);

    std::uint32_t networkId = config.userId;
    _registry->emplaceComponent<NetworkIdComponent>(playerEntity, networkId);

    _networkSystem.get().registerNetworkedEntity(playerEntity, networkId,
                                            EntityType::Player, spawnX, spawnY);
    _networkSystem.get().updateEntityHealth(networkId, kDefaultPlayerLives,
                                       kDefaultPlayerLives);
    _networkSystem.get().setPlayerEntity(config.userId, playerEntity);

    result.entity = playerEntity;
    result.networkId = networkId;
    result.x = spawnX;
    result.y = spawnY;
    result.health = kDefaultPlayerLives;
    result.maxHealth = kDefaultPlayerLives;
    result.success = true;

    return result;
}

void RTypeEntitySpawner::destroyPlayer(ECS::Entity entity) {
    if (!_registry) {
        return;
    }

    _networkSystem.get().unregisterNetworkedEntity(entity);
    _registry->killEntity(entity);
}

float RTypeEntitySpawner::getPlayerSpeed() const noexcept {
    if (_gameConfig && _gameConfig->get().isInitialized()) {
        return _gameConfig->get().getGameplaySettings().playerSpeed;
    }
    return kDefaultPlayerSpeed;
}

std::uint32_t RTypeEntitySpawner::handlePlayerShoot(
    ECS::Entity playerEntity, std::uint32_t playerNetworkId) {
    if (!_gameEngine || !_registry) {
        return 0;
    }

    float playerX = 0.0F;
    float playerY = 0.0F;
    if (!getEntityPosition(playerEntity, playerX, playerY)) {
        return 0;
    }

    auto* rtypeEngine = dynamic_cast<GameEngine*>(&_gameEngine->get());
    if (!rtypeEngine) {
        return 0;
    }

    return rtypeEngine->spawnPlayerProjectile(playerNetworkId, playerX,
                                              playerY);
}

bool RTypeEntitySpawner::canPlayerShoot(ECS::Entity playerEntity) const {
    if (!_registry) {
        return false;
    }

    if (!_registry->hasComponent<shared::ShootCooldownComponent>(
            playerEntity)) {
        return false;
    }

    const auto& cooldown =
        _registry->getComponent<shared::ShootCooldownComponent>(playerEntity);
    return cooldown.canShoot();
}

std::optional<std::uint32_t> RTypeEntitySpawner::getEntityNetworkId(
    ECS::Entity entity) const {
    if (!_registry) {
        return std::nullopt;
    }

    if (!_registry->hasComponent<shared::NetworkIdComponent>(entity)) {
        return std::nullopt;
    }

    return _registry->getComponent<shared::NetworkIdComponent>(entity)
        .networkId;
}

bool RTypeEntitySpawner::getEntityPosition(ECS::Entity entity, float& outX,
                                           float& outY) const {
    if (!_registry) {
        return false;
    }

    if (!_registry->hasComponent<shared::Position>(entity)) {
        return false;
    }

    const auto& pos = _registry->getComponent<shared::Position>(entity);
    outX = pos.x;
    outY = pos.y;
    return true;
}

void RTypeEntitySpawner::updatePlayerVelocity(ECS::Entity entity, float vx,
                                              float vy) {
    if (!_registry) {
        return;
    }

    if (_registry->hasComponent<shared::VelocityComponent>(entity)) {
        auto& vel = _registry->getComponent<shared::VelocityComponent>(entity);
        vel.vx = vx;
        vel.vy = vy;
    }
}

void RTypeEntitySpawner::triggerShootCooldown(ECS::Entity entity) {
    if (!_registry) {
        return;
    }

    if (_registry->hasComponent<shared::ShootCooldownComponent>(entity)) {
        auto& cooldown =
            _registry->getComponent<shared::ShootCooldownComponent>(entity);
        cooldown.triggerCooldown();
    }
}

std::unique_ptr<::rtype::server::IEntitySpawner> createRTypeEntitySpawner(
    std::shared_ptr<ECS::Registry> registry,
    ::rtype::server::ServerNetworkSystem& networkSystem,
    GameEngineOpt gameEngine,
    GameConfigOpt gameConfig) {
    return std::make_unique<RTypeEntitySpawner>(
        std::move(registry), networkSystem, gameEngine, gameConfig);
}

void registerRTypeEntitySpawner() {
    static bool registered = false;
    if (registered) {
        return;
    }
    registered = true;

    ::rtype::server::EntitySpawnerFactory::registerSpawner(
        "rtype", [](std::shared_ptr<ECS::Registry> registry,
                    ::rtype::server::ServerNetworkSystem& networkSystem,
                    ::rtype::server::GameEngineOpt gameEngine,
                    ::rtype::server::GameConfigOpt gameConfig) {
            return createRTypeEntitySpawner(std::move(registry), networkSystem,
                                            gameEngine, gameConfig);
        });
}

namespace {
struct RTypeEntitySpawnerAutoRegistrar {
    RTypeEntitySpawnerAutoRegistrar() { registerRTypeEntitySpawner(); }
} rtypeEntitySpawnerAutoRegistrar;
}  // namespace

}  // namespace rtype::games::rtype::server
