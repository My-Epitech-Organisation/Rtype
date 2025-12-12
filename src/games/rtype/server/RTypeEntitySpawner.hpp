/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** RTypeEntitySpawner - RType-specific entity spawner implementation
*/

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include <rtype/ecs.hpp>
#include <rtype/engine.hpp>

#include "server/network/ServerNetworkSystem.hpp"
#include "server/shared/IEntitySpawner.hpp"
#include "server/shared/IGameConfig.hpp"

namespace rtype::games::rtype::server {

// Type aliases for optional references (matching EntitySpawnerFactory)
using GameEngineOpt =
    std::optional<std::reference_wrapper<::rtype::engine::IGameEngine>>;
using GameConfigOpt =
    std::optional<std::reference_wrapper<const ::rtype::server::IGameConfig>>;

// Forward declaration
class GameEngine;

/**
 * @class RTypeEntitySpawner
 * @brief RType-specific implementation of IEntitySpawner
 *
 * Handles spawning of player entities with RType-specific components
 * such as weapons, health, and bounding boxes.
 */
class RTypeEntitySpawner : public ::rtype::server::IEntitySpawner {
   public:
    /**
     * @brief Construct an RTypeEntitySpawner
     *
     * @param registry Shared pointer to the ECS registry
     * @param networkSystem Shared pointer to the network system
     * @param gameEngine Optional reference to the game engine
     * @param gameConfig Optional reference to the game config
     */
    RTypeEntitySpawner(
        std::shared_ptr<ECS::Registry> registry,
        std::shared_ptr<::rtype::server::ServerNetworkSystem> networkSystem,
        GameEngineOpt gameEngine, GameConfigOpt gameConfig);

    ~RTypeEntitySpawner() override = default;

    RTypeEntitySpawner(const RTypeEntitySpawner&) = delete;
    RTypeEntitySpawner& operator=(const RTypeEntitySpawner&) = delete;
    RTypeEntitySpawner(RTypeEntitySpawner&&) = delete;
    RTypeEntitySpawner& operator=(RTypeEntitySpawner&&) = delete;

    [[nodiscard]] ::rtype::server::PlayerSpawnResult spawnPlayer(
        const ::rtype::server::PlayerSpawnConfig& config) override;

    void destroyPlayer(ECS::Entity entity) override;

    [[nodiscard]] bool destroyPlayerByUserId(std::uint32_t userId) override;

    [[nodiscard]] std::optional<ECS::Entity> getPlayerEntity(
        std::uint32_t userId) const override;

    [[nodiscard]] float getPlayerSpeed() const noexcept override;

    [[nodiscard]] ::rtype::server::WorldBounds getWorldBounds()
        const noexcept override;

    [[nodiscard]] std::string getGameId() const noexcept override {
        return "rtype";
    }

    [[nodiscard]] std::uint32_t handlePlayerShoot(
        ECS::Entity playerEntity, std::uint32_t playerNetworkId) override;

    [[nodiscard]] bool canPlayerShoot(ECS::Entity playerEntity) const override;

    void triggerShootCooldown(ECS::Entity entity) override;

    [[nodiscard]] std::optional<std::uint32_t> getEntityNetworkId(
        ECS::Entity entity) const override;

    [[nodiscard]] std::optional<::rtype::server::EntityPosition>
    getEntityPosition(ECS::Entity entity) const override;

    void updatePlayerVelocity(ECS::Entity entity, float vx, float vy) override;

    void updateAllPlayersMovement(
        float deltaTime, const PositionUpdateCallback& callback) override;

   private:
    std::shared_ptr<ECS::Registry> _registry;
    std::shared_ptr<::rtype::server::ServerNetworkSystem> _networkSystem;
    GameEngineOpt _gameEngine;
    GameConfigOpt _gameConfig;

    static constexpr int kDefaultPlayerHealth = 100;
    static constexpr float kDefaultPlayerSpeed = 250.0F;
    static constexpr float kPlayerWidth = 33.0F;
    static constexpr float kPlayerHeight = 17.0F;
    static constexpr float kSpawnBaseX = 100.0F;
    static constexpr float kSpawnBaseY = 150.0F;
    static constexpr float kSpawnYOffset = 100.0F;
    static constexpr float kShootCooldown = 0.3F;
    static constexpr float kWorldMinX = 0.0F;
    static constexpr float kWorldMaxX = 1920.0F - 64.0F;
    static constexpr float kWorldMinY = 0.0F;
    static constexpr float kWorldMaxY = 1080.0F - 64.0F;
};

/**
 * @brief Factory function to create an RType entity spawner
 *
 * @param registry Shared pointer to the ECS registry
 * @param networkSystem Shared pointer to the network system
 * @param gameEngine Optional reference to the game engine
 * @param gameConfig Optional reference to the game config
 * @return Unique pointer to the spawner
 */
std::unique_ptr<::rtype::server::IEntitySpawner> createRTypeEntitySpawner(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<::rtype::server::ServerNetworkSystem> networkSystem,
    GameEngineOpt gameEngine, GameConfigOpt gameConfig);

/**
 * @brief Register RType entity spawner with the factory
 *
 * This function must be called once during application startup
 * to register the RType entity spawner with the EntitySpawnerFactory.
 */
void registerRTypeEntitySpawner();

}  // namespace rtype::games::rtype::server
