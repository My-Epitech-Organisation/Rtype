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
#include "Components/ImageComponent.hpp"
#include "Components/ParallaxComponent.hpp"
#include "Components/PositionComponent.hpp"
#include "Components/SizeComponent.hpp"
#include "Components/TagComponent.hpp"
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
    registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        background, cfg::PARALLAX_BACKGROUND, true);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        background, cfg::ZINDEX_BACKGROUND);

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

ECS::Entity EntityFactory::createPlayer(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> assetsManager, sf::Vector2i scale,
    bool isControllable) {
    auto ent = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::client::Image>(
        ent, assetsManager->textureManager->get("player_vessel"));
    registry->emplaceComponent<rtype::games::rtype::client::TextureRect>(
        ent, std::pair<int, int>({0, 0}), std::pair<int, int>({33, 17}));
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(ent, 0,
                                                                      0);
    registry->emplaceComponent<rtype::games::rtype::client::Size>(ent, scale.x,
                                                                  scale.y);
    registry->emplaceComponent<rtype::games::rtype::shared::VelocityComponent>(
        ent, 0.f, 0.f);
    registry->emplaceComponent<rtype::games::rtype::client::PlayerTag>(ent);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(ent, 0);
    if (isControllable)
        registry
            ->emplaceComponent<rtype::games::rtype::client::ControllableTag>(
                ent);
    return ent;
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
        entity, assetManager->fontManager->get("title_font"), bounds.size.x,
        bounds.size.y, placeholder, initialValue, maxLength, isNumericOnly);

    registry->emplaceComponent<rtype::games::rtype::shared::Position>(
        entity, bounds.position.x, bounds.position.y);

    registry->emplaceComponent<rtype::games::rtype::client::TextInputTag>(
        entity);

    return entity;
}
