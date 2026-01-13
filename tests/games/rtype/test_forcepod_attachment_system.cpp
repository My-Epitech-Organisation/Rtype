/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_forcepod_attachment_system - Tests for Force Pod attachment
*/

#include <gtest/gtest.h>

#include <memory>

#include "games/rtype/server/Systems/ForcePod/ForcePodAttachmentSystem.hpp"
#include "games/rtype/shared/Components/ForcePodComponent.hpp"
#include "games/rtype/shared/Components/NetworkIdComponent.hpp"
#include "games/rtype/shared/Components/Tags.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"

using namespace rtype::games::rtype::server;
using namespace rtype::games::rtype::shared;

class ForcePodAttachmentSystemTest : public ::testing::Test {
   protected:
    std::unique_ptr<ECS::Registry> registry;
    std::unique_ptr<ForcePodAttachmentSystem> system;

    void SetUp() override {
        registry = std::make_unique<ECS::Registry>();
        system = std::make_unique<ForcePodAttachmentSystem>();
    }

    void TearDown() override {
        system.reset();
        registry.reset();
    }
};

TEST_F(ForcePodAttachmentSystemTest, GetNameReturnsCorrectName) {
    EXPECT_EQ(system->getName(), "ForcePodAttachmentSystem");
}

TEST_F(ForcePodAttachmentSystemTest, AttachedPodFollowsPlayer) {
    ECS::Entity player = registry->spawnEntity();
    registry->emplaceComponent<PlayerTag>(player);
    registry->emplaceComponent<NetworkIdComponent>(player, 1000);
    registry->emplaceComponent<TransformComponent>(player, 100.0F, 200.0F, 0.0F);

    ECS::Entity forcePod = registry->spawnEntity();
    registry->emplaceComponent<ForcePodTag>(forcePod);
    registry->emplaceComponent<ForcePodComponent>(forcePod, ForcePodState::Attached,
                                                  50.0F, 20.0F, 1000);
    registry->emplaceComponent<TransformComponent>(forcePod, 0.0F, 0.0F, 0.0F);

    system->update(*registry, 0.016F);

    const auto& podTransform = registry->getComponent<TransformComponent>(forcePod);
    EXPECT_FLOAT_EQ(podTransform.x, 150.0F);
    EXPECT_FLOAT_EQ(podTransform.y, 220.0F);
}

TEST_F(ForcePodAttachmentSystemTest, AttachedPodWithNegativeOffset) {
    ECS::Entity player = registry->spawnEntity();
    registry->emplaceComponent<PlayerTag>(player);
    registry->emplaceComponent<NetworkIdComponent>(player, 2000);
    registry->emplaceComponent<TransformComponent>(player, 300.0F, 400.0F, 0.0F);

    ECS::Entity forcePod = registry->spawnEntity();
    registry->emplaceComponent<ForcePodTag>(forcePod);
    registry->emplaceComponent<ForcePodComponent>(forcePod, ForcePodState::Attached,
                                                  -60.0F, -30.0F, 2000);
    registry->emplaceComponent<TransformComponent>(forcePod, 0.0F, 0.0F, 0.0F);

    system->update(*registry, 0.016F);

    const auto& podTransform = registry->getComponent<TransformComponent>(forcePod);
    EXPECT_FLOAT_EQ(podTransform.x, 240.0F);
    EXPECT_FLOAT_EQ(podTransform.y, 370.0F);
}

TEST_F(ForcePodAttachmentSystemTest, DetachedPodDoesNotFollowPlayer) {
    ECS::Entity player = registry->spawnEntity();
    registry->emplaceComponent<PlayerTag>(player);
    registry->emplaceComponent<NetworkIdComponent>(player, 3000);
    registry->emplaceComponent<TransformComponent>(player, 100.0F, 200.0F, 0.0F);

    ECS::Entity forcePod = registry->spawnEntity();
    registry->emplaceComponent<ForcePodTag>(forcePod);
    registry->emplaceComponent<ForcePodComponent>(forcePod, ForcePodState::Detached,
                                                  50.0F, 20.0F, 3000);
    registry->emplaceComponent<TransformComponent>(forcePod, 500.0F, 600.0F, 0.0F);

    system->update(*registry, 0.016F);

    const auto& podTransform = registry->getComponent<TransformComponent>(forcePod);
    EXPECT_FLOAT_EQ(podTransform.x, 500.0F);
    EXPECT_FLOAT_EQ(podTransform.y, 600.0F);
}

TEST_F(ForcePodAttachmentSystemTest, ReturningPodDoesNotSnapToPlayer) {
    ECS::Entity player = registry->spawnEntity();
    registry->emplaceComponent<PlayerTag>(player);
    registry->emplaceComponent<NetworkIdComponent>(player, 4000);
    registry->emplaceComponent<TransformComponent>(player, 100.0F, 200.0F, 0.0F);

    ECS::Entity forcePod = registry->spawnEntity();
    registry->emplaceComponent<ForcePodTag>(forcePod);
    registry->emplaceComponent<ForcePodComponent>(forcePod, ForcePodState::Returning,
                                                  50.0F, 20.0F, 4000);
    registry->emplaceComponent<TransformComponent>(forcePod, 300.0F, 400.0F, 0.0F);

    system->update(*registry, 0.016F);

    const auto& podTransform = registry->getComponent<TransformComponent>(forcePod);
    EXPECT_FLOAT_EQ(podTransform.x, 300.0F);
    EXPECT_FLOAT_EQ(podTransform.y, 400.0F);
}

