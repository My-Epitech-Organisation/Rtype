/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for Client Systems
*/

#include <gtest/gtest.h>
#include <memory>
#include <SFML/Graphics.hpp>
#include <ecs/ECS.hpp>
#include "../src/client/System/MovementSystem.hpp"
#include "../src/client/System/RenderSystem.hpp"
#include "../src/client/Components/Common/PositionComponent.hpp"
#include "../src/client/Components/Graphic/VelocityComponent.hpp"
#include "../src/client/Components/Graphic/ImageComponent.hpp"

class SystemsTest : public ::testing::Test {
protected:
    std::shared_ptr<ECS::Registry> registry;
    sf::RenderWindow window;

    void SetUp() override {
        registry = std::make_shared<ECS::Registry>();
        window.create(sf::VideoMode({800, 600}), "Test");
    }

    void TearDown() override {
        window.close();
    }
};

TEST_F(SystemsTest, MovementSystem_Update_AppliesVelocity) {
    sf::Texture texture;
    unsigned char pixels[4] = {255, 0, 0, 255};  // Red pixel
    texture.loadFromMemory(pixels, sizeof(pixels));
    auto entity = registry->spawnEntity();
    registry->emplaceComponent<Velocity>(entity, Velocity{1.0f, 2.0f});
    registry->emplaceComponent<Position>(entity, Position{0.0f, 0.0f});
    registry->emplaceComponent<Image>(entity, Image{texture});

    MovementSystem::update(registry, 1.0f);

    auto& pos = registry->getComponent<Position>(entity);
    EXPECT_EQ(pos.x, 1.0f);
    EXPECT_EQ(pos.y, 2.0f);
}

TEST_F(SystemsTest, RenderSystem_Draw_DoesNotThrow) {
    auto entity = registry->spawnEntity();
    // Add renderable components if needed

    EXPECT_NO_THROW(RenderSystem::draw(registry, window));
}

TEST_F(SystemsTest, MovementSystem_NoVelocity_NoChange) {
    auto entity = registry->spawnEntity();
    registry->emplaceComponent<Position>(entity, Position{10.0f, 20.0f});
    // No velocity

    MovementSystem::update(registry, 1.0f);

    auto& pos = registry->getComponent<Position>(entity);
    EXPECT_EQ(pos.x, 10.0f);
    EXPECT_EQ(pos.y, 20.0f);
}
