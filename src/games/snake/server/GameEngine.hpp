/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Snake Game Engine - Server-side implementation
*/

#pragma once

#include <memory>
#include <random>
#include <vector>

#include <rtype/ecs.hpp>
#include <rtype/engine.hpp>

#include "../shared/Components.hpp"

namespace rtype::games::snake::server {

/**
 * @brief Snake Game Engine Configuration
 */
struct SnakeGameConfig {
    static constexpr int GRID_WIDTH = 20;
    static constexpr int GRID_HEIGHT = 20;
    static constexpr float CELL_SIZE = 30.0F;

    static constexpr float MOVE_INTERVAL = 0.12F;
    static constexpr int INITIAL_LENGTH = 3;
    static constexpr float FOOD_SPAWN_INTERVAL = 2.0F;
};

/**
 * @brief Snake Game Engine - Multiplayer Snake using the generic game engine
 *
 * This demonstrates that the GameEngineFactory pattern allows creating
 * completely different game types using the same infrastructure.
 */
class SnakeGameEngine : public engine::AGameEngine {
   public:
    /**
     * @brief Construct SnakeGameEngine with a shared ECS registry
     * @param registry Shared pointer to the ECS registry
     */
    explicit SnakeGameEngine(std::shared_ptr<ECS::Registry> registry);
    ~SnakeGameEngine() override;

    // Prevent copy/move
    SnakeGameEngine(const SnakeGameEngine&) = delete;
    SnakeGameEngine& operator=(const SnakeGameEngine&) = delete;
    SnakeGameEngine(SnakeGameEngine&&) = delete;
    SnakeGameEngine& operator=(SnakeGameEngine&&) = delete;

    /**
     * @brief Initialize the snake game engine
     * @return true if initialization succeeded
     */
    bool initialize() override;

    /**
     * @brief Update game state
     * @param deltaTime Time elapsed since last update in seconds
     */
    void update(float deltaTime) override;

    /**
     * @brief Shutdown the engine and release resources
     */
    void shutdown() override;

    /**
     * @brief Process a game event
     * @param event The game event to process
     * @return Processed event with network data
     */
    engine::ProcessedEvent processEvent(
        const engine::GameEvent& event) override;

    /**
     * @brief Sync entity positions (for network transmission)
     * @param callback Callback to receive position updates
     */
    void syncEntityPositions(
        std::function<void(uint32_t, float, float, float, float)> callback)
        override;

    /**
     * @brief Get the game identifier
     * @return "snake"
     */
    [[nodiscard]] std::string getGameId() const override { return "snake"; }

   private:
    std::shared_ptr<ECS::Registry> _registry;
    float _moveTimer = 0.0F;
    bool _foodEaten = false;

    // Thread-safe RNG for spawning food
    std::mt19937 _rng;

    /**
     * @brief Setup ECS systems for snake game
     */
    void setupSystems();

    /**
     * @brief Spawn initial snakes for each player
     */
    void spawnInitialSnakes();

    /**
     * @brief Spawn food at random position
     */
    void spawnFood();
};

/**
 * @brief Register snake game engine with the factory
 *
 * Must be called during application startup.
 */
void registerSnakeGameEngine();

}  // namespace rtype::games::snake::server
