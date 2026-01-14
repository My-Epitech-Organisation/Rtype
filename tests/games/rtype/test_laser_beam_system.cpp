/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_laser_beam_system - Tests for LaserBeamSystem and LaserBeamComponent
*/

#include <gtest/gtest.h>

#include "ECS.hpp"
#include "games/rtype/server/Systems/LaserBeam/LaserBeamSystem.hpp"
#include "games/rtype/shared/Components.hpp"
#include "games/rtype/shared/Config/GameConfig/RTypeGameConfig.hpp"
#include "rtype/engine.hpp"

using namespace rtype::games::rtype;
using namespace rtype::game::config;

// =============================================================================
// LaserBeamComponent State Tests
// =============================================================================

TEST(LaserBeamComponentTest, CanFireOnlyWhenInactive) {
    shared::LaserBeamComponent beam;

    beam.state = shared::LaserBeamState::Inactive;
    EXPECT_TRUE(beam.canFire());

    beam.state = shared::LaserBeamState::Active;
    EXPECT_FALSE(beam.canFire());

    beam.state = shared::LaserBeamState::Cooldown;
    EXPECT_FALSE(beam.canFire());
}

TEST(LaserBeamComponentTest, IsActiveOnlyWhenActive) {
    shared::LaserBeamComponent beam;

    beam.state = shared::LaserBeamState::Inactive;
    EXPECT_FALSE(beam.isActive());

    beam.state = shared::LaserBeamState::Active;
    EXPECT_TRUE(beam.isActive());

    beam.state = shared::LaserBeamState::Cooldown;
    EXPECT_FALSE(beam.isActive());
}

TEST(LaserBeamComponentTest, IsCoolingDownOnlyWhenCooldown) {
    shared::LaserBeamComponent beam;

    beam.state = shared::LaserBeamState::Inactive;
    EXPECT_FALSE(beam.isCoolingDown());

    beam.state = shared::LaserBeamState::Active;
    EXPECT_FALSE(beam.isCoolingDown());

    beam.state = shared::LaserBeamState::Cooldown;
    EXPECT_TRUE(beam.isCoolingDown());
}

TEST(LaserBeamComponentTest, GetCooldownProgressReturnsCorrectValue) {
    shared::LaserBeamComponent beam;
    beam.cooldownDuration = 2.0F;

    // When not in cooldown, should return 1.0 (ready)
    beam.state = shared::LaserBeamState::Inactive;
    EXPECT_FLOAT_EQ(beam.getCooldownProgress(), 1.0F);

    // When in cooldown at start
    beam.state = shared::LaserBeamState::Cooldown;
    beam.cooldownTime = 2.0F;  // Full cooldown remaining
    EXPECT_FLOAT_EQ(beam.getCooldownProgress(), 0.0F);

    // When in cooldown halfway
    beam.cooldownTime = 1.0F;  // Half remaining
    EXPECT_FLOAT_EQ(beam.getCooldownProgress(), 0.5F);

    // When in cooldown almost done
    beam.cooldownTime = 0.0F;  // No remaining
    EXPECT_FLOAT_EQ(beam.getCooldownProgress(), 1.0F);
}

TEST(LaserBeamComponentTest, GetDurationProgressReturnsCorrectValue) {
    shared::LaserBeamComponent beam;
    beam.maxDuration = 3.0F;

    // When not active, should return 0.0
    beam.state = shared::LaserBeamState::Inactive;
    EXPECT_FLOAT_EQ(beam.getDurationProgress(), 0.0F);

    // When active at start
    beam.state = shared::LaserBeamState::Active;
    beam.activeTime = 0.0F;
    EXPECT_FLOAT_EQ(beam.getDurationProgress(), 0.0F);

    // When active halfway
    beam.activeTime = 1.5F;
    EXPECT_FLOAT_EQ(beam.getDurationProgress(), 0.5F);

    // When active at max
    beam.activeTime = 3.0F;
    EXPECT_FLOAT_EQ(beam.getDurationProgress(), 1.0F);
}

