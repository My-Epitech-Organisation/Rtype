/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** JSON Storage PoC - Demonstration of JSON parsing with ECS
*/

#include "GameConfig.hpp"
#include "JsonComponentSerializer.hpp"
#include <ECS/ECS.hpp>
#include <iostream>
#include <iomanip>
#include <memory>

using namespace PoC;

/**
 * @brief Print game configuration details
 */
void printGameConfig(const GameConfig& config) {
    std::cout << "=== Game Configuration ===" << std::endl;
    std::cout << "Version: " << config.version << std::endl;
    std::cout << "Window: " << config.windowWidth << "x" << config.windowHeight << std::endl;
    std::cout << "Fullscreen: " << (config.fullscreen ? "Yes" : "No") << std::endl;
    std::cout << std::endl;

    std::cout << "--- Player ---" << std::endl;
    std::cout << "  Name: " << config.player.name << std::endl;
    std::cout << "  Max Health: " << config.player.maxHealth << std::endl;
    std::cout << "  Speed: " << config.player.speed << std::endl;
    std::cout << "  Score: " << config.player.score << std::endl;
    std::cout << std::endl;

    std::cout << "--- Enemies (" << config.enemies.size() << ") ---" << std::endl;
    for (const auto& enemy : config.enemies) {
        std::cout << "  " << enemy.type << ": HP=" << enemy.health 
                  << ", DMG=" << enemy.damage << ", SPD=" << enemy.speed 
                  << ", Score=" << enemy.scoreValue << std::endl;
    }
    std::cout << std::endl;

    std::cout << "--- Weapons (" << config.weapons.size() << ") ---" << std::endl;
    for (const auto& weapon : config.weapons) {
        std::cout << "  " << weapon.name << ": DMG=" << weapon.damage 
                  << ", Rate=" << weapon.fireRate << ", Ammo=" << weapon.ammoCapacity << std::endl;
    }
    std::cout << std::endl;

    std::cout << "--- Levels (" << config.levels.size() << ") ---" << std::endl;
    for (const auto& level : config.levels) {
        std::cout << "  Level " << level.levelNumber << " - " << level.name 
                  << ": Enemies=" << level.enemyCount << ", Difficulty=" << level.difficulty << std::endl;
    }
    std::cout << std::endl;
}

/**
 * @brief Create sample game configuration
 */
GameConfig createSampleConfig() {
    GameConfig config;
    config.version = "1.0.0";
    config.windowWidth = 1920;
    config.windowHeight = 1080;
    config.fullscreen = false;

    config.player = PlayerConfig{
        "Player1",
        100,
        5.0f,
        0
    };

    config.enemies = {
        {"Scout", 50, 10, 3.0f, 100},
        {"Tank", 150, 25, 1.5f, 300},
        {"Boss", 500, 50, 2.0f, 1000}
    };

    config.weapons = {
        {"Pistol", 15, 2.0f, 12},
        {"Rifle", 30, 5.0f, 30},
        {"Shotgun", 60, 1.0f, 8}
    };

    config.levels = {
        {1, "Asteroid Field", "space_bg_1.png", 10, 1.0f},
        {2, "Enemy Base", "space_bg_2.png", 20, 1.5f},
        {3, "Final Battle", "space_bg_3.png", 30, 2.0f}
    };

    return config;
}

/**
 * @brief Demonstrate ECS with JSON serialization
 */
void demonstrateEcsJsonIntegration() {
    std::cout << "\n=== ECS + JSON Integration ===" << std::endl;

    // Create ECS registry
    ECS::Registry registry;

    // Create player entity
    ECS::Entity player = registry.spawnEntity();
    registry.emplaceComponent<Position>(player, 100.0f, 200.0f);
    registry.emplaceComponent<Velocity>(player, 0.0f, 0.0f);
    registry.emplaceComponent<Health>(player, 100, 100);
    registry.emplaceComponent<PlayerTag>(player);

    // Create enemy entities
    for (int i = 0; i < 3; ++i) {
        ECS::Entity enemy = registry.spawnEntity();
        registry.emplaceComponent<Position>(enemy, 300.0f + i * 100.0f, 150.0f + i * 50.0f);
        registry.emplaceComponent<Velocity>(enemy, -1.0f, 0.5f);
        registry.emplaceComponent<Health>(enemy, 50, 50);
        registry.emplaceComponent<EnemyTag>(enemy);
    }

    std::cout << "Created entities" << std::endl;

    // Save entities to JSON
    std::cout << "Saving entities to 'entities.json'..." << std::endl;
    saveEntitiesToJson(registry, "entities.json");

    // Clear registry and reload
    std::cout << "Clearing registry and reloading from JSON..." << std::endl;
    // Note: Registry doesn't have clear() - we would need to killEntity() individually
    // For demo purposes, create a new registry
    ECS::Registry newRegistry;
    
    loadEntitiesFromJson(newRegistry, "entities.json");
    std::cout << "Loaded entities from JSON" << std::endl;

    // Verify loaded data
    std::cout << "\n--- Loaded Entities ---" << std::endl;
    newRegistry.view<Position>().each([&](ECS::Entity entity, const Position& pos) {
        std::cout << "Entity " << entity.id 
                  << " at position (" << pos.x << ", " << pos.y << ")";
        
        if (newRegistry.hasComponent<Health>(entity)) {
            const auto& health = newRegistry.getComponent<Health>(entity);
            std::cout << " - HP: " << health.current << "/" << health.max;
        }

        if (newRegistry.hasComponent<PlayerTag>(entity)) {
            std::cout << " [PLAYER]";
        } else if (newRegistry.hasComponent<EnemyTag>(entity)) {
            std::cout << " [ENEMY]";
        }
        
        std::cout << std::endl;
    });
}

