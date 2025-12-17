/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** RTypeEntitySpawner - Implementation
*/

#include "RTypeEntitySpawner.hpp"

#include <algorithm>

#include "../shared/Components/BoundingBoxComponent.hpp"
#include "../shared/Components/CooldownComponent.hpp"
#include "../shared/Components/HealthComponent.hpp"
#include "../shared/Components/NetworkIdComponent.hpp"
#include "../shared/Components/PlayerIdComponent.hpp"
#include "../shared/Components/PowerUpComponent.hpp"
#include "../shared/Components/Tags.hpp"
#include "../shared/Components/TransformComponent.hpp"
#include "../shared/Components/VelocityComponent.hpp"
#include "../shared/Components/WeaponComponent.hpp"
#include "GameEngine.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"
#include "server/serverApp/game/entitySpawnerFactory/EntitySpawnerFactory.hpp"

namespace rtype::games::rtype::server {

RTypeEntitySpawner::RTypeEntitySpawner(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<::rtype::server::ServerNetworkSystem> networkSystem,
    GameEngineOpt gameEngine, GameConfigOpt gameConfig)
    : _registry(std::move(registry)),
      _networkSystem(std::move(networkSystem)),
      _gameEngine(gameEngine),
      _gameConfig(gameConfig) {}

::rtype::server::PlayerSpawnResult RTypeEntitySpawner::spawnPlayer(
    const ::rtype::server::PlayerSpawnConfig& config) {
    using shared::BoundingBoxComponent;
    using shared::HealthComponent;
    using shared::NetworkIdComponent;
    using shared::PlayerIdComponent;
    using shared::PlayerTag;
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
        playerEntity, kDefaultPlayerHealth, kDefaultPlayerHealth);

    std::uint32_t networkId = config.userId;
    _registry->emplaceComponent<NetworkIdComponent>(playerEntity, networkId);

    std::uint32_t playerId = config.playerIndex + 1;
    _registry->emplaceComponent<PlayerIdComponent>(playerEntity, playerId);

    _networkSystem->registerNetworkedEntity(playerEntity, networkId,
                                            EntityType::Player, spawnX, spawnY);
    _networkSystem->updateEntityHealth(networkId, kDefaultPlayerHealth,
                                       kDefaultPlayerHealth);
    _networkSystem->setPlayerEntity(config.userId, playerEntity);

    result.entity = playerEntity;
    result.networkId = networkId;
    result.x = spawnX;
    result.y = spawnY;
    result.health = kDefaultPlayerHealth;
    result.maxHealth = kDefaultPlayerHealth;
    result.success = true;

    return result;
}

void RTypeEntitySpawner::destroyPlayer(ECS::Entity entity) {
    if (!_registry) {
        return;
    }

    _networkSystem->unregisterNetworkedEntity(entity);
    _registry->killEntity(entity);
}

bool RTypeEntitySpawner::destroyPlayerByUserId(std::uint32_t userId) {
    if (!_networkSystem) {
        return false;
    }

    auto entityOpt = _networkSystem->getPlayerEntity(userId);
    if (!entityOpt.has_value()) {
        return false;
    }

    destroyPlayer(*entityOpt);
    return true;
}