TEST(LaserBeamComponentTest, DefaultValuesAreCorrect) {
    shared::LaserBeamComponent beam;

    EXPECT_EQ(beam.state, shared::LaserBeamState::Inactive);
    EXPECT_EQ(beam.ownerNetworkId, 0U);
    EXPECT_FLOAT_EQ(beam.activeTime, 0.0F);
    EXPECT_FLOAT_EQ(beam.maxDuration, 3.0F);
    EXPECT_FLOAT_EQ(beam.cooldownTime, 0.0F);
    EXPECT_FLOAT_EQ(beam.cooldownDuration, 2.0F);
    EXPECT_FLOAT_EQ(beam.pulsePhase, 0.0F);
    EXPECT_FLOAT_EQ(beam.pulseSpeed, 8.0F);
}

// =============================================================================
// LaserBeamSystem Static Helper Tests
// =============================================================================

TEST(LaserBeamSystemTest, StartFiringBeamTransitionsToActive) {
    shared::LaserBeamComponent beam;
    beam.state = shared::LaserBeamState::Inactive;
    beam.activeTime = 5.0F;  // Should be reset
    beam.pulsePhase = 3.0F;  // Should be reset

    server::LaserBeamSystem::startFiringBeam(beam);

    EXPECT_EQ(beam.state, shared::LaserBeamState::Active);
    EXPECT_FLOAT_EQ(beam.activeTime, 0.0F);
    EXPECT_FLOAT_EQ(beam.pulsePhase, 0.0F);
}

TEST(LaserBeamSystemTest, StartFiringBeamOnlyIfCanFire) {
    shared::LaserBeamComponent beam;

    // Should not start if already Active
    beam.state = shared::LaserBeamState::Active;
    beam.activeTime = 1.5F;
    server::LaserBeamSystem::startFiringBeam(beam);
    EXPECT_EQ(beam.state, shared::LaserBeamState::Active);
    EXPECT_FLOAT_EQ(beam.activeTime, 1.5F);  // Unchanged

    // Should not start if in Cooldown
    beam.state = shared::LaserBeamState::Cooldown;
    beam.cooldownTime = 1.0F;
    server::LaserBeamSystem::startFiringBeam(beam);
    EXPECT_EQ(beam.state, shared::LaserBeamState::Cooldown);
}

TEST(LaserBeamSystemTest, StopFiringBeamTransitionsToCooldown) {
    shared::LaserBeamComponent beam;
    beam.state = shared::LaserBeamState::Active;
    beam.cooldownDuration = 2.0F;

    server::LaserBeamSystem::stopFiringBeam(beam);

    EXPECT_EQ(beam.state, shared::LaserBeamState::Cooldown);
    EXPECT_FLOAT_EQ(beam.cooldownTime, 2.0F);
}

TEST(LaserBeamSystemTest, StopFiringBeamOnlyIfActive) {
    shared::LaserBeamComponent beam;
    beam.cooldownDuration = 2.0F;

    // Should not stop if already Inactive
    beam.state = shared::LaserBeamState::Inactive;
    server::LaserBeamSystem::stopFiringBeam(beam);
    EXPECT_EQ(beam.state, shared::LaserBeamState::Inactive);

    // Should not stop if already in Cooldown
    beam.state = shared::LaserBeamState::Cooldown;
    beam.cooldownTime = 0.5F;  // Partial cooldown
    server::LaserBeamSystem::stopFiringBeam(beam);
    EXPECT_FLOAT_EQ(beam.cooldownTime, 0.5F);  // Unchanged
}

TEST(LaserBeamSystemTest, ForceStopBeamAlwaysTransitionsToCooldown) {
    shared::LaserBeamComponent beam;
    beam.cooldownDuration = 2.0F;

    // Force stop from Active
    beam.state = shared::LaserBeamState::Active;
    server::LaserBeamSystem::forceStopBeam(beam);
    EXPECT_EQ(beam.state, shared::LaserBeamState::Cooldown);
    EXPECT_FLOAT_EQ(beam.cooldownTime, 2.0F);

    // Force stop from Inactive (still works)
    beam.state = shared::LaserBeamState::Inactive;
    server::LaserBeamSystem::forceStopBeam(beam);
    EXPECT_EQ(beam.state, shared::LaserBeamState::Cooldown);
}

