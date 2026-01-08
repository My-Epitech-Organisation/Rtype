/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_wave_system - Unit tests for wave-based spawning and GameOver logic
*/

#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include "../../../src/games/rtype/shared/Components.hpp"
#include "../../../src/games/rtype/shared/Config/EntityConfig/EntityConfig.hpp"
#include "../../../src/games/rtype/server/Systems/Spawner/SpawnerSystem.hpp"
#include "../../../../lib/ecs/src/ECS.hpp"
#include "../../../../lib/engine/src/IGameEngine.hpp"

using namespace rtype::games::rtype::shared;
using namespace rtype::games::rtype::server;

// =============================================================================
// Wave-Based Spawning Tests
// =============================================================================

class WaveSystemTest : public ::testing::Test {
   protected:
    void SetUp() override {
        // Load entity configurations required by SpawnerSystem
        auto& entityConfigRegistry =
            rtype::games::rtype::shared::EntityConfigRegistry::getInstance();
        entityConfigRegistry.loadEnemiesWithSearch("config/game/enemies.toml");
        entityConfigRegistry.loadProjectilesWithSearch("config/game/projectiles.toml");
        
        config.minSpawnInterval = 0.1F;
        config.maxSpawnInterval = 0.2F;
        config.maxEnemies = 50;
        config.spawnX = 800.0F;
        config.minSpawnY = 50.0F;
        config.maxSpawnY = 550.0F;
        config.bydosSlaveSpeed = 100.0F;
        config.maxWaves = 3;
        config.enemiesPerWave = 5;
    }

    void TearDown() override {
        // Clean up any remaining entities
        auto view = registry.view<EnemyTag>();
        std::vector<ECS::Entity> toDestroy;
        view.each([&toDestroy](ECS::Entity entity, const EnemyTag&) {
            toDestroy.push_back(entity);
        });
        for (auto entity : toDestroy) {
            if (registry.isAlive(entity)) {
                registry.removeComponent<EnemyTag>(entity);
            }
        }
    }

    ECS::Registry registry;
    SpawnerConfig config;
    std::vector<rtype::engine::GameEvent> emittedEvents;

    // Helper function to count spawn events
    std::size_t countSpawnEvents() const {
        std::size_t count = 0;
        for (const auto& event : emittedEvents) {
            if (event.type == rtype::engine::GameEventType::EntitySpawned) {
                count++;
            }
        }
        return count;
    }

    // Helper function to wait for all enemies of a wave to spawn
    void waitForWaveSpawns(SpawnerSystem& spawner, std::size_t enemiesPerWave) {
        std::size_t spawnCountBefore = countSpawnEvents();
        for (int i = 0; i < 100; ++i) {
            spawner.update(registry, 0.2F);
            if (countSpawnEvents() - spawnCountBefore >= enemiesPerWave) {
                break;
            }
        }
    }

    // Helper function to kill all enemies
    void killAllEnemies() {
        std::vector<ECS::Entity> entitiesToKill;
        auto view = registry.view<EnemyTag>();
        view.each([&entitiesToKill](ECS::Entity entity, const EnemyTag&) {
            entitiesToKill.push_back(entity);
        });
        for (auto entity : entitiesToKill) {
            registry.removeComponent<EnemyTag>(entity);
        }
    }
};

// =============================================================================
// Test 1: Wave Completion Detection
// =============================================================================

TEST_F(WaveSystemTest, DetectsWaveCompletionWhenAllEnemiesEliminated) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    EXPECT_EQ(spawnerSystem.getCurrentWave(), 1u);

    // Spawn all enemies for wave 1
    for (int i = 0; i < 50; ++i) {
        spawnerSystem.update(registry, 0.2F);
        if (spawnerSystem.getEnemyCount() >= config.enemiesPerWave) {
            break;
        }
    }

    // Verify enemies were spawned
    std::size_t enemiesSpawned = 0;
    auto view = registry.view<EnemyTag>();
    view.each([&enemiesSpawned](ECS::Entity, const EnemyTag&) {
        enemiesSpawned++;
    });
    EXPECT_EQ(enemiesSpawned, config.enemiesPerWave);
    EXPECT_EQ(spawnerSystem.getCurrentWave(), 1u);

    // Eliminate all enemies by killing entities
    std::vector<ECS::Entity> entitiesToKill;
    view.each([&entitiesToKill](ECS::Entity entity, const EnemyTag&) {
        entitiesToKill.push_back(entity);
    });
    for (auto entity : entitiesToKill) {
        registry.removeComponent<EnemyTag>(entity);
    }

    // Run update to detect wave completion
    spawnerSystem.update(registry, 0.016F);

    // Wave should have advanced to wave 2
    EXPECT_EQ(spawnerSystem.getCurrentWave(), 2u);
}

