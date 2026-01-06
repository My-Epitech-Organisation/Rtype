#include <gtest/gtest.h>

#include "client/Graphic/SceneManager/Scenes/Lobby/Lobby.hpp"
#include "client/network/NetworkClient.hpp"

#include <memory>

using namespace rtype::client;
using namespace rtype::server; // for Endpoint alias if needed

TEST(LobbyTest, EntityDestroyMapsToUser) {
    // Minimal setup
    auto registry = std::make_shared<ECS::Registry>();
    rtype::game::config::RTypeGameConfig cfg;
    auto assets = std::make_shared<AssetManager>(cfg);

    NetworkClient::Config netCfg;
    auto networkClient = std::make_shared<NetworkClient>(netCfg, nullptr, false);

    Lobby lobby(registry, assets, nullptr, [](const SceneManager::Scene&) {}, networkClient, nullptr, nullptr);

    // Create an entity and associate with userId 42
    ECS::Entity ent = registry->spawnEntity();
    lobby.addUserForTest(42, std::vector<ECS::Entity>{ent});

    // Simulate entity destroy
    lobby.onEntityDestroyEvent(ent.id);

    const auto& pending = lobby.getPendingPlayerRemovals();
    ASSERT_EQ(pending.size(), 1u);
    EXPECT_EQ(pending[0], 42u);
}