TEST(LaserBeamSystemTest, UpdateBeamStateIncrementsActiveTime) {
    shared::LaserBeamComponent beam;
    beam.state = shared::LaserBeamState::Active;
    beam.activeTime = 0.0F;
    beam.maxDuration = 3.0F;
    beam.pulsePhase = 0.0F;
    beam.pulseSpeed = 8.0F;

    bool forceStop = server::LaserBeamSystem::updateBeamState(beam, 0.5F);

    EXPECT_FALSE(forceStop);
    EXPECT_FLOAT_EQ(beam.activeTime, 0.5F);
    EXPECT_FLOAT_EQ(beam.pulsePhase, 4.0F);  // 0.5 * 8.0
}

TEST(LaserBeamSystemTest, UpdateBeamStateReturnsTrueWhenMaxDurationReached) {
    shared::LaserBeamComponent beam;
    beam.state = shared::LaserBeamState::Active;
    beam.activeTime = 2.9F;
    beam.maxDuration = 3.0F;
    beam.cooldownDuration = 2.0F;

    bool forceStop = server::LaserBeamSystem::updateBeamState(beam, 0.2F);

    EXPECT_TRUE(forceStop);
    EXPECT_EQ(beam.state, shared::LaserBeamState::Cooldown);
    EXPECT_FLOAT_EQ(beam.cooldownTime, 2.0F);
}

TEST(LaserBeamSystemTest, UpdateBeamStateDecreasesCooldownTime) {
    shared::LaserBeamComponent beam;
    beam.state = shared::LaserBeamState::Cooldown;
    beam.cooldownTime = 2.0F;
    beam.cooldownDuration = 2.0F;

    bool forceStop = server::LaserBeamSystem::updateBeamState(beam, 0.5F);

    EXPECT_FALSE(forceStop);
    EXPECT_EQ(beam.state, shared::LaserBeamState::Cooldown);
    EXPECT_FLOAT_EQ(beam.cooldownTime, 1.5F);
}

TEST(LaserBeamSystemTest, UpdateBeamStateTransitionsToInactiveAfterCooldown) {
    shared::LaserBeamComponent beam;
    beam.state = shared::LaserBeamState::Cooldown;
    beam.cooldownTime = 0.3F;

    bool forceStop = server::LaserBeamSystem::updateBeamState(beam, 0.5F);

    EXPECT_FALSE(forceStop);
    EXPECT_EQ(beam.state, shared::LaserBeamState::Inactive);
    EXPECT_FLOAT_EQ(beam.cooldownTime, 0.0F);
}

TEST(LaserBeamSystemTest, UpdateBeamStateNoOpWhenInactive) {
    shared::LaserBeamComponent beam;
    beam.state = shared::LaserBeamState::Inactive;
    beam.activeTime = 0.0F;
    beam.cooldownTime = 0.0F;

    bool forceStop = server::LaserBeamSystem::updateBeamState(beam, 1.0F);

    EXPECT_FALSE(forceStop);
    EXPECT_EQ(beam.state, shared::LaserBeamState::Inactive);
    EXPECT_FLOAT_EQ(beam.activeTime, 0.0F);
    EXPECT_FLOAT_EQ(beam.cooldownTime, 0.0F);
}

// =============================================================================
// LaserBeamSystem Fixture for Integration Tests
// =============================================================================

class LaserBeamSystemFixture : public ::testing::Test {
   protected:
    void SetUp() override {
        registry = std::make_unique<ECS::Registry>();
        LaserConfig config{};
        config.damagePerSecond = 50.0F;
        config.startupDelay = 0.5F;
        config.maxDuration = 3.0F;
        config.cooldownDuration = 2.0F;
        config.hitboxWidth = 600.0F;
        config.hitboxHeight = 50.0F;
        config.offsetX = 300.0F;

        system = std::make_unique<server::LaserBeamSystem>(
            [this](const rtype::engine::GameEvent& evt) {
                lastEvent = evt;
                eventCount++;
            },
            config);
    }

