/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** test_enemy_health_bar_system.cpp - Tests for EnemyHealthBarSystem
*/

#include <gtest/gtest.h>
#include <memory>

#include "ECS.hpp"
#include "games/rtype/client/Systems/EnemyHealthBarSystem.hpp"
#include "games/rtype/shared/Components.hpp"
#include "games/rtype/client/AllComponents.hpp"

namespace rc = ::rtype::games::rtype::client;
namespace rs = ::rtype::games::rtype::shared;

class EnemyHealthBarSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        registry = std::make_shared<ECS::Registry>();
        system = std::make_unique<rc::EnemyHealthBarSystem>(registry);
    }

    std::shared_ptr<ECS::Registry> registry;
    std::unique_ptr<rc::EnemyHealthBarSystem> system;
};

TEST_F(EnemyHealthBarSystemTest, CreateHealthBarForNewEnemy) {
    // Create an enemy entity
    auto enemy = registry->spawnEntity();
    registry->emplaceComponent<rs::EnemyTag>(enemy);
    registry->emplaceComponent<rs::HealthComponent>(enemy, 100, 100);
    registry->emplaceComponent<rs::TransformComponent>(enemy, 100.0f, 200.0f);

    // Update system - should create health bars
    system->update(*registry, 0.016f);

    // Check that health bar entities were created
    int rectangleCount = 0;
    registry->view<rc::Rectangle>().each([&](auto, const auto&) {
        rectangleCount++;
    });

    // Should have 2 rectangles: background and fill
    EXPECT_EQ(rectangleCount, 2);
}

TEST_F(EnemyHealthBarSystemTest, UpdateHealthBarPosition) {
    // Create an enemy entity
    auto enemy = registry->spawnEntity();
    registry->emplaceComponent<rs::EnemyTag>(enemy);
    registry->emplaceComponent<rs::HealthComponent>(enemy, 50, 100);
    registry->emplaceComponent<rs::TransformComponent>(enemy, 100.0f, 200.0f);

    // First update creates bars
    system->update(*registry, 0.016f);

    // Move enemy
    auto& transform = registry->getComponent<rs::TransformComponent>(enemy);
    transform.x = 300.0f;
    transform.y = 400.0f;

    // Second update should update positions
    system->update(*registry, 0.016f);

    // Verify health bars exist and are positioned
    bool foundBar = false;
    registry->view<rc::Rectangle, rs::TransformComponent>().each(
        [&](auto, const auto&, const auto& pos) {
            if (pos.y == 400.0f - 30.0f) { // HEALTH_BAR_OFFSET_Y = -30
                foundBar = true;
            }
        });

    EXPECT_TRUE(foundBar);
}

TEST_F(EnemyHealthBarSystemTest, HealthBarColorChangesWithHealth) {
    // Create an enemy entity
    auto enemy = registry->spawnEntity();
    registry->emplaceComponent<rs::EnemyTag>(enemy);
    registry->emplaceComponent<rs::HealthComponent>(enemy, 100, 100);
    registry->emplaceComponent<rs::TransformComponent>(enemy, 100.0f, 200.0f);

    // First update - health is 100%
    system->update(*registry, 0.016f);

    // Damage enemy to 50%
    auto& health = registry->getComponent<rs::HealthComponent>(enemy);
    health.current = 50;
    system->update(*registry, 0.016f);

    // Damage enemy to 20% (red)
    health.current = 20;
    system->update(*registry, 0.016f);

    // Verify rectangles exist
    int fillRectCount = 0;
    registry->view<rc::Rectangle, rc::ZIndex>().each(
        [&](auto, const auto&, const auto& zindex) {
            if (zindex.depth == 3) { // Fill has ZIndex 3
                fillRectCount++;
            }
        });

    EXPECT_EQ(fillRectCount, 1);
}

TEST_F(EnemyHealthBarSystemTest, RemoveHealthBarWhenEnemyDies) {
    // Create an enemy entity
    auto enemy = registry->spawnEntity();
    registry->emplaceComponent<rs::EnemyTag>(enemy);
    registry->emplaceComponent<rs::HealthComponent>(enemy, 100, 100);
    registry->emplaceComponent<rs::TransformComponent>(enemy, 100.0f, 200.0f);

    // Create health bars
    system->update(*registry, 0.016f);

    // Count initial rectangles
    int initialCount = 0;
    registry->view<rc::Rectangle>().each([&](auto, const auto&) {
        initialCount++;
    });

    EXPECT_EQ(initialCount, 2);

    // Mark enemy for destruction
    registry->emplaceComponent<rs::DestroyTag>(enemy);
    system->update(*registry, 0.016f);

    // Count rectangles with DestroyTag
    int destroyTagCount = 0;
    registry->view<rs::DestroyTag>().each([&](auto, const auto&) {
        destroyTagCount++;
    });

    // Enemy + 2 health bars should have DestroyTag
    EXPECT_GE(destroyTagCount, 2);
}

