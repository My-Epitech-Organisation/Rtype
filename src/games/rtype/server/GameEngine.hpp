/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** GameEngine - Server-side game engine implementation
*/

#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <rtype/ecs.hpp>
#include <rtype/engine.hpp>

#include "../shared/Config/PrefabLoader.hpp"
#include "../shared/Systems/Systems.hpp"
#include "Systems/ForcePod/ForcePodAttachmentSystem.hpp"
#include "Systems/ForcePod/ForcePodLaunchSystem.hpp"
#include "Systems/ForcePod/ForcePodShootingSystem.hpp"
#include "Systems/Spawner/DataDrivenSpawnerSystem.hpp"
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
    static constexpr float STATIONARY_SPAWN_INSET = 150.0F;
    static constexpr float SPAWN_OFFSET = -30.0F;

    // Spawn parameters
    static constexpr float MIN_SPAWN_INTERVAL = 1.6F;
    static constexpr float MAX_SPAWN_INTERVAL = 3.6F;
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
    [[nodiscard]] std::string getGameId() const override { return "rtype"; }
    engine::ProcessedEvent processEvent(
        const engine::GameEvent& event) override;
    void syncEntityPositions(
        std::function<void(uint32_t, float, float, float, float)> callback)
        override;

    /**
     * @brief Load a level from file path
     * @param filepath Path to level configuration file
     * @return true if level loaded successfully
     */
    bool loadLevelFromFile(const std::string& filepath) override;

    /**
     * @brief Start the loaded level
     */
    void startLevel();

    /**
     * @brief Check if using data-driven spawning
     * @return true if a level is loaded and data-driven mode is active
     */
    [[nodiscard]] bool isDataDrivenMode() const noexcept {
        return _useDataDrivenSpawner;
    }

    /**
     * @brief Set whether to use data-driven spawning
     * @param enabled true to use data-driven spawner
     */
    void setDataDrivenMode(bool enabled) noexcept {
        _useDataDrivenSpawner = enabled;
    }

    /**
     * @brief Spawn a projectile for a player
     * @param playerNetworkId Network ID of the player shooting
     * @param playerX Player X position
     * @param playerY Player Y position
     * @return Network ID of the spawned projectile (0 if failed)
     */
    uint32_t spawnProjectile(uint32_t playerNetworkId, float playerX,
                             float playerY);

    /**
     * @brief Spawn a charged projectile for a player
     * @param playerNetworkId Network ID of the player shooting
     * @param playerX Player X position
     * @param playerY Player Y position
     * @param chargeLevel Charge level (1-3)
     * @return Network ID of the spawned projectile (0 if failed)
     */
    uint32_t spawnChargedProjectile(uint32_t playerNetworkId, float playerX,
                                    float playerY, uint8_t chargeLevel);

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

    /**
     * @brief Get Force Pod launch system
     * @return Pointer to ForcePodLaunchSystem
     */
    ForcePodLaunchSystem* getForcePodLaunchSystem() {
        return _forcePodLaunchSystem.get();
    }

    /**
     * @brief Get the data-driven spawner (for wave management)
     * @return Pointer to DataDrivenSpawnerSystem
     */
    DataDrivenSpawnerSystem* getDataDrivenSpawner() {
        return _dataDrivenSpawnerSystem.get();
    }

   private:
    /**
     * @brief Configure les signaux ECS pour les logs et statistiques
     */
    void setupECSSignals();
    /**
     * @brief Emit a game event
     * @param event The event to emit
     */
    void emitEvent(const engine::GameEvent& event);

    std::shared_ptr<ECS::Registry> _registry;
    std::unique_ptr<ECS::SystemScheduler> _systemScheduler;
    std::unique_ptr<ECS::PrefabManager> _prefabManager;

    bool _running = false;
    bool _useDataDrivenSpawner = true;

    std::unique_ptr<SpawnerSystem> _spawnerSystem;
    std::unique_ptr<DataDrivenSpawnerSystem> _dataDrivenSpawnerSystem;
    std::unique_ptr<ProjectileSpawnerSystem> _projectileSpawnerSystem;
    std::unique_ptr<EnemyShootingSystem> _enemyShootingSystem;
    std::unique_ptr<shared::AISystem> _aiSystem;
    std::unique_ptr<shared::MovementSystem> _movementSystem;
    std::unique_ptr<shared::LifetimeSystem> _lifetimeSystem;
    std::unique_ptr<shared::PowerUpSystem> _powerUpSystem;
    std::unique_ptr<CollisionSystem> _collisionSystem;
    std::unique_ptr<CleanupSystem> _cleanupSystem;
    std::unique_ptr<DestroySystem> _destroySystem;
    std::unique_ptr<ForcePodAttachmentSystem> _forcePodAttachmentSystem;
    std::unique_ptr<ForcePodLaunchSystem> _forcePodLaunchSystem;
    std::unique_ptr<ForcePodShootingSystem> _forcePodShootingSystem;

    EventCallback _eventCallback;
    std::vector<engine::GameEvent> _pendingEvents;
    mutable std::mutex _eventMutex;

    std::atomic<size_t> _totalEntitiesCreated{0};
    std::atomic<size_t> _totalEntitiesDestroyed{0};
    float _lastDeltaTime = 0.0f;
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