    ECS::Entity createPlayer(uint32_t networkId, float x = 100.0F,
                             float y = 100.0F) {
        auto entity = registry->spawnEntity();
        registry->emplaceComponent<shared::TransformComponent>(entity, x, y,
                                                               0.0F);
        registry->emplaceComponent<shared::PlayerTag>(entity);
        registry->emplaceComponent<shared::NetworkIdComponent>(entity,
                                                               networkId);
        return entity;
    }

    std::unique_ptr<ECS::Registry> registry;
    std::unique_ptr<server::LaserBeamSystem> system;
    rtype::engine::GameEvent lastEvent{};
    int eventCount = 0;
};

// =============================================================================
// Integration Tests
// =============================================================================

TEST_F(LaserBeamSystemFixture, HandleLaserInputStartsFiring) {
    auto player = createPlayer(1);

    system->handleLaserInput(*registry, player, 1, true);

    EXPECT_TRUE(system->hasActiveLaser(*registry, 1));
    EXPECT_GT(eventCount, 0);  // Spawn event emitted
}

TEST_F(LaserBeamSystemFixture, HandleLaserInputStopsFiring) {
    auto player = createPlayer(1);

    // Start firing first
    system->handleLaserInput(*registry, player, 1, true);
    EXPECT_TRUE(system->hasActiveLaser(*registry, 1));

    // Stop firing
    system->handleLaserInput(*registry, player, 1, false);

    // Beam should be in cooldown, not active
    EXPECT_FALSE(system->hasActiveLaser(*registry, 1));
}

TEST_F(LaserBeamSystemFixture, HasActiveLaserReturnsFalseInitially) {
    createPlayer(1);

    EXPECT_FALSE(system->hasActiveLaser(*registry, 1));
}

TEST_F(LaserBeamSystemFixture, HasActiveLaserReturnsTrueWhenFiring) {
    auto player = createPlayer(1);
    system->handleLaserInput(*registry, player, 1, true);

    EXPECT_TRUE(system->hasActiveLaser(*registry, 1));
}

TEST_F(LaserBeamSystemFixture, UpdateProcessesActiveBeams) {
    auto player = createPlayer(1);
    system->handleLaserInput(*registry, player, 1, true);

    // Update should process the beam
    system->update(*registry, 0.1F);

    // Beam should still be active
    EXPECT_TRUE(system->hasActiveLaser(*registry, 1));
}

TEST_F(LaserBeamSystemFixture, BeamReachesMaxDurationAndStops) {
    auto player = createPlayer(1);
    system->handleLaserInput(*registry, player, 1, true);

    // Simulate 3.5 seconds (max duration is 3.0s)
    for (int i = 0; i < 35; ++i) {
        system->update(*registry, 0.1F);
    }

    // Beam should no longer be active (in cooldown or destroyed)
    EXPECT_FALSE(system->hasActiveLaser(*registry, 1));
}

TEST_F(LaserBeamSystemFixture, MultiplePlayersCanHaveSeparateBeams) {
    auto player1 = createPlayer(1, 100.0F, 100.0F);
    auto player2 = createPlayer(2, 200.0F, 200.0F);

    system->handleLaserInput(*registry, player1, 1, true);
    system->handleLaserInput(*registry, player2, 2, true);

    EXPECT_TRUE(system->hasActiveLaser(*registry, 1));
    EXPECT_TRUE(system->hasActiveLaser(*registry, 2));

    // Stop only player 1's laser
    system->handleLaserInput(*registry, player1, 1, false);

    EXPECT_FALSE(system->hasActiveLaser(*registry, 1));
    EXPECT_TRUE(system->hasActiveLaser(*registry, 2));
}