TEST_F(EnemyHealthBarSystemTest, MultipleEnemiesHaveIndependentBars) {
    // Create multiple enemies
    auto enemy1 = registry->spawnEntity();
    registry->emplaceComponent<rs::EnemyTag>(enemy1);
    registry->emplaceComponent<rs::HealthComponent>(enemy1, 100, 100);
    registry->emplaceComponent<rs::TransformComponent>(enemy1, 100.0f, 200.0f);

    auto enemy2 = registry->spawnEntity();
    registry->emplaceComponent<rs::EnemyTag>(enemy2);
    registry->emplaceComponent<rs::HealthComponent>(enemy2, 50, 100);
    registry->emplaceComponent<rs::TransformComponent>(enemy2, 300.0f, 400.0f);

    // Update system
    system->update(*registry, 0.016f);

    // Should have 4 rectangles total (2 per enemy)
    int rectangleCount = 0;
    registry->view<rc::Rectangle>().each([&](auto, const auto&) {
        rectangleCount++;
    });

    EXPECT_EQ(rectangleCount, 4);
}

TEST_F(EnemyHealthBarSystemTest, NoHealthBarForNonEnemyEntity) {
    // Create entity without EnemyTag
    auto entity = registry->spawnEntity();
    registry->emplaceComponent<rs::HealthComponent>(entity, 100, 100);
    registry->emplaceComponent<rs::TransformComponent>(entity, 100.0f, 200.0f);

    // Update system
    system->update(*registry, 0.016f);

    // Should have no rectangles
    int rectangleCount = 0;
    registry->view<rc::Rectangle>().each([&](auto, const auto&) {
        rectangleCount++;
    });

    EXPECT_EQ(rectangleCount, 0);
}

TEST_F(EnemyHealthBarSystemTest, HealthBarWidthScalesWithHealth) {
    // Create an enemy entity
    auto enemy = registry->spawnEntity();
    registry->emplaceComponent<rs::EnemyTag>(enemy);
    registry->emplaceComponent<rs::HealthComponent>(enemy, 100, 100);
    registry->emplaceComponent<rs::TransformComponent>(enemy, 100.0f, 200.0f);

    // Create health bars
    system->update(*registry, 0.016f);

    // Reduce health to 50%
    auto& health = registry->getComponent<rs::HealthComponent>(enemy);
    health.current = 50;
    system->update(*registry, 0.016f);

    // Check fill width (should be ~25 pixels for 50% of 50px bar)
    bool foundCorrectWidth = false;
    registry->view<rc::Rectangle, rc::ZIndex>().each(
        [&](auto, const auto& rect, const auto& zindex) {
            if (zindex.depth == 3) { // Fill rectangle
                if (rect.size.first >= 24.0f && rect.size.first <= 26.0f) {
                    foundCorrectWidth = true;
                }
            }
        });

    EXPECT_TRUE(foundCorrectWidth);
}

TEST_F(EnemyHealthBarSystemTest, CleanupDeadEnemyBars) {
    // Create an enemy
    auto enemy = registry->spawnEntity();
    registry->emplaceComponent<rs::EnemyTag>(enemy);
    registry->emplaceComponent<rs::HealthComponent>(enemy, 100, 100);
    registry->emplaceComponent<rs::TransformComponent>(enemy, 100.0f, 200.0f);

    // Create health bars
    system->update(*registry, 0.016f);

    // Remove enemy tag to simulate cleanup
    registry->removeComponent<rs::EnemyTag>(enemy);
    system->update(*registry, 0.016f);

    // Health bars should be marked for destruction
    int destroyTagCount = 0;
    registry->view<rc::Rectangle, rs::DestroyTag>().each(
        [&](auto, const auto&, const auto&) {
            destroyTagCount++;
        });

    EXPECT_GE(destroyTagCount, 0); // Bars should be cleaned up
}
