/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for EntityFactory
*/

#include <gtest/gtest.h>
#include <ECS.hpp>
#include "../src/client/Graphic/EntityFactory/EntityFactory.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"
#include "games/rtype/client/Components/TextComponent.hpp"
#include "../src/client/Graphic/AssetManager/AssetManager.hpp"
#include "games/rtype/client/Components/RectangleComponent.hpp"
#include "games/rtype/client/Components/ButtonComponent.hpp"
#include "games/rtype/client/Components/UserEventComponent.hpp"
#include "games/rtype/client/Components/TagComponent.hpp"

TEST(EntityFactoryTest, CreateButton_AddsComponents) {
    std::shared_ptr<ECS::Registry> registry = std::make_shared<ECS::Registry>();
    sf::Font font;
    rtype::games::rtype::client::Text text{font, sf::Color::White, 24, "Test"};
    rtype::games::rtype::shared::TransformComponent pos{0.0f, 0.0f};
    rtype::games::rtype::client::Rectangle rect{{100.0f, 50.0f}, sf::Color::Blue, sf::Color::Red};
    std::function<void()> onClick = []() {};

    auto entity = EntityFactory::createButton(registry, text, pos, rect, nullptr, onClick);

    EXPECT_TRUE(registry->hasComponent<rtype::games::rtype::client::Text>(entity));
    EXPECT_TRUE(registry->hasComponent<rtype::games::rtype::shared::TransformComponent>(entity));
    EXPECT_TRUE(registry->hasComponent<rtype::games::rtype::client::Rectangle>(entity));
    EXPECT_TRUE(registry->hasComponent<rtype::games::rtype::client::Button<>>(entity));
    EXPECT_TRUE(registry->hasComponent<rtype::games::rtype::client::UserEvent>(entity));
    EXPECT_TRUE(registry->hasComponent<rtype::games::rtype::client::ButtonTag>(entity));
}

TEST(EntityFactoryTest, CreateButton_ReturnsValidEntity) {
    std::shared_ptr<ECS::Registry> registry = std::make_shared<ECS::Registry>();
    sf::Font font;
    rtype::games::rtype::client::Text text{font, sf::Color::White, 24, "Test"};
    rtype::games::rtype::shared::TransformComponent pos{0.0f, 0.0f};
    rtype::games::rtype::client::Rectangle rect{{100.0f, 50.0f}, sf::Color::Blue, sf::Color::Red};
    std::function<void()> onClick = []() {};

    auto entity =  EntityFactory::createButton(registry, text, pos, rect, nullptr, onClick);

    EXPECT_NE(entity, ECS::Entity{});
}

TEST(EntityFactoryTest, CreateBackgroundAndSection) {
    std::shared_ptr<ECS::Registry> registry = std::make_shared<ECS::Registry>();
    rtype::game::config::RTypeConfigParser parser;
    auto cfg = parser.loadFromFile("../../assets/config.toml");
    ASSERT_TRUE(cfg.has_value());

    auto assets = std::make_shared<AssetManager>(*cfg);
    // Preload textures and fonts
    assets->textureManager->load("bg_menu", "../../assets/img/bgMainMenu.png");
    assets->textureManager->load("bg_planet_1", "../../assets/img/planet1.png");
    assets->textureManager->load("bg_planet_2", "../../assets/img/planet2.png");
    assets->textureManager->load("bg_planet_3", "../../assets/img/planet3.png");
    assets->fontManager->load("title_font", "../../assets/fonts/Audiowide-Regular.ttf");

    // No title
    auto bgEntities = EntityFactory::createBackground(registry, assets, "");
    EXPECT_EQ(bgEntities.size(), 4u);

    // With title
    auto bgWithTitle = EntityFactory::createBackground(registry, assets, "MyPage");
    EXPECT_EQ(bgWithTitle.size(), 5u);

    // Section without title
    sf::Vector2f posVec(0.f, 0.f);
    sf::Vector2f sizeVec(100.f, 50.f);
    sf::FloatRect rect(posVec, sizeVec);
    auto sectionNoTitle = EntityFactory::createSection(registry, assets, "", rect);
    EXPECT_EQ(sectionNoTitle.size(), 1u);

    // Section with title
    auto sectionWithTitle = EntityFactory::createSection(registry, assets, "Section", rect);
    EXPECT_EQ(sectionWithTitle.size(), 2u);
}

TEST(EntityFactoryTest, CreateStaticTextAndTextInput) {
    std::shared_ptr<ECS::Registry> registry = std::make_shared<ECS::Registry>();
    rtype::game::config::RTypeConfigParser parser;
    auto cfg = parser.loadFromFile("../../assets/config.toml");
    ASSERT_TRUE(cfg.has_value());

    auto assets = std::make_shared<AssetManager>(*cfg);
    assets->fontManager->load("main_font", "../../assets/fonts/Orbitron-VariableFont_wght.ttf");

    auto staticEnt = EntityFactory::createStaticText(registry, assets, "Title", "main_font", {10.f, 10.f}, 24.f);
    EXPECT_TRUE(registry->hasComponent<rtype::games::rtype::client::Text>(staticEnt));

    sf::Vector2f bpos(0.f, 0.f);
    sf::Vector2f bsize(150.f, 30.f);
    sf::FloatRect bounds(bpos, bsize);
    auto inputEnt = EntityFactory::createTextInput(registry, assets, bounds, "Enter", "", 10, false);
    EXPECT_TRUE(registry->hasComponent<rtype::games::rtype::client::TextInput>(inputEnt));
    EXPECT_TRUE(registry->hasComponent<rtype::games::rtype::client::TextInputTag>(inputEnt));
}