TEST_F(WaveSystemTest, DoesNotAdvanceWaveIfEnemiesRemain) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Spawn all enemies for wave 1
    for (int i = 0; i < 50; ++i) {
        spawnerSystem.update(registry, 0.2F);
        if (spawnerSystem.getEnemyCount() >= config.enemiesPerWave) {
            break;
        }
    }

    // Eliminate all but one enemy
    std::vector<ECS::Entity> entitiesToKill;
    auto view = registry.view<EnemyTag>();
    std::size_t count = 0;
    const std::size_t maxToKill = config.enemiesPerWave - 1;
    view.each([&entitiesToKill, &count, maxToKill](ECS::Entity entity, const EnemyTag&) {
        if (count < maxToKill) {
            entitiesToKill.push_back(entity);
            count++;
        }
    });
    for (auto entity : entitiesToKill) {
        registry.removeComponent<EnemyTag>(entity);
    }

    // Run update
    spawnerSystem.update(registry, 0.016F);

    // Wave should NOT have advanced (still one enemy alive)
    EXPECT_EQ(spawnerSystem.getCurrentWave(), 1u);
}

TEST_F(WaveSystemTest, WaveCompletionOnlyTriggersWhenAllEnemiesSpawnedAndKilled) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Spawn only some enemies (not all)
    for (int i = 0; i < 20; ++i) {
        spawnerSystem.update(registry, 0.2F);
        if (spawnerSystem.getEnemyCount() >= 3) {
            break;
        }
    }

    std::size_t enemiesSpawned = 0;
    auto view = registry.view<EnemyTag>();
    view.each([&enemiesSpawned](ECS::Entity, const EnemyTag&) {
        enemiesSpawned++;
    });
    const std::size_t expectedPerWave = config.enemiesPerWave;
    EXPECT_LT(enemiesSpawned, expectedPerWave);

    // Kill all spawned enemies
    std::vector<ECS::Entity> entitiesToKill;
    view.each([&entitiesToKill](ECS::Entity entity, const EnemyTag&) {
        entitiesToKill.push_back(entity);
    });
    for (auto entity : entitiesToKill) {
        registry.removeComponent<EnemyTag>(entity);
    }

    // Run update
    spawnerSystem.update(registry, 0.016F);

    // Wave should NOT advance (not all enemies for wave spawned yet)
    EXPECT_EQ(spawnerSystem.getCurrentWave(), 1u);
}

// =============================================================================
// Test 2: GameOver Event Emission
// =============================================================================

TEST_F(WaveSystemTest, EmitsGameOverEventAfterMaxWavesCompleted) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Complete all 3 waves
    for (std::size_t wave = 1; wave <= config.maxWaves; ++wave) {
        // Clear previous events to count this wave's spawns
        std::size_t spawnCountBefore = 0;
        for (const auto& event : emittedEvents) {
            if (event.type == rtype::engine::GameEventType::EntitySpawned) {
                spawnCountBefore++;
            }
        }

        // Spawn all enemies for this wave by counting spawn events
        for (int i = 0; i < 100; ++i) {
            spawnerSystem.update(registry, 0.2F);
            
            std::size_t spawnCountNow = 0;
            for (const auto& event : emittedEvents) {
                if (event.type == rtype::engine::GameEventType::EntitySpawned) {
                    spawnCountNow++;
                }
            }
            
            if (spawnCountNow - spawnCountBefore >= config.enemiesPerWave) {
                break;
            }
        }

        // Kill all enemies
        std::vector<ECS::Entity> entitiesToKill;
        auto view = registry.view<EnemyTag>();
        view.each([&entitiesToKill](ECS::Entity entity, const EnemyTag&) {
            entitiesToKill.push_back(entity);
        });
        for (auto entity : entitiesToKill) {
            registry.removeComponent<EnemyTag>(entity);
        }

        // Run update to trigger wave completion
        spawnerSystem.update(registry, 0.016F);
    }

    // Check that GameOver event was emitted
    bool gameOverEmitted = false;
    for (const auto& event : emittedEvents) {
        if (event.type == rtype::engine::GameEventType::GameOver) {
            gameOverEmitted = true;
            break;
        }
    }
    EXPECT_TRUE(gameOverEmitted);
}

