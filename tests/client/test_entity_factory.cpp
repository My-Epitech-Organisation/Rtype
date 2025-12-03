/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for EntityFactory
*/

#include <gtest/gtest.h>
#include <ecs/ECS.hpp>
#include "../src/client/Graphic/EntityFactory/EntityFactory.hpp"
#include "../src/client/Components/Common/PositionComponent.hpp"
#include "../src/client/Components/Graphic/TextComponent.hpp"
#include "../src/client/Components/Graphic/RectangleComponent.hpp"
#include "../src/client/Components/Graphic/ButtonComponent.hpp"
#include "../src/client/Components/Graphic/UserEventComponent.hpp"
#include "../src/client/Components/Graphic/TagComponent.hpp"

TEST(EntityFactoryTest, CreateButton_AddsComponents) {
    ECS::Registry registry;
    sf::Font font;
    Text text{font, sf::Color::White, 24, "Test"};
    Position pos{0.0f, 0.0f};
    Rectangle rect{{100.0f, 50.0f}, sf::Color::Blue, sf::Color::Red};
    std::function<void()> onClick = []() {};

    auto entity = EntityFactory::createButton(registry, text, pos, rect, onClick);

    EXPECT_TRUE(registry.hasComponent<Text>(entity));
    EXPECT_TRUE(registry.hasComponent<Position>(entity));
    EXPECT_TRUE(registry.hasComponent<Rectangle>(entity));
    EXPECT_TRUE(registry.hasComponent<Button<>>(entity));
    EXPECT_TRUE(registry.hasComponent<UserEvent>(entity));
    EXPECT_TRUE(registry.hasComponent<ButtonTag>(entity));
}

TEST(EntityFactoryTest, CreateButton_ReturnsValidEntity) {
    ECS::Registry registry;
    sf::Font font;
    Text text{font, sf::Color::White, 24, "Test"};
    Position pos{0.0f, 0.0f};
    Rectangle rect{{100.0f, 50.0f}, sf::Color::Blue, sf::Color::Red};
    std::function<void()> onClick = []() {};

    auto entity = EntityFactory::createButton(registry, text, pos, rect, onClick);

    EXPECT_NE(entity, ECS::Entity{});
}
