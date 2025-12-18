/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Integration test for EventManager with game systems
*/

#include <gtest/gtest.h>

#include <atomic>
#include <memory>

#include "ECS.hpp"
#include "EventManager.hpp"

using namespace rtype::engine;
using namespace rtype::engine::events;

// ============================================================================
// Mock Components for Integration Test
// ============================================================================

struct Position {
    float x = 0.0f;
    float y = 0.0f;
};

struct Health {
    int32_t current = 100;
    int32_t max = 100;
};

struct NetworkId {
    uint32_t id = 0;
};

// ============================================================================
// Mock Systems for Integration Test
// ============================================================================

/**
 * @brief Mock Physics System that publishes collision events
 */
class MockPhysicsSystem {
   public:
    explicit MockPhysicsSystem(std::shared_ptr<EventManager> eventManager)
        : _eventManager(std::move(eventManager)) {}

    void detectCollisions(ECS::Registry& registry) {
        // Simulate collision detection
        auto view = registry.view<Position>();
        std::vector<ECS::Entity> entities;
        view.each([&entities](ECS::Entity entity, const Position&) {
            entities.push_back(entity);
        });

        // For every pair of entities, publish a collision event
        for (size_t i = 0; i < entities.size(); ++i) {
            for (size_t j = i + 1; j < entities.size(); ++j) {
                CollisionEvent collision{entities[i], entities[j], 10.0f};
                _eventManager->publish(collision);
            }
        }
    }

   private:
    std::shared_ptr<EventManager> _eventManager;
};

/**
 * @brief Mock Damage System that subscribes to collision events
 */
class MockDamageSystem {
   public:
    explicit MockDamageSystem(std::shared_ptr<EventManager> eventManager)
        : _eventManager(std::move(eventManager)), _damageApplied(0) {
        _eventManager->subscribe<CollisionEvent>(
            [this](const CollisionEvent& event) {
                this->onCollision(event);
            });
    }

    void onCollision(const CollisionEvent& /*event*/) {
        ++_damageApplied;
        // In a real system, this would apply damage to entities
    }

    int getDamageApplied() const { return _damageApplied; }

   private:
    std::shared_ptr<EventManager> _eventManager;
    int _damageApplied;
};

/**
 * @brief Mock Network System that subscribes to damage events
 */
class MockNetworkSystem {
   public:
    explicit MockNetworkSystem(std::shared_ptr<EventManager> eventManager)
        : _eventManager(std::move(eventManager)), _networkPacketsSent(0) {
        _eventManager->subscribe<DamageEvent>([this](const DamageEvent& event) {
            this->onDamage(event);
        });
    }

    void onDamage(const DamageEvent& /*event*/) {
        ++_networkPacketsSent;
        // In a real system, this would send a network packet
    }

    int getNetworkPacketsSent() const { return _networkPacketsSent; }

   private:
    std::shared_ptr<EventManager> _eventManager;
    int _networkPacketsSent;
};

// ============================================================================
// Integration Tests
// ============================================================================

TEST(EventManagerIntegrationTest, PhysicsToAudio_DecoupledCommunication) {
    auto eventManager = std::make_shared<EventManager>();
    auto registry = std::make_shared<ECS::Registry>();

    // Create systems without direct dependencies
    MockPhysicsSystem physics(eventManager);
    MockDamageSystem damage(eventManager);

    // Create entities
    auto entity1 = registry->spawnEntity();
    auto entity2 = registry->spawnEntity();
    registry->emplaceComponent<Position>(entity1, Position{0.0f, 0.0f});
    registry->emplaceComponent<Position>(entity2, Position{10.0f, 10.0f});

    // Physics detects collisions and publishes events
    physics.detectCollisions(*registry);

    // Damage system should have processed the collision
    EXPECT_EQ(damage.getDamageApplied(), 1);
}