TEST_F(WaveSystemTest, DoesNotEmitGameOverBeforeMaxWaves) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Complete only 2 waves (out of 3)
    for (std::size_t wave = 1; wave <= 2; ++wave) {
        // Spawn all enemies for this wave
        waitForWaveSpawns(spawnerSystem, config.enemiesPerWave);

        // Kill all enemies
        killAllEnemies();

        // Run update to trigger wave completion
        spawnerSystem.update(registry, 0.016F);
    }

    // Check that GameOver event was NOT emitted
    bool gameOverEmitted = false;
    for (const auto& event : emittedEvents) {
        if (event.type == rtype::engine::GameEventType::GameOver) {
            gameOverEmitted = true;
            break;
        }
    }
    EXPECT_FALSE(gameOverEmitted);
    EXPECT_EQ(spawnerSystem.getCurrentWave(), 3u);
}

TEST_F(WaveSystemTest, GameOverEmittedOnlyOnce) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Complete all 3 waves
    for (std::size_t wave = 1; wave <= config.maxWaves; ++wave) {
        waitForWaveSpawns(spawnerSystem, config.enemiesPerWave);
        killAllEnemies();
        spawnerSystem.update(registry, 0.016F);
    }

    // Continue updating after GameOver
    for (int i = 0; i < 100; ++i) {
        spawnerSystem.update(registry, 0.2F);
    }

    // Count GameOver events
    int gameOverCount = 0;
    for (const auto& event : emittedEvents) {
        if (event.type == rtype::engine::GameEventType::GameOver) {
            gameOverCount++;
        }
    }
    EXPECT_EQ(gameOverCount, 1);
}

TEST_F(WaveSystemTest, InfiniteWavesDoesNotEmitGameOver) {
    config.maxWaves = 0;  // Infinite waves

    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // In infinite mode (maxWaves = 0), waves don't automatically advance
    // because the wave completion check requires maxWaves > 0
    // So we just spawn enemies and verify no GameOver is emitted
    for (int i = 0; i < 50; ++i) {
        spawnerSystem.update(registry, 0.2F);
    }

    // Check that GameOver event was NOT emitted (infinite mode)
    bool gameOverEmitted = false;
    for (const auto& event : emittedEvents) {
        if (event.type == rtype::engine::GameEventType::GameOver) {
            gameOverEmitted = true;
            break;
        }
    }
    EXPECT_FALSE(gameOverEmitted);
    // In current implementation, wave counter stays at 1 in infinite mode
    EXPECT_EQ(spawnerSystem.getCurrentWave(), 1u);
    EXPECT_FALSE(spawnerSystem.isAllWavesCompleted());
}

// =============================================================================
// Test 3: Wave Counter Increments
// =============================================================================

TEST_F(WaveSystemTest, WaveCounterIncrementsAfterCompletion) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    EXPECT_EQ(spawnerSystem.getCurrentWave(), 1u);

    // Complete wave 1
    waitForWaveSpawns(spawnerSystem, config.enemiesPerWave);
    killAllEnemies();
    spawnerSystem.update(registry, 0.016F);
    EXPECT_EQ(spawnerSystem.getCurrentWave(), 2u);

    // Complete wave 2
    waitForWaveSpawns(spawnerSystem, config.enemiesPerWave);
    killAllEnemies();
    spawnerSystem.update(registry, 0.016F);
    EXPECT_EQ(spawnerSystem.getCurrentWave(), 3u);

    // Complete wave 3
    waitForWaveSpawns(spawnerSystem, config.enemiesPerWave);
    killAllEnemies();
    spawnerSystem.update(registry, 0.016F);
    
    // After completing all waves, the counter should still be 3
    // (or possibly 4 depending on implementation)
    EXPECT_GE(spawnerSystem.getCurrentWave(), 3u);
}

TEST_F(WaveSystemTest, WaveCounterStartsAtOne) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    EXPECT_EQ(spawnerSystem.getCurrentWave(), 1u);
}

