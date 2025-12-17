/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for Client Systems
*/

#include <gtest/gtest.h>
#include <memory>
#include <SFML/Graphics.hpp>
#include <ECS.hpp>
#include "../../src/games/rtype/client/Systems/SpritePositionSystem.hpp"
#include "../../src/games/rtype/client/Systems/RenderSystem.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"
#include "games/rtype/shared/Components/VelocityComponent.hpp"
#include "games/rtype/client/Components/ImageComponent.hpp"

using Position = rtype::games::rtype::shared::TransformComponent;
using Velocity = rtype::games::rtype::shared::VelocityComponent;
using Image = rtype::games::rtype::client::Image;
using SpritePositionSystem = rtype::games::rtype::client::SpritePositionSystem;
using RenderSystem = rtype::games::rtype::client::RenderSystem;

class SystemsTest : public ::testing::Test {
protected:
    std::shared_ptr<ECS::Registry> registry;
    std::shared_ptr<sf::RenderWindow> window;

    void SetUp() override {
        registry = std::make_shared<ECS::Registry>();
        window = std::make_shared<sf::RenderWindow>(sf::VideoMode({800, 600}), "Test");
    }

    void TearDown() override {
        window->close();
    }
};

// Type aliases for readability
using Velocity = rtype::games::rtype::shared::VelocityComponent;
using Position = rtype::games::rtype::shared::TransformComponent;
using Image = rtype::games::rtype::client::Image;
using SpritePositionSystem = rtype::games::rtype::client::SpritePositionSystem;
using RenderSystem = rtype::games::rtype::client::RenderSystem;

TEST_F(SystemsTest, SpritePositionSystem_Update_SyncsSprite) {
    sf::Texture texture;
    unsigned char pixels[4] = {255, 0, 0, 255};  // Red pixel
    texture.loadFromMemory(pixels, sizeof(pixels));
    auto entity = registry->spawnEntity();
    registry->emplaceComponent<Position>(entity, Position{10.0f, 20.0f});
    registry->emplaceComponent<Image>(entity, texture);

    SpritePositionSystem spritePositionSystem;
    spritePositionSystem.update(*registry, 0.0f);

    auto& img = registry->getComponent<Image>(entity);
    auto spritePos = img.sprite.getPosition();
    EXPECT_FLOAT_EQ(spritePos.x, 10.0f);
    EXPECT_FLOAT_EQ(spritePos.y, 20.0f);
}

TEST_F(SystemsTest, RenderSystem_Draw_DoesNotThrow) {
    auto entity = registry->spawnEntity();
    // Add renderable components if needed

    RenderSystem renderSystem(window);
    EXPECT_NO_THROW(renderSystem.update(*registry, 0.f));
}

TEST_F(SystemsTest, SpritePositionSystem_NoImage_NoError) {
    auto entity = registry->spawnEntity();
    registry->emplaceComponent<Position>(entity, Position{10.0f, 20.0f});
    // No Image component

    SpritePositionSystem spritePositionSystem;
    EXPECT_NO_THROW(spritePositionSystem.update(*registry, 0.0f));

    auto& pos = registry->getComponent<Position>(entity);
    EXPECT_EQ(pos.x, 10.0f);
    EXPECT_EQ(pos.y, 20.0f);
}