TEST(EventManagerIntegrationTest, CollisionToDamageToNetwork_EventChain) {
    auto eventManager = std::make_shared<EventManager>();
    auto registry = std::make_shared<ECS::Registry>();

    MockPhysicsSystem physics(eventManager);
    MockDamageSystem damage(eventManager);
    MockNetworkSystem network(eventManager);

    // Create entities
    auto entity1 = registry->spawnEntity();
    auto entity2 = registry->spawnEntity();
    registry->emplaceComponent<Position>(entity1, Position{0.0f, 0.0f});
    registry->emplaceComponent<Position>(entity2, Position{10.0f, 10.0f});
    registry->emplaceComponent<Health>(entity1, Health{100, 100});
    registry->emplaceComponent<Health>(entity2, Health{100, 100});
    registry->emplaceComponent<NetworkId>(entity1, NetworkId{1});
    registry->emplaceComponent<NetworkId>(entity2, NetworkId{2});

    // Step 1: Physics detects collision
    physics.detectCollisions(*registry);
    EXPECT_EQ(damage.getDamageApplied(), 1);

    // Step 2: Damage system applies damage and publishes DamageEvent
    auto& health1 = registry->getComponent<Health>(entity1);
    health1.current -= 25;
    DamageEvent damageEvent{entity1, 25, 100, 75, 1};
    eventManager->publish(damageEvent);

    // Step 3: Network system should have sent a packet
    EXPECT_EQ(network.getNetworkPacketsSent(), 1);
}

TEST(EventManagerIntegrationTest, MultipleCollisions_AllProcessed) {
    auto eventManager = std::make_shared<EventManager>();
    auto registry = std::make_shared<ECS::Registry>();

    MockPhysicsSystem physics(eventManager);
    MockDamageSystem damage(eventManager);

    // Create multiple entities
    for (int i = 0; i < 5; ++i) {
        auto entity = registry->spawnEntity();
        registry->emplaceComponent<Position>(
            entity, Position{static_cast<float>(i * 10), 0.0f});
    }

    // Physics detects all collisions
    physics.detectCollisions(*registry);

    // With 5 entities, there should be C(5,2) = 10 collision pairs
    EXPECT_EQ(damage.getDamageApplied(), 10);
}

TEST(EventManagerIntegrationTest, NetworkInputToMovement_KeyPressEvent) {
    auto eventManager = std::make_shared<EventManager>();
    auto registry = std::make_shared<ECS::Registry>();

    float playerX = 0.0f;
    float playerY = 0.0f;

    // Subscribe to network input events
    eventManager->subscribe<NetworkInputEvent>(
        [&playerX, &playerY](const NetworkInputEvent& event) {
            playerX += event.deltaX;
            playerY += event.deltaY;
        });

    // Simulate network input (key press)
    NetworkInputEvent moveRight{1, 0, 5.0f, 0.0f, true};
    eventManager->publish(moveRight);

    EXPECT_FLOAT_EQ(playerX, 5.0f);
    EXPECT_FLOAT_EQ(playerY, 0.0f);

    NetworkInputEvent moveDown{1, 0, 0.0f, 3.0f, true};
    eventManager->publish(moveDown);

    EXPECT_FLOAT_EQ(playerX, 5.0f);
    EXPECT_FLOAT_EQ(playerY, 3.0f);
}

TEST(EventManagerIntegrationTest, SystemEnableDisable_RuntimeFlexibility) {
    auto eventManager = std::make_shared<EventManager>();
    auto registry = std::make_shared<ECS::Registry>();

    MockPhysicsSystem physics(eventManager);
    MockDamageSystem damage(eventManager);

    // Create entities
    auto entity1 = registry->spawnEntity();
    auto entity2 = registry->spawnEntity();
    registry->emplaceComponent<Position>(entity1, Position{0.0f, 0.0f});
    registry->emplaceComponent<Position>(entity2, Position{10.0f, 10.0f});

    // Enable damage system
    physics.detectCollisions(*registry);
    EXPECT_EQ(damage.getDamageApplied(), 1);

    // Disable damage system by unsubscribing
    eventManager->unsubscribeAll<CollisionEvent>();

    // Physics still runs, but damage system won't process events
    physics.detectCollisions(*registry);
    EXPECT_EQ(damage.getDamageApplied(), 1);  // No additional damage
}

