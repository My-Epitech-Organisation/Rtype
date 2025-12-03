/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for Graphic module
*/

#include <gtest/gtest.h>
#include <SFML/Window/Keyboard.hpp>
#include "../src/client/Graphic/KeyboardActions.hpp"
#include "../src/client/Graphic/AssetManager/AssetManager.hpp"
#include "../src/client/GameAction.hpp"

TEST(KeyboardActionsTest, Constructor_InitializesDefaultBindings) {
    KeyboardActions actions;
    // Assuming default bindings are set
    auto key = actions.getKeyBinding(GameAction::MOVE_UP);
    EXPECT_TRUE(key.has_value());
}

TEST(KeyboardActionsTest, SetKeyBinding_StoresCorrectly) {
    KeyboardActions actions;
    actions.setKeyBinding(GameAction::MOVE_UP, sf::Keyboard::Key::W);
    auto key = actions.getKeyBinding(GameAction::MOVE_UP);
    EXPECT_EQ(key.value(), sf::Keyboard::Key::W);
}

TEST(KeyboardActionsTest, GetKeyBinding_ReturnsNoneForUnset) {
    KeyboardActions actions;
    auto action = actions.getKeyBinding(sf::Keyboard::Key::X);
    EXPECT_FALSE(action.has_value());
}

TEST(AssetManagerTest, Constructor_CreatesManagers) {
    AssetManager manager;
    EXPECT_NE(manager.textureManager, nullptr);
    EXPECT_NE(manager.fontManager, nullptr);
}

TEST(TextureManagerTest, LoadTexture_ThrowsOnMissing) {
    TextureManager manager;
    EXPECT_THROW(manager.load("test", "nonexistent.png"), std::runtime_error);
}

TEST(TextureManagerTest, GetTexture_ThrowsOnMissing) {
    TextureManager manager;
    EXPECT_THROW(manager.get("missing"), std::out_of_range);
}