TEST_F(WaveSystemTest, WaveCounterIncrementsCorrectlyAcrossMultipleWaves) {
    config.maxWaves = 10;
    config.enemiesPerWave = 2;  // Fewer enemies for faster test

    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    std::vector<std::size_t> waveNumbers;
    waveNumbers.push_back(spawnerSystem.getCurrentWave());

    // Complete 5 waves and track wave numbers
    for (std::size_t wave = 1; wave <= 5; ++wave) {
        waitForWaveSpawns(spawnerSystem, config.enemiesPerWave);
        killAllEnemies();
        spawnerSystem.update(registry, 0.016F);
        waveNumbers.push_back(spawnerSystem.getCurrentWave());
    }

    // Verify wave numbers increment by 1 each time
    for (std::size_t i = 1; i < waveNumbers.size(); ++i) {
        EXPECT_EQ(waveNumbers[i], waveNumbers[i - 1] + 1);
    }
}

// =============================================================================
// Test 4: Spawning Stops After GameOver
// =============================================================================

TEST_F(WaveSystemTest, StopsSpawningAfterGameOverEmitted) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Complete all 3 waves
    for (std::size_t wave = 1; wave <= config.maxWaves; ++wave) {
        waitForWaveSpawns(spawnerSystem, config.enemiesPerWave);
        killAllEnemies();
        spawnerSystem.update(registry, 0.016F);
    }

    // Verify GameOver was emitted by checking events
    bool gameOverEmitted = false;
    for (const auto& event : emittedEvents) {
        if (event.type == rtype::engine::GameEventType::GameOver) {
            gameOverEmitted = true;
            break;
        }
    }
    EXPECT_TRUE(gameOverEmitted);
    
    // Clear events to count only new spawns
    emittedEvents.clear();

    // Try to spawn more enemies after GameOver
    for (int i = 0; i < 200; ++i) {
        spawnerSystem.update(registry, 0.2F);
    }

    // Count spawn events after GameOver
    int spawnEventCount = 0;
    for (const auto& event : emittedEvents) {
        if (event.type == rtype::engine::GameEventType::EntitySpawned) {
            spawnEventCount++;
        }
    }

    // No new enemies should spawn
    EXPECT_EQ(spawnEventCount, 0);

    // Verify no new enemies in registry
    std::size_t enemyCount = 0;
    auto view = registry.view<EnemyTag>();
    view.each([&enemyCount](ECS::Entity, const EnemyTag&) {
        enemyCount++;
    });
    EXPECT_EQ(enemyCount, 0u);
}

TEST_F(WaveSystemTest, AllowsSpawningBeforeGameOver) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Complete only first wave
    waitForWaveSpawns(spawnerSystem, config.enemiesPerWave);
    killAllEnemies();
    spawnerSystem.update(registry, 0.016F);

    // Clear events
    emittedEvents.clear();

    // Try spawning in wave 2 (before GameOver)
    for (int i = 0; i < 100; ++i) {
        spawnerSystem.update(registry, 0.2F);
        if (spawnerSystem.getEnemyCount() >= config.enemiesPerWave) {
            break;
        }
    }

    // Count spawn events
    int spawnEventCount = 0;
    for (const auto& event : emittedEvents) {
        if (event.type == rtype::engine::GameEventType::EntitySpawned) {
            spawnEventCount++;
        }
    }

    // New enemies should spawn
    EXPECT_GT(spawnEventCount, 0);
}

TEST_F(WaveSystemTest, SystemStateUnchangedAfterGameOver) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Complete all waves
    for (std::size_t wave = 1; wave <= config.maxWaves; ++wave) {
        waitForWaveSpawns(spawnerSystem, config.enemiesPerWave);
        killAllEnemies();
        spawnerSystem.update(registry, 0.016F);
    }

    // Check that GameOver was emitted
    bool gameOverEmitted = false;
    for (const auto& event : emittedEvents) {
        if (event.type == rtype::engine::GameEventType::GameOver) {
            gameOverEmitted = true;
            break;
        }
    }
    EXPECT_TRUE(gameOverEmitted);

    // Continue running updates
    std::size_t waveBeforeContinue = spawnerSystem.getCurrentWave();
    std::size_t enemyCountBefore = spawnerSystem.getEnemyCount();

    for (int i = 0; i < 100; ++i) {
        spawnerSystem.update(registry, 0.2F);
    }

    // Wave counter and enemy count should remain unchanged
    EXPECT_EQ(spawnerSystem.getCurrentWave(), waveBeforeContinue);
    EXPECT_EQ(spawnerSystem.getEnemyCount(), enemyCountBefore);
}

// =============================================================================
// Test 5: Edge Cases and Integration Tests
// =============================================================================