TEST_F(ForcePodAttachmentSystemTest, PodFollowsPlayerMovement) {
    ECS::Entity player = registry->spawnEntity();
    registry->emplaceComponent<PlayerTag>(player);
    registry->emplaceComponent<NetworkIdComponent>(player, 5000);
    registry->emplaceComponent<TransformComponent>(player, 100.0F, 200.0F, 0.0F);

    ECS::Entity forcePod = registry->spawnEntity();
    registry->emplaceComponent<ForcePodTag>(forcePod);
    registry->emplaceComponent<ForcePodComponent>(forcePod, ForcePodState::Attached,
                                                  40.0F, 10.0F, 5000);
    registry->emplaceComponent<TransformComponent>(forcePod, 0.0F, 0.0F, 0.0F);

    system->update(*registry, 0.016F);
    const auto& podTransform1 = registry->getComponent<TransformComponent>(forcePod);
    EXPECT_FLOAT_EQ(podTransform1.x, 140.0F);
    EXPECT_FLOAT_EQ(podTransform1.y, 210.0F);

    auto& playerTransform = registry->getComponent<TransformComponent>(player);
    playerTransform.x = 200.0F;
    playerTransform.y = 300.0F;

    system->update(*registry, 0.016F);
    const auto& podTransform2 = registry->getComponent<TransformComponent>(forcePod);
    EXPECT_FLOAT_EQ(podTransform2.x, 240.0F);
    EXPECT_FLOAT_EQ(podTransform2.y, 310.0F);
}

TEST_F(ForcePodAttachmentSystemTest, PodInheritsPlayerRotation) {
    ECS::Entity player = registry->spawnEntity();
    registry->emplaceComponent<PlayerTag>(player);
    registry->emplaceComponent<NetworkIdComponent>(player, 6000);
    registry->emplaceComponent<TransformComponent>(player, 100.0F, 200.0F, 45.0F);

    ECS::Entity forcePod = registry->spawnEntity();
    registry->emplaceComponent<ForcePodTag>(forcePod);
    registry->emplaceComponent<ForcePodComponent>(forcePod, ForcePodState::Attached,
                                                  50.0F, 20.0F, 6000);
    registry->emplaceComponent<TransformComponent>(forcePod, 0.0F, 0.0F, 0.0F);

    system->update(*registry, 0.016F);

    const auto& podTransform = registry->getComponent<TransformComponent>(forcePod);
    EXPECT_FLOAT_EQ(podTransform.rotation, 45.0F);
}

TEST_F(ForcePodAttachmentSystemTest, MultiplePodsSamePlayer) {
    ECS::Entity player = registry->spawnEntity();
    registry->emplaceComponent<PlayerTag>(player);
    registry->emplaceComponent<NetworkIdComponent>(player, 7000);
    registry->emplaceComponent<TransformComponent>(player, 100.0F, 200.0F, 0.0F);

    ECS::Entity forcePod1 = registry->spawnEntity();
    registry->emplaceComponent<ForcePodTag>(forcePod1);
    registry->emplaceComponent<ForcePodComponent>(forcePod1, ForcePodState::Attached,
                                                  50.0F, 20.0F, 7000);
    registry->emplaceComponent<TransformComponent>(forcePod1, 0.0F, 0.0F, 0.0F);

    ECS::Entity forcePod2 = registry->spawnEntity();
    registry->emplaceComponent<ForcePodTag>(forcePod2);
    registry->emplaceComponent<ForcePodComponent>(forcePod2, ForcePodState::Attached,
                                                  -50.0F, -20.0F, 7000);
    registry->emplaceComponent<TransformComponent>(forcePod2, 0.0F, 0.0F, 0.0F);

    system->update(*registry, 0.016F);

    const auto& podTransform1 = registry->getComponent<TransformComponent>(forcePod1);
    EXPECT_FLOAT_EQ(podTransform1.x, 150.0F);
    EXPECT_FLOAT_EQ(podTransform1.y, 220.0F);

    const auto& podTransform2 = registry->getComponent<TransformComponent>(forcePod2);
    EXPECT_FLOAT_EQ(podTransform2.x, 50.0F);
    EXPECT_FLOAT_EQ(podTransform2.y, 180.0F);
}

TEST_F(ForcePodAttachmentSystemTest, PodWithNonExistentOwner) {
    ECS::Entity forcePod = registry->spawnEntity();
    registry->emplaceComponent<ForcePodTag>(forcePod);
    registry->emplaceComponent<ForcePodComponent>(forcePod, ForcePodState::Attached,
                                                  50.0F, 20.0F, 9999);
    registry->emplaceComponent<TransformComponent>(forcePod, 100.0F, 200.0F, 0.0F);

    system->update(*registry, 0.016F);

    const auto& podTransform = registry->getComponent<TransformComponent>(forcePod);
    EXPECT_FLOAT_EQ(podTransform.x, 100.0F);
    EXPECT_FLOAT_EQ(podTransform.y, 200.0F);
}

TEST_F(ForcePodAttachmentSystemTest, UpdateWithNoEntities) {
    system->update(*registry, 0.016F);
}
