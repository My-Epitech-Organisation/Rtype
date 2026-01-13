/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_forcepod_launch_system - Tests for Force Pod launch and recall mechanics
*/

#include <gtest/gtest.h>

#include <cmath>
#include <memory>

#include "games/rtype/server/Systems/ForcePod/ForcePodLaunchSystem.hpp"
#include "games/rtype/shared/Components/ForcePodComponent.hpp"
#include "games/rtype/shared/Components/NetworkIdComponent.hpp"
#include "games/rtype/shared/Components/Tags.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"
#include "games/rtype/shared/Components/VelocityComponent.hpp"

using namespace rtype::games::rtype::server;
using namespace rtype::games::rtype::shared;

class ForcePodLaunchSystemTest : public ::testing::Test {
   protected:
    std::unique_ptr<ECS::Registry> registry;
    std::unique_ptr<ForcePodLaunchSystem> system;

    void SetUp() override {
        registry = std::make_unique<ECS::Registry>();
        system = std::make_unique<ForcePodLaunchSystem>();
    }

    void TearDown() override {
        system.reset();
        registry.reset();
    }

    ECS::Entity createPlayer(std::uint32_t networkId, float x, float y) {
        ECS::Entity player = registry->spawnEntity();
        registry->emplaceComponent<PlayerTag>(player);
        registry->emplaceComponent<NetworkIdComponent>(player, networkId);
        registry->emplaceComponent<TransformComponent>(player, x, y, 0.0F);
        return player;
    }

    ECS::Entity createForcePod(std::uint32_t ownerNetworkId, ForcePodState state,
                                float x, float y) {
        ECS::Entity forcePod = registry->spawnEntity();
        registry->emplaceComponent<ForcePodTag>(forcePod);
        registry->emplaceComponent<ForcePodComponent>(forcePod, state, 50.0F,
                                                      20.0F, ownerNetworkId);
        registry->emplaceComponent<TransformComponent>(forcePod, x, y, 0.0F);
        return forcePod;
    }
};

TEST_F(ForcePodLaunchSystemTest, GetNameReturnsCorrectName) {
    EXPECT_EQ(system->getName(), "ForcePodLaunchSystem");
}

TEST_F(ForcePodLaunchSystemTest, SetForcePodForPlayer) {
    ECS::Entity player = createPlayer(1000, 100.0F, 200.0F);
    ECS::Entity forcePod = createForcePod(1000, ForcePodState::Attached, 150.0F, 220.0F);

    system->setForcePodForPlayer(1000, forcePod);

    // Should not crash and handle input properly
    system->handleForcePodInput(*registry, 1000);
}

TEST_F(ForcePodLaunchSystemTest, RemoveForcePodForPlayer) {
    ECS::Entity forcePod = createForcePod(1000, ForcePodState::Attached, 150.0F, 220.0F);
    system->setForcePodForPlayer(1000, forcePod);
    system->removeForcePodForPlayer(1000);

    // Should not crash after removal
    system->handleForcePodInput(*registry, 1000);
}

TEST_F(ForcePodLaunchSystemTest, HandleForcePodInputUnknownPlayer) {
    // Should not crash with unknown player
    system->handleForcePodInput(*registry, 9999);
}

TEST_F(ForcePodLaunchSystemTest, HandleForcePodInputDeadEntity) {
    ECS::Entity forcePod = createForcePod(1000, ForcePodState::Attached, 150.0F, 220.0F);
    system->setForcePodForPlayer(1000, forcePod);

    // Kill the entity
    registry->killEntity(forcePod);

    // Should not crash with dead entity
    system->handleForcePodInput(*registry, 1000);
}

TEST_F(ForcePodLaunchSystemTest, LaunchAttachedPod) {
    ECS::Entity player = createPlayer(1000, 100.0F, 200.0F);
    ECS::Entity forcePod = createForcePod(1000, ForcePodState::Attached, 150.0F, 220.0F);
    system->setForcePodForPlayer(1000, forcePod);

    system->handleForcePodInput(*registry, 1000);

    const auto& forcePodComp = registry->getComponent<ForcePodComponent>(forcePod);
    EXPECT_EQ(forcePodComp.state, ForcePodState::Detached);

    // Should have velocity component added
    ASSERT_TRUE(registry->hasComponent<VelocityComponent>(forcePod));
    const auto& vel = registry->getComponent<VelocityComponent>(forcePod);
    EXPECT_GT(vel.vx, 0.0F);  // Launch speed is positive
}

TEST_F(ForcePodLaunchSystemTest, LaunchPodWithExistingVelocity) {
    ECS::Entity player = createPlayer(1000, 100.0F, 200.0F);
    ECS::Entity forcePod = createForcePod(1000, ForcePodState::Attached, 150.0F, 220.0F);
    registry->emplaceComponent<VelocityComponent>(forcePod, 50.0F, 50.0F);
    system->setForcePodForPlayer(1000, forcePod);

    system->handleForcePodInput(*registry, 1000);

    const auto& vel = registry->getComponent<VelocityComponent>(forcePod);
    EXPECT_EQ(vel.vy, 0.0F);  // Y velocity should be reset
}

