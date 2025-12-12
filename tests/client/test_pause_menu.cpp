/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for RtypePauseMenu toggle behavior
*/

#include <gtest/gtest.h>

#include <memory>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Window/Event.hpp>

#include "games/rtype/client/GameScene/RtypePauseMenu.hpp"
#include "games/rtype/client/PauseState.hpp"
#include "ECS.hpp"
#include "AllComponents.hpp"

namespace client = rtype::games::rtype::client;

// Helper to create a registry
static std::shared_ptr<ECS::Registry> makeRegistry() {
    return std::make_shared<ECS::Registry>();
}

TEST(RtypePauseMenuTest, ToggleCreatesPauseStateWhenMissing) {
    auto registry = makeRegistry();

    EXPECT_FALSE(registry->hasSingleton<client::PauseState>());

    client::RtypePauseMenu::togglePauseMenu(registry);

    EXPECT_TRUE(registry->hasSingleton<client::PauseState>());
    auto &pauseState = registry->getSingleton<client::PauseState>();
    EXPECT_TRUE(pauseState.isPaused);
}

TEST(RtypePauseMenuTest, ToggleFlipsPauseStateWhenPresent) {
    auto registry = makeRegistry();

    registry->setSingleton<client::PauseState>(client::PauseState{false});
    auto &pauseState0 = registry->getSingleton<client::PauseState>();
    EXPECT_FALSE(pauseState0.isPaused);

    client::RtypePauseMenu::togglePauseMenu(registry);
    auto &pauseState1 = registry->getSingleton<client::PauseState>();
    EXPECT_TRUE(pauseState1.isPaused);

    client::RtypePauseMenu::togglePauseMenu(registry);
    auto &pauseState2 = registry->getSingleton<client::PauseState>();
    EXPECT_FALSE(pauseState2.isPaused);
}

TEST(RtypePauseMenuTest, ToggleFlipsHiddenOnTaggedEntities) {
    auto registry = makeRegistry();

    // Create 3 entities, all tagged as part of pause menu
    auto e1 = registry->spawnEntity();
    auto e2 = registry->spawnEntity();
    auto e3 = registry->spawnEntity();

    registry->emplaceComponent<client::HiddenComponent>(e1, true);
    registry->emplaceComponent<client::HiddenComponent>(e2, false);
    registry->emplaceComponent<client::HiddenComponent>(e3, true);

    registry->emplaceComponent<rtype::games::rtype::client::PauseMenuTag>(e1);
    registry->emplaceComponent<rtype::games::rtype::client::PauseMenuTag>(e2);
    registry->emplaceComponent<rtype::games::rtype::client::PauseMenuTag>(e3);

    // First toggle: all HiddenComponent should flip
    client::RtypePauseMenu::togglePauseMenu(registry);
    {
        auto &h1 = registry->getComponent<client::HiddenComponent>(e1);
        auto &h2 = registry->getComponent<client::HiddenComponent>(e2);
        auto &h3 = registry->getComponent<client::HiddenComponent>(e3);
        EXPECT_FALSE(h1.isHidden);
        EXPECT_TRUE(h2.isHidden);
        EXPECT_FALSE(h3.isHidden);
    }

    // Second toggle: flip back
    client::RtypePauseMenu::togglePauseMenu(registry);
    {
        auto &h1 = registry->getComponent<client::HiddenComponent>(e1);
        auto &h2 = registry->getComponent<client::HiddenComponent>(e2);
        auto &h3 = registry->getComponent<client::HiddenComponent>(e3);
        EXPECT_TRUE(h1.isHidden);
        EXPECT_FALSE(h2.isHidden);
        EXPECT_TRUE(h3.isHidden);
    }
}
