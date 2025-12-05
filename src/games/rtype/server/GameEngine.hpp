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

#include <rtype/engine.hpp>

#include "../shared/Systems/Systems.hpp"
#include "Systems/Systems.hpp"
#include "ecs/ECS.hpp"

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
    GameEngine();
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

    /**
     * @brief Get the ECS registry (for testing purposes)
     * @return Reference to the ECS registry
     */
    ECS::Registry& getRegistry() { return _registry; }
    const ECS::Registry& getRegistry() const { return _registry; }

   private:
    /**
     * @brief Emit a game event
     * @param event The event to emit
     */
    void emitEvent(const engine::GameEvent& event);

    ECS::Registry _registry;
    ECS::SystemScheduler _systemScheduler;

    bool _running = false;

    std::unique_ptr<SpawnerSystem> _spawnerSystem;
    std::unique_ptr<shared::AISystem> _aiSystem;
    std::unique_ptr<shared::MovementSystem> _movementSystem;
    std::unique_ptr<CleanupSystem> _cleanupSystem;
    std::unique_ptr<DestroySystem> _destroySystem;

    EventCallback _eventCallback;
    std::vector<engine::GameEvent> _pendingEvents;
    mutable std::mutex _eventMutex;
};

}  // namespace rtype::games::rtype::server