TEST_F(LaserBeamSystemFixture, BeamFollowsPlayerPosition) {
    auto player = createPlayer(1, 100.0F, 100.0F);
    system->handleLaserInput(*registry, player, 1, true);

    // Move player
    auto& transform = registry->getComponent<shared::TransformComponent>(player);
    transform.x = 200.0F;
    transform.y = 150.0F;

    // Update system (should update beam position)
    system->update(*registry, 0.1F);

    // Find beam entity and check position
    auto beamView = registry->view<shared::LaserBeamTag,
                                   shared::TransformComponent>();
    bool foundBeam = false;
    beamView.each([&](ECS::Entity /*entity*/, const shared::LaserBeamTag&,
                      const shared::TransformComponent& beamTransform) {
        foundBeam = true;
        // Beam should be at player position (offset applied internally)
        EXPECT_FLOAT_EQ(beamTransform.y, 150.0F);
    });
    EXPECT_TRUE(foundBeam);
}

// =============================================================================
// Config Validation Tests
// =============================================================================

TEST(LaserConfigValidationTest, ValidConfigPassesValidation) {
    RTypeGameConfig config;
    config.gameplay.laser.damagePerSecond = 50.0F;
    config.gameplay.laser.startupDelay = 0.5F;
    config.gameplay.laser.maxDuration = 3.0F;
    config.gameplay.laser.cooldownDuration = 2.0F;
    config.gameplay.laser.hitboxWidth = 600.0F;
    config.gameplay.laser.hitboxHeight = 50.0F;

    auto errors = config.validate();

    // Check that no laser-related errors exist
    bool hasLaserError = false;
    for (const auto& error : errors) {
        if (error.section.find("laser") != std::string::npos) {
            hasLaserError = true;
            break;
        }
    }
    EXPECT_FALSE(hasLaserError);
}

TEST(LaserConfigValidationTest, NegativeDamagePerSecondFails) {
    RTypeGameConfig config;
    config.gameplay.laser.damagePerSecond = -10.0F;

    auto errors = config.validate();

    bool hasDamageError = false;
    for (const auto& error : errors) {
        if (error.key == "damagePerSecond") {
            hasDamageError = true;
            break;
        }
    }
    EXPECT_TRUE(hasDamageError);
}

TEST(LaserConfigValidationTest, ZeroDamagePerSecondFails) {
    RTypeGameConfig config;
    config.gameplay.laser.damagePerSecond = 0.0F;

    auto errors = config.validate();

    bool hasDamageError = false;
    for (const auto& error : errors) {
        if (error.key == "damagePerSecond") {
            hasDamageError = true;
            break;
        }
    }
    EXPECT_TRUE(hasDamageError);
}

TEST(LaserConfigValidationTest, NegativeStartupDelayFails) {
    RTypeGameConfig config;
    config.gameplay.laser.startupDelay = -0.5F;

    auto errors = config.validate();

    bool hasDelayError = false;
    for (const auto& error : errors) {
        if (error.key == "startupDelay") {
            hasDelayError = true;
            break;
        }
    }
    EXPECT_TRUE(hasDelayError);
}

TEST(LaserConfigValidationTest, ZeroMaxDurationFails) {
    RTypeGameConfig config;
    config.gameplay.laser.maxDuration = 0.0F;

    auto errors = config.validate();

    bool hasDurationError = false;
    for (const auto& error : errors) {
        if (error.key == "maxDuration") {
            hasDurationError = true;
            break;
        }
    }
    EXPECT_TRUE(hasDurationError);
}

TEST(LaserConfigValidationTest, NegativeHitboxWidthFails) {
    RTypeGameConfig config;
    config.gameplay.laser.hitboxWidth = -100.0F;

    auto errors = config.validate();

    bool hasWidthError = false;
    for (const auto& error : errors) {
        if (error.key == "hitboxWidth") {
            hasWidthError = true;
            break;
        }
    }
    EXPECT_TRUE(hasWidthError);
}

TEST(LaserConfigValidationTest, NegativeHitboxHeightFails) {
    RTypeGameConfig config;
    config.gameplay.laser.hitboxHeight = -50.0F;

    auto errors = config.validate();

    bool hasHeightError = false;
    for (const auto& error : errors) {
        if (error.key == "hitboxHeight") {
            hasHeightError = true;
            break;
        }
    }
    EXPECT_TRUE(hasHeightError);
}
