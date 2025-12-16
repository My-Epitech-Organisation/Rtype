/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** EntityFactory.hpp
*/

#include "EntityFactory.hpp"

#include <string>
#include <string_view>

#include "../../games/rtype/client/GraphicsConstants.hpp"
#include "Components/CountdownComponent.hpp"
#include "Components/ImageComponent.hpp"
#include "Components/LifetimeComponent.hpp"
#include "Components/ParallaxComponent.hpp"
#include "Components/PositionComponent.hpp"
#include "Components/SizeComponent.hpp"
#include "Components/TagComponent.hpp"
#include "Components/Tags.hpp"
#include "Components/TextInputComponent.hpp"
#include "Components/TextureRectComponent.hpp"
#include "Components/VelocityComponent.hpp"
#include "Components/ZIndexComponent.hpp"

namespace cfg = ::rtype::games::rtype::client::GraphicsConfig;

std::vector<ECS::Entity> EntityFactory::createBackground(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> assetManager, std::string_view PageName) {
    auto background = registry->spawnEntity();
    auto& bgTexture = assetManager->textureManager->get("bg_menu");
    registry->emplaceComponent<rtype::games::rtype::client::Image>(background,
                                                                   bgTexture);
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(
        background, 0, 0);
    // registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
    //     background, cfg::PARALLAX_BACKGROUND, true);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        background, cfg::ZINDEX_BACKGROUND);

    auto sun = registry->spawnEntity();
    auto& sunTexture = assetManager->textureManager->get("bg_sun");
    registry->emplaceComponent<rtype::games::rtype::client::Image>(sun,
                                                                   sunTexture);
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(sun, 0,
                                                                      0);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        sun, cfg::ZINDEX_SUN);

    auto bigAsteroids = registry->spawnEntity();
    auto& bigAsteroidsTexture =
        assetManager->textureManager->get("bg_big_asteroids");
    registry->emplaceComponent<rtype::games::rtype::client::Image>(
        bigAsteroids, bigAsteroidsTexture);
    registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        bigAsteroids, cfg::PARALLAX_BIG_ASTEROIDS, true);
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(
        bigAsteroids, 0, 0);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        bigAsteroids, cfg::ZINDEX_BIG_SMALL_ASTEROIDS);

    auto smallAsteroids = registry->spawnEntity();
    auto& smallAsteroidsTexture =
        assetManager->textureManager->get("bg_small_asteroids");
    registry->emplaceComponent<rtype::games::rtype::client::Image>(
        smallAsteroids, smallAsteroidsTexture);
    registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        smallAsteroids, cfg::PARALLAX_SMALL_ASTEROIDS, true);
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(
        smallAsteroids, 0, 0);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        smallAsteroids, cfg::ZINDEX_BIG_SMALL_ASTEROIDS);

    auto firstPlanAsteroids = registry->spawnEntity();
    auto& firstPlanAsteroidsTexture =
        assetManager->textureManager->get("bg_fst_plan_asteroids");
    registry->emplaceComponent<rtype::games::rtype::client::Image>(
        firstPlanAsteroids, firstPlanAsteroidsTexture);
    registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        firstPlanAsteroids, cfg::PARALLAX_ASTEROIDS_FST_PLAN, true);
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(
        firstPlanAsteroids, 0, 0);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        firstPlanAsteroids, cfg::ZINDEX_FST_PLAN_ASTEROIDS);

    auto secondPlanAsteroids = registry->spawnEntity();
    auto& secondPlanAsteroidsTexture =
        assetManager->textureManager->get("bg_snd_plan_asteroids");
    registry->emplaceComponent<rtype::games::rtype::client::Image>(
        secondPlanAsteroids, secondPlanAsteroidsTexture);
    registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        secondPlanAsteroids, cfg::PARALLAX_ASTEROIDS_SND_PLAN, true);
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(
        secondPlanAsteroids, 0, 0);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        secondPlanAsteroids, cfg::ZINDEX_SND_PLAN_ASTEROIDS);

    auto planet1 = registry->spawnEntity();
    auto& planet1Texture = assetManager->textureManager->get("bg_planet_1");
    registry->emplaceComponent<rtype::games::rtype::client::Image>(
        planet1, planet1Texture);
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(planet1,
                                                                      0, 0);
    registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        planet1, cfg::PARALLAX_PLANET_1, true);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        planet1, cfg::ZINDEX_PLANETS);

    auto planet2 = registry->spawnEntity();
    auto& planet2Texture = assetManager->textureManager->get("bg_planet_2");
    registry->emplaceComponent<rtype::games::rtype::client::Image>(
        planet2, planet2Texture);
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(planet2,
                                                                      0, 0);
    registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        planet2, cfg::PARALLAX_PLANET_2, true);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        planet2, cfg::ZINDEX_PLANETS);

    auto planet3 = registry->spawnEntity();
    auto& planet3Texture = assetManager->textureManager->get("bg_planet_3");
    registry->emplaceComponent<rtype::games::rtype::client::Image>(
        planet3, planet3Texture);
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(planet3,
                                                                      0, 0);
    registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        planet3, cfg::PARALLAX_PLANET_3, true);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        planet3, cfg::ZINDEX_PLANETS);

    if (PageName.empty()) return {planet1, planet2, planet3, background};
    auto appTitle = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::client::Text>(
        appTitle, assetManager->fontManager->get("title_font"),
        sf::Color::White, 72, PageName);
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(appTitle,
                                                                      50, 50);
    registry->emplaceComponent<rtype::games::rtype::client::StaticTextTag>(
        appTitle);
    return {planet1, planet2, planet3, background, appTitle};
}