std::optional<ECS::Entity> RTypeEntitySpawner::getPlayerEntity(
    std::uint32_t userId) const {
    if (!_networkSystem) {
        return std::nullopt;
    }
    return _networkSystem->getPlayerEntity(userId);
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

    auto posOpt = getEntityPosition(playerEntity);
    if (!posOpt.has_value()) {
        return 0;
    }

    if (!_gameEngine.has_value()) {
        return 0;
    }

    auto* rtypeEngine = dynamic_cast<GameEngine*>(&_gameEngine->get());
    if (rtypeEngine == nullptr) {
        return 0;
    }

    return rtypeEngine->spawnProjectile(playerNetworkId, posOpt->x, posOpt->y);
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

std::optional<::rtype::server::EntityPosition>
RTypeEntitySpawner::getEntityPosition(ECS::Entity entity) const {
    if (!_registry) {
        return std::nullopt;
    }

    if (!_registry->hasComponent<shared::TransformComponent>(entity)) {
        return std::nullopt;
    }

    const auto& pos =
        _registry->getComponent<shared::TransformComponent>(entity);
    return ::rtype::server::EntityPosition{pos.x, pos.y};
}

void RTypeEntitySpawner::updatePlayerVelocity(ECS::Entity entity, float vx,
                                              float vy) {
    if (!_registry) {
        return;
    }

    if (_registry->hasComponent<shared::VelocityComponent>(entity)) {
        auto& vel = _registry->getComponent<shared::VelocityComponent>(entity);
        float speedMultiplier = 1.0F;
        if (_registry->hasComponent<shared::ActivePowerUpComponent>(entity)) {
            const auto& active =
                _registry->getComponent<shared::ActivePowerUpComponent>(entity);
            speedMultiplier = active.speedMultiplier;
        }
        vel.vx = vx * speedMultiplier;
        vel.vy = vy * speedMultiplier;
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

void RTypeEntitySpawner::updateAllPlayersMovement(
    float deltaTime, const PositionUpdateCallback& callback) {
    if (!_registry) {
        return;
    }

    auto view =
        _registry
            ->view<shared::TransformComponent, shared::VelocityComponent>();
    view.each([this, deltaTime, &callback](ECS::Entity entity,
                                           shared::TransformComponent& pos,
                                           shared::VelocityComponent& vel) {
        if (vel.vx == 0.0F && vel.vy == 0.0F) {
            return;
        }

        pos.x += vel.vx * deltaTime;
        pos.y += vel.vy * deltaTime;
        pos.x = std::clamp(pos.x, kWorldMinX, kWorldMaxX);
        pos.y = std::clamp(pos.y, kWorldMinY, kWorldMaxY);

        if (_registry->hasComponent<shared::TransformComponent>(entity)) {
            auto& transform =
                _registry->getComponent<shared::TransformComponent>(entity);
            transform.x = pos.x;
            transform.y = pos.y;
        }

        auto networkIdOpt = getEntityNetworkId(entity);
        if (networkIdOpt.has_value() && callback) {
            callback(*networkIdOpt, pos.x, pos.y, vel.vx, vel.vy);
        }
    });
}

::rtype::server::WorldBounds RTypeEntitySpawner::getWorldBounds()
    const noexcept {
    return {kWorldMinX, kWorldMaxX, kWorldMinY, kWorldMaxY};
}

std::unique_ptr<::rtype::server::IEntitySpawner> createRTypeEntitySpawner(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<::rtype::server::ServerNetworkSystem> networkSystem,
    GameEngineOpt gameEngine, GameConfigOpt gameConfig) {
    return std::make_unique<RTypeEntitySpawner>(
        std::move(registry), std::move(networkSystem), gameEngine, gameConfig);
}

void registerRTypeEntitySpawner() {
    static bool registered = false;
    if (registered) {
        return;
    }
    registered = true;

    ::rtype::server::EntitySpawnerFactory::registerSpawner(
        "rtype",
        [](std::shared_ptr<ECS::Registry> registry,
           std::shared_ptr<::rtype::server::ServerNetworkSystem> networkSystem,
           ::rtype::server::GameEngineOpt gameEngine,
           ::rtype::server::GameConfigOpt gameConfig) {
            return createRTypeEntitySpawner(std::move(registry),
                                            std::move(networkSystem),
                                            gameEngine, gameConfig);
        });
}

namespace {
struct RTypeEntitySpawnerAutoRegistrar {
    RTypeEntitySpawnerAutoRegistrar() { registerRTypeEntitySpawner(); }
} rtypeEntitySpawnerAutoRegistrar;
}  // namespace

}  // namespace rtype::games::rtype::server
