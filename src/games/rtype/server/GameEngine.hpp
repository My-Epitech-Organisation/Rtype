/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** GameEngine - Server-side game engine implementation
*/

#pragma once

#include <memory>
#include <mutex>
#include <vector>

#include <rtype/ecs.hpp>
#include <rtype/engine.hpp>

#include "../shared/Systems/Systems.hpp"
#include "Systems/Systems.hpp"

namespace rtype::games::rtype::server {

/**
 * @brief Game configuration constants
 *
 * These define the game world boundaries and spawn parameters.
 * @attention Will be externalized to configuration files in the future.
 */
struct GameConfig {
    // Screen boundaries
    static constexpr float SCREEN_WIDTH = 1920.0F;
    static constexpr float SCREEN_HEIGHT = 1080.0F;
    static constexpr float SPAWN_MARGIN = 50.0F;

    // Spawn parameters
    static constexpr float MIN_SPAWN_INTERVAL = 1.0F;
    static constexpr float MAX_SPAWN_INTERVAL = 3.0F;
    static constexpr std::size_t MAX_ENEMIES = 50;

    // Cleanup boundaries (destroy entities outside these bounds)
    static constexpr float CLEANUP_LEFT = -100.0F;
    static constexpr float CLEANUP_RIGHT = 2020.0F;
    static constexpr float CLEANUP_TOP = -100.0F;
    static constexpr float CLEANUP_BOTTOM = 1180.0F;

    // Enemy parameters
    static constexpr float BYDOS_SLAVE_SPEED = 100.0F;
};

/**
 * @brief Server-side game engine for R-Type
 *
 * Manages the game state, ECS registry, and systems.
 * Emits events for entity state changes to be sent over the network.
 *
 * Thread safety:
 * - Event callback is thread-safe
 * - Pending events queue is thread-safe
 * - update() should be called from a single thread
 */
class GameEngine : public engine::AGameEngine {
   public:
    /**
     * @brief Construct GameEngine with a shared registry
     * @param registry Shared pointer to the ECS registry (must not be null)
     */
    explicit GameEngine(std::shared_ptr<ECS::Registry> registry);
    ~GameEngine() override;

    GameEngine(const GameEngine&) = delete;
    GameEngine& operator=(const GameEngine&) = delete;
    GameEngine(GameEngine&&) = delete;
    GameEngine& operator=(GameEngine&&) = delete;

    bool initialize() override;
    void update(float deltaTime) override;
    void shutdown() override;
    void setEventCallback(EventCallback callback) override;
    std::vector<engine::GameEvent> getPendingEvents() override;
    void clearPendingEvents() override;
    std::size_t getEntityCount() const override;
    bool isRunning() const override;
    engine::ProcessedEvent processEvent(
        const engine::GameEvent& event) override;
    void syncEntityPositions(
        std::function<void(uint32_t, float, float, float, float)> callback)
        override;

    /**
     * @brief Spawn a projectile for a player (IGameEngine interface)
     */
    uint32_t spawnProjectile(uint32_t playerNetworkId, float playerX,
                             float playerY) override;

    /**
     * @brief Update player positions based on velocity (IGameEngine interface)
     */
    void updatePlayerPositions(
        float deltaTime,
        std::function<void(uint32_t, float, float, float, float)>
            positionCallback) override;

    /**
     * @brief Set velocity for a player entity (IGameEngine interface)
     */
    bool setPlayerVelocity(uint32_t networkId, float vx, float vy) override;

    /**
     * @brief Get position and velocity of a player (IGameEngine interface)
     */
    [[nodiscard]] std::optional<engine::PlayerState> getPlayerPosition(
        uint32_t networkId) const override;

    /**
     * @brief Get the ECS registry
     * @return Reference to the ECS registry
     */
    ECS::Registry& getRegistry() { return *_registry; }
    const ECS::Registry& getRegistry() const { return *_registry; }

    /**
     * @brief Get projectile spawner system (for advanced configuration)
     * @return Pointer to ProjectileSpawnerSystem
     */
    std::unique_ptr<ProjectileSpawnerSystem>& getProjectileSpawner() {
        return _projectileSpawnerSystem;
    }

   private:
    /**
     * @brief Emit a game event
     * @param event The event to emit
     */
    void emitEvent(const engine::GameEvent& event);

    std::shared_ptr<ECS::Registry> _registry;
    std::unique_ptr<ECS::SystemScheduler> _systemScheduler;

    bool _running = false;

    std::unique_ptr<SpawnerSystem> _spawnerSystem;
    std::unique_ptr<ProjectileSpawnerSystem> _projectileSpawnerSystem;
    std::unique_ptr<shared::AISystem> _aiSystem;
    std::unique_ptr<shared::MovementSystem> _movementSystem;
    std::unique_ptr<shared::LifetimeSystem> _lifetimeSystem;
    std::unique_ptr<CollisionSystem> _collisionSystem;
    std::unique_ptr<CleanupSystem> _cleanupSystem;
    std::unique_ptr<DestroySystem> _destroySystem;

    EventCallback _eventCallback;
    std::vector<engine::GameEvent> _pendingEvents;
    mutable std::mutex _eventMutex;
};

/**
 * @brief Register RType game engine with the factory
 *
 * This function must be called once during application startup
 * to register the RType game engine with the GameEngineFactory.
 * This is typically done automatically via static initialization,
 * but can be called explicitly if needed.
 */
void registerRTypeGameEngine();

}  // namespace rtype::games::rtype::server
