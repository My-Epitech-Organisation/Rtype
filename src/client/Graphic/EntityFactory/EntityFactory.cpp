/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** EntityFactory.hpp
*/

#include "EntityFactory.hpp"

#include "../../../games/rtype/client/Components/ImageComponent.hpp"
#include "../../../games/rtype/client/Components/ParallaxComponent.hpp"
#include "../../../games/rtype/client/Components/SizeComponent.hpp"
#include "../../../games/rtype/client/Components/TagComponent.hpp"
#include "../../../games/rtype/client/Components/TextureRectComponent.hpp"
#include "../../../games/rtype/shared/Components/PositionComponent.hpp"
#include "../../../games/rtype/shared/Components/VelocityComponent.hpp"
#include "Components/ImageComponent.hpp"
#include "Components/ParallaxComponent.hpp"
#include "Components/PositionComponent.hpp"
#include "Components/SizeComponent.hpp"
#include "Components/TagComponent.hpp"
#include "Components/TextureRectComponent.hpp"
#include "Components/VelocityComponent.hpp"
#include "Components/ZIndexComponent.hpp"

std::vector<ECS::Entity> EntityFactory::createBackground(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> assetManager, const std::string& PageName) {
    auto background = registry->spawnEntity();
    auto& bgTexture = assetManager->textureManager->get("bg_menu");
    registry->emplaceComponent<rtype::games::rtype::client::Image>(background,
                                                                   bgTexture);
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(
        background, 0, 0);
    registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        background, 0.2, true);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(background,
                                                                    -2);

    auto planet1 = registry->spawnEntity();
    auto& planet1Texture = assetManager->textureManager->get("bg_planet_1");
    registry->emplaceComponent<rtype::games::rtype::client::Image>(
        planet1, planet1Texture);
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(planet1,
                                                                      0, 0);
    registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        planet1, 0.7, true);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(planet1,
                                                                    -1);

    auto planet2 = registry->spawnEntity();
    auto& planet2Texture = assetManager->textureManager->get("bg_planet_2");
    registry->emplaceComponent<rtype::games::rtype::client::Image>(
        planet2, planet2Texture);
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(planet2,
                                                                      0, 0);
    registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        planet2, 0.4, true);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(planet2,
                                                                    -1);

    auto planet3 = registry->spawnEntity();
    auto& planet3Texture = assetManager->textureManager->get("bg_planet_3");
    registry->emplaceComponent<rtype::games::rtype::client::Image>(
        planet3, planet3Texture);
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(planet3,
                                                                      0, 0);
    registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        planet3, 0.2, true);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(planet3,
                                                                    -1);

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
    std::shared_ptr<AssetManager> assets, const std::string& title, float x,
    float y, float width, float height) {
    std::vector<ECS::Entity> entities;
    auto bg = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(bg, x, y);
    registry->emplaceComponent<rtype::games::rtype::client::Rectangle>(
        bg, std::pair<float, float>{width, height}, sf::Color(0, 0, 0, 150),
        sf::Color(0, 0, 0, 150));

    if (registry->hasComponent<rtype::games::rtype::client::Rectangle>(bg)) {
        auto& rect =
            registry->getComponent<rtype::games::rtype::client::Rectangle>(bg);
        rect.outlineThickness = 2.0f;
        rect.outlineColor = sf::Color::White;
    }

    if (title.empty()) return {bg};

    entities.push_back(bg);

    auto titleEnt = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(
        titleEnt, x + 20, y + 10);
    registry->emplaceComponent<rtype::games::rtype::client::Text>(
        titleEnt, assets->fontManager->get("title_font"), sf::Color::White, 30,
        title);
    registry->emplaceComponent<rtype::games::rtype::client::StaticTextTag>(
        titleEnt);
    entities.push_back(titleEnt);
    return entities;
}

ECS::Entity EntityFactory::createStaticText(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> assets, const std::string& title,
    const std::string& fontId, float posX, float posY, float size) {
    auto titleEnt = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(
        titleEnt, posX, posY);
    registry->emplaceComponent<rtype::games::rtype::client::Text>(
        titleEnt, assets->fontManager->get(fontId), sf::Color::White, size,
        title);
    registry->emplaceComponent<rtype::games::rtype::client::StaticTextTag>(
        titleEnt);
    return titleEnt;
}