std::vector<ECS::Entity> EntityFactory::createSection(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> assets, std::string_view title,
    const sf::FloatRect& bounds) {
    std::vector<ECS::Entity> entities;
    auto bg = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(
        bg, bounds.position.x, bounds.position.y);
    registry->emplaceComponent<rtype::games::rtype::client::Rectangle>(
        bg, std::pair<float, float>{bounds.size.x, bounds.size.y},
        sf::Color(0, 0, 0, 150), sf::Color(0, 0, 0, 150));

    if (registry->hasComponent<rtype::games::rtype::client::Rectangle>(bg)) {
        auto& rect =
            registry->getComponent<rtype::games::rtype::client::Rectangle>(bg);
        rect.outlineThickness = cfg::UI_OUTLINE_THICKNESS;
        rect.outlineColor = sf::Color::White;
    }

    if (title.empty()) return {bg};

    entities.push_back(bg);

    auto titleEnt = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(
        titleEnt, bounds.position.x + cfg::SECTION_TITLE_OFFSET_X,
        bounds.position.y + cfg::SECTION_TITLE_OFFSET_Y);
    registry->emplaceComponent<rtype::games::rtype::client::Text>(
        titleEnt, assets->fontManager->get("title_font"), sf::Color::White,
        cfg::SECTION_TITLE_FONT_SIZE, title);
    registry->emplaceComponent<rtype::games::rtype::client::StaticTextTag>(
        titleEnt);
    entities.push_back(titleEnt);
    return entities;
}

ECS::Entity EntityFactory::createStaticText(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> assets, std::string_view title,
    std::string_view fontId, const sf::Vector2f& position, float size) {
    auto titleEnt = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(
        titleEnt, position.x, position.y);
    registry->emplaceComponent<rtype::games::rtype::client::Text>(
        titleEnt, assets->fontManager->get(std::string(fontId)),
        sf::Color::White, size, title);
    registry->emplaceComponent<rtype::games::rtype::client::StaticTextTag>(
        titleEnt);
    return titleEnt;
}

ECS::Entity EntityFactory::createTextInput(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> assetManager, const sf::FloatRect& bounds,
    std::string_view placeholder, std::string_view initialValue,
    std::size_t maxLength, bool isNumericOnly) {
    auto entity = registry->spawnEntity();

    registry->emplaceComponent<rtype::games::rtype::client::TextInput>(
        entity, assetManager->fontManager->get("main_font"), bounds.size.x,
        bounds.size.y, placeholder, initialValue, maxLength, isNumericOnly);

    registry->emplaceComponent<rtype::games::rtype::shared::Position>(
        entity, bounds.position.x, bounds.position.y);

    registry->emplaceComponent<rtype::games::rtype::client::Rectangle>(
        entity, std::pair<float, float>{bounds.size.x, bounds.size.y},
        sf::Color::Transparent, sf::Color::Transparent);

    registry->emplaceComponent<rtype::games::rtype::client::TextInputTag>(
        entity);

    registry->emplaceComponent<rtype::games::rtype::client::UserEvent>(entity);

    return entity;
}