TEST_F(ForcePodLaunchSystemTest, RecallDetachedPod) {
    ECS::Entity player = createPlayer(1000, 100.0F, 200.0F);
    ECS::Entity forcePod = createForcePod(1000, ForcePodState::Detached, 500.0F, 200.0F);
    registry->emplaceComponent<VelocityComponent>(forcePod, 0.0F, 0.0F);
    system->setForcePodForPlayer(1000, forcePod);

    system->handleForcePodInput(*registry, 1000);

    const auto& forcePodComp = registry->getComponent<ForcePodComponent>(forcePod);
    EXPECT_EQ(forcePodComp.state, ForcePodState::Returning);
}

TEST_F(ForcePodLaunchSystemTest, UpdateDetachedPhysicsDecelerates) {
    ECS::Entity forcePod = createForcePod(1000, ForcePodState::Detached, 500.0F, 200.0F);
    registry->emplaceComponent<VelocityComponent>(forcePod, 100.0F, 50.0F);

    system->update(*registry, 0.1F);

    const auto& vel = registry->getComponent<VelocityComponent>(forcePod);
    EXPECT_LT(vel.vx, 100.0F);  // Should have decelerated
    EXPECT_LT(vel.vy, 50.0F);
}

TEST_F(ForcePodLaunchSystemTest, UpdateDetachedPhysicsNegativeVelocity) {
    ECS::Entity forcePod = createForcePod(1000, ForcePodState::Detached, 500.0F, 200.0F);
    registry->emplaceComponent<VelocityComponent>(forcePod, -100.0F, -50.0F);

    system->update(*registry, 0.1F);

    const auto& vel = registry->getComponent<VelocityComponent>(forcePod);
    EXPECT_GT(vel.vx, -100.0F);  // Should have moved toward zero
    EXPECT_GT(vel.vy, -50.0F);
}

TEST_F(ForcePodLaunchSystemTest, ReturningPodMovesTowardPlayer) {
    ECS::Entity player = createPlayer(1000, 100.0F, 200.0F);
    ECS::Entity forcePod = createForcePod(1000, ForcePodState::Returning, 500.0F, 200.0F);
    registry->emplaceComponent<VelocityComponent>(forcePod, 0.0F, 0.0F);

    system->update(*registry, 0.016F);

    const auto& vel = registry->getComponent<VelocityComponent>(forcePod);
    EXPECT_LT(vel.vx, 0.0F);  // Should move left toward player
}

TEST_F(ForcePodLaunchSystemTest, ReturningPodWithNoPlayer) {
    ECS::Entity forcePod = createForcePod(9999, ForcePodState::Returning, 500.0F, 200.0F);
    registry->emplaceComponent<VelocityComponent>(forcePod, 0.0F, 0.0F);

    // Should not crash
    system->update(*registry, 0.016F);

    const auto& vel = registry->getComponent<VelocityComponent>(forcePod);
    EXPECT_FLOAT_EQ(vel.vx, 0.0F);  // No movement without player
}

TEST_F(ForcePodLaunchSystemTest, ReturningPodVeryCloseToPlayer) {
    ECS::Entity player = createPlayer(1000, 100.0F, 200.0F);
    ECS::Entity forcePod = createForcePod(1000, ForcePodState::Returning, 100.05F, 200.0F);
    registry->emplaceComponent<VelocityComponent>(forcePod, 50.0F, 50.0F);

    system->update(*registry, 0.016F);

    const auto& vel = registry->getComponent<VelocityComponent>(forcePod);
    EXPECT_FLOAT_EQ(vel.vx, 0.0F);  // Should stop when very close
    EXPECT_FLOAT_EQ(vel.vy, 0.0F);
}

TEST_F(ForcePodLaunchSystemTest, ReattachmentWhenClose) {
    ECS::Entity player = createPlayer(1000, 100.0F, 200.0F);
    ECS::Entity forcePod = createForcePod(1000, ForcePodState::Returning, 120.0F, 210.0F);
    registry->emplaceComponent<VelocityComponent>(forcePod, 50.0F, 50.0F);

    system->update(*registry, 0.016F);

    const auto& forcePodComp = registry->getComponent<ForcePodComponent>(forcePod);
    EXPECT_EQ(forcePodComp.state, ForcePodState::Attached);

    const auto& vel = registry->getComponent<VelocityComponent>(forcePod);
    EXPECT_FLOAT_EQ(vel.vx, 0.0F);
    EXPECT_FLOAT_EQ(vel.vy, 0.0F);
}

TEST_F(ForcePodLaunchSystemTest, AutoRecallWhenTooFar) {
    ECS::Entity player = createPlayer(1000, 100.0F, 200.0F);
    ECS::Entity forcePod = createForcePod(1000, ForcePodState::Detached, 1000.0F, 200.0F);
    registry->emplaceComponent<VelocityComponent>(forcePod, 0.0F, 0.0F);

    system->update(*registry, 0.016F);

    const auto& forcePodComp = registry->getComponent<ForcePodComponent>(forcePod);
    EXPECT_EQ(forcePodComp.state, ForcePodState::Returning);
}

