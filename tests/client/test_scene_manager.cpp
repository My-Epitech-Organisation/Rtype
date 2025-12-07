/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for SceneManager
*/

#include <gtest/gtest.h>
#include <memory>
#include <SFML/Graphics.hpp>
#include <ecs/ECS.hpp>
#include "../src/client/Graphic/SceneManager/SceneManager.hpp"
#include "../src/client/Graphic/AssetManager/AssetManager.hpp"

class SceneManagerTest : public ::testing::Test {
protected:
    std::shared_ptr<ECS::Registry> registry;
    std::shared_ptr<AssetManager> assetManager;
    std::shared_ptr<KeyboardActions> keyboardActions;
    std::shared_ptr<sf::RenderWindow> window;

    void SetUp() override {
        registry = std::make_shared<ECS::Registry>();
        assetManager = std::make_shared<AssetManager>();
        keyboardActions = std::make_shared<KeyboardActions>();
        window = std::make_shared<sf::RenderWindow>(sf::VideoMode({800, 600}), "Test");
    }

    void TearDown() override {
        window->close();
    }
};

TEST_F(SceneManagerTest, Constructor_InitializesWithMainMenuSceneAfterUpdate) {
    SceneManager manager(registry, assetManager, window, keyboardActions);
    // Scene is applied lazily during update/draw
    manager.update();
    EXPECT_EQ(manager.getCurrentScene(), SceneManager::Scene::MAIN_MENU);
}

TEST_F(SceneManagerTest, SetCurrentScene_ChangesScene) {
    SceneManager manager(registry, assetManager, window, keyboardActions);
    manager.update();  // Apply initial scene
    manager.setCurrentScene(SceneManager::Scene::SETTINGS_MENU);
    manager.update();  // Apply scene change
    EXPECT_EQ(manager.getCurrentScene(), SceneManager::Scene::SETTINGS_MENU);
}

TEST_F(SceneManagerTest, Update_DoesNotThrow) {
    SceneManager manager(registry, assetManager, window, keyboardActions);
    EXPECT_NO_THROW(manager.update());
}

TEST_F(SceneManagerTest, Draw_DoesNotThrow) {
    SceneManager manager(registry, assetManager, window, keyboardActions);
    EXPECT_NO_THROW(manager.draw());
}
