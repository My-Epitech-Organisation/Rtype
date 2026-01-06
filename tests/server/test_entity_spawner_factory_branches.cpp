/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_entity_spawner_factory_branches - Unit tests for EntitySpawnerFactory
*/

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "serverApp/game/entitySpawnerFactory/EntitySpawnerFactory.hpp"

using namespace rtype::server;

TEST(EntitySpawnerFactoryTest, RegisterEmptyGameIdFails) {
    EXPECT_FALSE(EntitySpawnerFactory::registerSpawner("", nullptr));
}

TEST(EntitySpawnerFactoryTest, RegisterNullCreatorFails) {
    EXPECT_FALSE(EntitySpawnerFactory::registerSpawner("mock_game", nullptr));
}

TEST(EntitySpawnerFactoryTest, RegisterDuplicateFails) {
    auto dummyCreator = [](std::shared_ptr<ECS::Registry>, std::shared_ptr<ServerNetworkSystem>, GameEngineOpt, GameConfigOpt) {
        return std::unique_ptr<IEntitySpawner>(nullptr);
    };

    // Ensure clean registry
    EntitySpawnerFactory::clearRegistry();

    EXPECT_TRUE(EntitySpawnerFactory::registerSpawner("dup_game", dummyCreator));
    EXPECT_FALSE(EntitySpawnerFactory::registerSpawner("dup_game", dummyCreator));

    EntitySpawnerFactory::clearRegistry();
}

TEST(EntitySpawnerFactoryTest, UnregisterNonExistentFails) {
    EntitySpawnerFactory::clearRegistry();
    EXPECT_FALSE(EntitySpawnerFactory::unregisterSpawner("no_such_game"));
}

TEST(EntitySpawnerFactoryTest, CreateUnknownReturnsNullptr) {
    EntitySpawnerFactory::clearRegistry();
    auto spawner = EntitySpawnerFactory::create("unknown_game", nullptr, nullptr, {}, {});
    EXPECT_EQ(spawner, nullptr);
}

TEST(EntitySpawnerFactoryTest, IsRegisteredAndGetRegisteredSpawners) {
    EntitySpawnerFactory::clearRegistry();

    auto dummyCreatorA = [](std::shared_ptr<ECS::Registry>, std::shared_ptr<ServerNetworkSystem>, GameEngineOpt, GameConfigOpt) {
        return std::unique_ptr<IEntitySpawner>(nullptr);
    };
    auto dummyCreatorB = dummyCreatorA;

    EXPECT_TRUE(EntitySpawnerFactory::registerSpawner("bbb", dummyCreatorA));
    EXPECT_TRUE(EntitySpawnerFactory::registerSpawner("aaa", dummyCreatorB));

    EXPECT_TRUE(EntitySpawnerFactory::isRegistered("aaa"));
    EXPECT_TRUE(EntitySpawnerFactory::isRegistered("bbb"));
    EXPECT_FALSE(EntitySpawnerFactory::isRegistered("ccc"));

    auto list = EntitySpawnerFactory::getRegisteredSpawners();
    ASSERT_EQ(list.size(), 2u);
    // Should be sorted
    EXPECT_EQ(list[0], "aaa");
    EXPECT_EQ(list[1], "bbb");

    EntitySpawnerFactory::clearRegistry();
    EXPECT_TRUE(EntitySpawnerFactory::getRegisteredSpawners().empty());
}