TEST_F(ForcePodLaunchSystemTest, AttachedPodDoesNotAutoRecall) {
    ECS::Entity player = createPlayer(1000, 100.0F, 200.0F);
    ECS::Entity forcePod = createForcePod(1000, ForcePodState::Attached, 1000.0F, 200.0F);

    system->update(*registry, 0.016F);

    const auto& forcePodComp = registry->getComponent<ForcePodComponent>(forcePod);
    EXPECT_EQ(forcePodComp.state, ForcePodState::Attached);
}

TEST_F(ForcePodLaunchSystemTest, UpdateWithNoEntities) {
    system->update(*registry, 0.016F);
}

TEST_F(ForcePodLaunchSystemTest, HandleInputWithNoPlayerFound) {
    ECS::Entity forcePod = createForcePod(1000, ForcePodState::Attached, 150.0F, 220.0F);
    system->setForcePodForPlayer(1000, forcePod);

    // No player with networkId 1000 exists
    system->handleForcePodInput(*registry, 1000);

    // State should remain unchanged
    const auto& forcePodComp = registry->getComponent<ForcePodComponent>(forcePod);
    EXPECT_EQ(forcePodComp.state, ForcePodState::Attached);
}

TEST_F(ForcePodLaunchSystemTest, DetachedPhysicsWithZeroVelocity) {
    ECS::Entity forcePod = createForcePod(1000, ForcePodState::Detached, 500.0F, 200.0F);
    registry->emplaceComponent<VelocityComponent>(forcePod, 0.0F, 0.0F);

    system->update(*registry, 0.1F);

    const auto& vel = registry->getComponent<VelocityComponent>(forcePod);
    EXPECT_FLOAT_EQ(vel.vx, 0.0F);
    EXPECT_FLOAT_EQ(vel.vy, 0.0F);
}

TEST_F(ForcePodLaunchSystemTest, AttachedPodNoPhysicsUpdate) {
    ECS::Entity forcePod = createForcePod(1000, ForcePodState::Attached, 500.0F, 200.0F);
    registry->emplaceComponent<VelocityComponent>(forcePod, 100.0F, 50.0F);

    system->update(*registry, 0.1F);

    // Velocity should remain unchanged for attached pods
    const auto& vel = registry->getComponent<VelocityComponent>(forcePod);
    EXPECT_FLOAT_EQ(vel.vx, 100.0F);
    EXPECT_FLOAT_EQ(vel.vy, 50.0F);
}

TEST_F(ForcePodLaunchSystemTest, MultipleForcePods) {
    ECS::Entity player1 = createPlayer(1000, 100.0F, 200.0F);
    ECS::Entity player2 = createPlayer(2000, 500.0F, 300.0F);

    ECS::Entity forcePod1 = createForcePod(1000, ForcePodState::Detached, 300.0F, 200.0F);
    ECS::Entity forcePod2 = createForcePod(2000, ForcePodState::Detached, 800.0F, 300.0F);

    registry->emplaceComponent<VelocityComponent>(forcePod1, 50.0F, 0.0F);
    registry->emplaceComponent<VelocityComponent>(forcePod2, 50.0F, 0.0F);

    system->setForcePodForPlayer(1000, forcePod1);
    system->setForcePodForPlayer(2000, forcePod2);

    system->update(*registry, 0.1F);

    // Both pods should have decelerated
    const auto& vel1 = registry->getComponent<VelocityComponent>(forcePod1);
    const auto& vel2 = registry->getComponent<VelocityComponent>(forcePod2);

    EXPECT_LT(vel1.vx, 50.0F);
    EXPECT_LT(vel2.vx, 50.0F);
}

TEST_F(ForcePodLaunchSystemTest, ReturningPodNoOwnerPlayer) {
    // Create a force pod with an owner that doesn't exist
    ECS::Entity forcePod = createForcePod(9999, ForcePodState::Returning, 500.0F, 200.0F);
    registry->emplaceComponent<VelocityComponent>(forcePod, 0.0F, 0.0F);

    // Should not crash
    system->update(*registry, 0.016F);
}

TEST_F(ForcePodLaunchSystemTest, CheckReattachmentNoOwnerPlayer) {
    ECS::Entity forcePod = createForcePod(9999, ForcePodState::Detached, 500.0F, 200.0F);

    // Should not crash
    system->update(*registry, 0.016F);

    const auto& forcePodComp = registry->getComponent<ForcePodComponent>(forcePod);
    EXPECT_EQ(forcePodComp.state, ForcePodState::Detached);
}

TEST_F(ForcePodLaunchSystemTest, CheckReattachmentDifferentOwner) {
    ECS::Entity player = createPlayer(1000, 100.0F, 200.0F);
    ECS::Entity forcePod = createForcePod(2000, ForcePodState::Returning, 120.0F, 210.0F);
    registry->emplaceComponent<VelocityComponent>(forcePod, 0.0F, 0.0F);

    system->update(*registry, 0.016F);

    // Should NOT reattach to wrong player
    const auto& forcePodComp = registry->getComponent<ForcePodComponent>(forcePod);
    EXPECT_EQ(forcePodComp.state, ForcePodState::Returning);
}