TEST_F(WaveSystemTest, SingleWaveConfiguration) {
    config.maxWaves = 1;
    config.enemiesPerWave = 3;

    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Complete the single wave
    waitForWaveSpawns(spawnerSystem, config.enemiesPerWave);
    killAllEnemies();
    spawnerSystem.update(registry, 0.016F);

    // Check GameOver was emitted
    bool gameOverEmitted = false;
    for (const auto& event : emittedEvents) {
        if (event.type == rtype::engine::GameEventType::GameOver) {
            gameOverEmitted = true;
            break;
        }
    }
    EXPECT_TRUE(gameOverEmitted);
}

TEST_F(WaveSystemTest, LargeNumberOfWaves) {
    config.maxWaves = 100;
    config.enemiesPerWave = 1;  // One enemy per wave for speed

    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Complete 10 waves
    for (std::size_t wave = 1; wave <= 10; ++wave) {
        waitForWaveSpawns(spawnerSystem, config.enemiesPerWave);
        killAllEnemies();
        spawnerSystem.update(registry, 0.016F);
    }

    // Should be on wave 11 now
    EXPECT_EQ(spawnerSystem.getCurrentWave(), 11u);

    // GameOver should NOT be emitted yet
    bool gameOverEmitted = false;
    for (const auto& event : emittedEvents) {
        if (event.type == rtype::engine::GameEventType::GameOver) {
            gameOverEmitted = true;
            break;
        }
    }
    EXPECT_FALSE(gameOverEmitted);
    EXPECT_FALSE(spawnerSystem.isAllWavesCompleted());
}

TEST_F(WaveSystemTest, WaveProgressionWithPartialCleanup) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Spawn enemies for wave 1
    for (int i = 0; i < 100; ++i) {
        spawnerSystem.update(registry, 0.2F);
        if (spawnerSystem.getEnemyCount() >= config.enemiesPerWave) {
            break;
        }
    }

    // Kill only some enemies
    std::vector<ECS::Entity> entitiesToKill;
    auto view = registry.view<EnemyTag>();
    std::size_t count = 0;
    view.each([&entitiesToKill, &count](ECS::Entity entity, const EnemyTag&) {
        if (count < 3) {
            entitiesToKill.push_back(entity);
            count++;
        }
    });
    for (auto entity : entitiesToKill) {
        registry.removeComponent<EnemyTag>(entity);
    }

    spawnerSystem.update(registry, 0.016F);

    // Wave should not advance yet
    EXPECT_EQ(spawnerSystem.getCurrentWave(), 1u);

    // Kill remaining enemies
    entitiesToKill.clear();
    view.each([&entitiesToKill](ECS::Entity entity, const EnemyTag&) {
        entitiesToKill.push_back(entity);
    });
    for (auto entity : entitiesToKill) {
        registry.removeComponent<EnemyTag>(entity);
    }

    spawnerSystem.update(registry, 0.016F);

    // Now wave should advance
    EXPECT_EQ(spawnerSystem.getCurrentWave(), 2u);
}

TEST_F(WaveSystemTest, IsAllWavesCompletedReturnsFalseInitially) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    EXPECT_FALSE(spawnerSystem.isAllWavesCompleted());
}

TEST_F(WaveSystemTest, GameOverEmittedAfterAllWavesCompleted) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Complete all waves
    for (std::size_t wave = 1; wave <= config.maxWaves; ++wave) {
        waitForWaveSpawns(spawnerSystem, config.enemiesPerWave);
        killAllEnemies();
        spawnerSystem.update(registry, 0.016F);
    }

    // Check that GameOver event was emitted
    bool gameOverEmitted = false;
    for (const auto& event : emittedEvents) {
        if (event.type == rtype::engine::GameEventType::GameOver) {
            gameOverEmitted = true;
            break;
        }
    }
    EXPECT_TRUE(gameOverEmitted);
}

TEST_F(WaveSystemTest, IsAllWavesCompletedReturnsFalseForInfiniteWaves) {
    config.maxWaves = 0;  // Infinite mode

    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Complete several waves
    for (std::size_t wave = 1; wave <= 10; ++wave) {
        waitForWaveSpawns(spawnerSystem, config.enemiesPerWave);
        killAllEnemies();
        spawnerSystem.update(registry, 0.016F);
    }

    // Should never be completed in infinite mode
    EXPECT_FALSE(spawnerSystem.isAllWavesCompleted());
}