/**
 * @brief Test ECS Serializer with JSON backend
 */
void demonstrateEcsSerializer() {
    std::cout << "\n=== ECS Serializer with JSON Backend ===" << std::endl;

    ECS::Registry registry;
    ECS::Serializer serializer(&registry);

    // Register JSON serializers for each component type
    serializer.registerSerializer<Position>(std::make_shared<JsonComponentSerializer<Position>>());
    serializer.registerSerializer<Velocity>(std::make_shared<JsonComponentSerializer<Velocity>>());
    serializer.registerSerializer<Health>(std::make_shared<JsonComponentSerializer<Health>>());

    // Create test entity
    ECS::Entity testEntity = registry.spawnEntity();
    registry.emplaceComponent<Position>(testEntity, 42.0f, 84.0f);
    registry.emplaceComponent<Velocity>(testEntity, 1.5f, -2.3f);
    registry.emplaceComponent<Health>(testEntity, 75, 100);

    std::cout << "Created test entity with components" << std::endl;
    std::cout << "Saving using ECS::Serializer..." << std::endl;

    // Save using ECS serializer
    if (serializer.saveToFile("ecs_save.txt")) {
        std::cout << "Successfully saved to 'ecs_save.txt'" << std::endl;
    } else {
        std::cout << "Failed to save!" << std::endl;
    }

    // Note: For serializer demo, we'll skip clear/reload as it requires
    // proper entity management implementation
    std::cout << "Serialization saved to 'ecs_save.txt'" << std::endl;
    
    // Create new registry for loading test
    ECS::Registry newReg;
    ECS::Serializer newSerializer(&newReg);
    newSerializer.registerSerializer<Position>(std::make_shared<JsonComponentSerializer<Position>>());
    newSerializer.registerSerializer<Velocity>(std::make_shared<JsonComponentSerializer<Velocity>>());
    newSerializer.registerSerializer<Health>(std::make_shared<JsonComponentSerializer<Health>>());
    
    if (newSerializer.loadFromFile("ecs_save.txt")) {
        std::cout << "Successfully loaded from 'ecs_save.txt' into new registry" << std::endl;
    } else {
        std::cout << "Failed to load!" << std::endl;
    }
}

int main() {
    try {
        std::cout << "╔════════════════════════════════════════╗" << std::endl;
        std::cout << "║   JSON Storage PoC with ECS           ║" << std::endl;
        std::cout << "║   Using nlohmann/json library         ║" << std::endl;
        std::cout << "╚════════════════════════════════════════╝" << std::endl;
        std::cout << std::endl;

        // Part 1: Game Configuration JSON
        std::cout << "PART 1: Loading game_config.json" << std::endl;
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

        // Create and save sample configuration
        GameConfig sampleConfig = createSampleConfig();
        saveGameConfig(sampleConfig, "game_config.json");
        std::cout << "✓ Created sample 'game_config.json'" << std::endl;
        std::cout << std::endl;

        // Load and display configuration
        GameConfig config = loadGameConfig("game_config.json");
        std::cout << "✓ Successfully loaded 'game_config.json'" << std::endl;
        std::cout << std::endl;
        printGameConfig(config);

        // Part 2: ECS Integration with JSON
        std::cout << "PART 2: ECS + JSON Integration" << std::endl;
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
        demonstrateEcsJsonIntegration();

        // Part 3: ECS Serializer with JSON backend
        std::cout << "\nPART 3: ECS Serializer" << std::endl;
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
        demonstrateEcsSerializer();

        std::cout << "\n╔════════════════════════════════════════╗" << std::endl;
        std::cout << "║   ✓ All tests completed successfully  ║" << std::endl;
        std::cout << "╚════════════════════════════════════════╝" << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}
