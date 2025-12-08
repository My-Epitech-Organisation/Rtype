/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for EntityFactory
*/

#include <gtest/gtest.h>
#include <ECS.hpp>
#include "../src/client/Graphic/EntityFactory/EntityFactory.hpp"
#include "games/rtype/shared/Components/PositionComponent.hpp"
#include "games/rtype/client/Components/TextComponent.hpp"
#include "games/rtype/client/Components/RectangleComponent.hpp"
#include "games/rtype/client/Components/ButtonComponent.hpp"
#include "games/rtype/client/Components/UserEventComponent.hpp"
#include "games/rtype/client/Components/TagComponent.hpp"

TEST(EntityFactoryTest, CreateButton_AddsComponents) {
    std::shared_ptr<ECS::Registry> registry = std::make_shared<ECS::Registry>();
    sf::Font font;
    rtype::games::rtype::client::Text text{font, sf::Color::White, 24, "Test"};
    rtype::games::rtype::shared::Position pos{0.0f, 0.0f};
    rtype::games::rtype::client::Rectangle rect{{100.0f, 50.0f}, sf::Color::Blue, sf::Color::Red};
    std::function<void()> onClick = []() {};

    auto entity = EntityFactory::createButton(registry, text, pos, rect, onClick);

    EXPECT_TRUE(registry->hasComponent<rtype::games::rtype::client::Text>(entity));
    EXPECT_TRUE(registry->hasComponent<rtype::games::rtype::shared::Position>(entity));
    EXPECT_TRUE(registry->hasComponent<rtype::games::rtype::client::Rectangle>(entity));
    EXPECT_TRUE(registry->hasComponent<rtype::games::rtype::client::Button<>>(entity));
    EXPECT_TRUE(registry->hasComponent<rtype::games::rtype::client::UserEvent>(entity));
    EXPECT_TRUE(registry->hasComponent<rtype::games::rtype::client::ButtonTag>(entity));
}

TEST(EntityFactoryTest, CreateButton_ReturnsValidEntity) {
    std::shared_ptr<ECS::Registry> registry = std::make_shared<ECS::Registry>();
    sf::Font font;
    rtype::games::rtype::client::Text text{font, sf::Color::White, 24, "Test"};
    rtype::games::rtype::shared::Position pos{0.0f, 0.0f};
    rtype::games::rtype::client::Rectangle rect{{100.0f, 50.0f}, sf::Color::Blue, sf::Color::Red};
    std::function<void()> onClick = []() {};

    auto entity =  EntityFactory::createButton(registry, text, pos, rect, onClick);

    EXPECT_NE(entity, ECS::Entity{});
}