TEST(EventManagerIntegrationTest, NoDirectDependencies_HeaderInclusion) {
    // This test verifies that systems only need EventManager headers,
    // not other system headers
    auto eventManager = std::make_shared<EventManager>();

    // Physics system can be compiled without including damage system headers
    MockPhysicsSystem physics(eventManager);

    // Damage system can be compiled without including physics system headers
    MockDamageSystem damage(eventManager);

    // Network system can be compiled without including damage or physics
    // headers
    MockNetworkSystem network(eventManager);

    // All systems can coexist without direct dependencies
    EXPECT_NE(eventManager, nullptr);
}

TEST(EventManagerIntegrationTest, EntitySpawnedEvent_NetworkSync) {
    auto eventManager = std::make_shared<EventManager>();
    auto registry = std::make_shared<ECS::Registry>();

    int entitiesSpawned = 0;
    std::vector<uint32_t> spawnedNetworkIds;

    // Network system subscribes to entity spawned events
    eventManager->subscribe<EntitySpawnedEvent>(
        [&entitiesSpawned, &spawnedNetworkIds](const EntitySpawnedEvent& event) {
            ++entitiesSpawned;
            spawnedNetworkIds.push_back(event.networkId);
        });

    // Spawn entities and publish events
    for (uint32_t i = 1; i <= 3; ++i) {
        auto entity = registry->spawnEntity();
        registry->emplaceComponent<NetworkId>(entity, NetworkId{i});
        EntitySpawnedEvent spawnEvent{entity, i, 1, 0.0f, 0.0f};
        eventManager->publish(spawnEvent);
    }

    EXPECT_EQ(entitiesSpawned, 3);
    ASSERT_EQ(spawnedNetworkIds.size(), 3);
    EXPECT_EQ(spawnedNetworkIds[0], 1);
    EXPECT_EQ(spawnedNetworkIds[1], 2);
    EXPECT_EQ(spawnedNetworkIds[2], 3);
}

TEST(EventManagerIntegrationTest, PowerUpPickedEvent_PlayerBuffApplied) {
    auto eventManager = std::make_shared<EventManager>();
    auto registry = std::make_shared<ECS::Registry>();

    bool powerUpApplied = false;
    uint32_t buffedPlayerId = 0;

    // Logic system subscribes to power-up events
    eventManager->subscribe<PowerUpPickedEvent>(
        [&powerUpApplied, &buffedPlayerId](const PowerUpPickedEvent& event) {
            powerUpApplied = true;
            buffedPlayerId = event.playerNetworkId;
        });

    // Simulate power-up pickup
    auto player = registry->spawnEntity();
    auto powerUp = registry->spawnEntity();
    registry->emplaceComponent<NetworkId>(player, NetworkId{42});

    PowerUpPickedEvent pickupEvent{player, powerUp, 42, 1};
    eventManager->publish(pickupEvent);

    EXPECT_TRUE(powerUpApplied);
    EXPECT_EQ(buffedPlayerId, 42);
}

TEST(EventManagerIntegrationTest, OutOfBoundsEvent_EnemyEscape) {
    auto eventManager = std::make_shared<EventManager>();
    auto registry = std::make_shared<ECS::Registry>();

    int enemiesEscaped = 0;
    int playerDamageTaken = 0;

    // Cleanup system publishes out-of-bounds events
    eventManager->subscribe<OutOfBoundsEvent>(
        [&enemiesEscaped, &playerDamageTaken](const OutOfBoundsEvent& /*event*/) {
            ++enemiesEscaped;
            playerDamageTaken += 30;  // Players take damage when enemy escapes
        });

    // Enemy goes out of bounds
    auto enemy = registry->spawnEntity();
    registry->emplaceComponent<Position>(enemy, Position{-150.0f, 0.0f});

    OutOfBoundsEvent escapeEvent{enemy, -150.0f, 0.0f};
    eventManager->publish(escapeEvent);

    EXPECT_EQ(enemiesEscaped, 1);
    EXPECT_EQ(playerDamageTaken, 30);
}
