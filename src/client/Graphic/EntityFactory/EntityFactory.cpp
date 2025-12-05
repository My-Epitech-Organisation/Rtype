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
#include "../../../games/rtype/client/Components/TextureRectComponent.hpp"
#include "../../../games/rtype/shared/Components/PositionComponent.hpp"
#include "../../../games/rtype/shared/Components/Tags.hpp"
#include "../../../games/rtype/shared/Components/VelocityComponent.hpp"

std::vector<ECS::Entity> createBackground(
    const std::shared_ptr<ECS::Registry>& registry,
    const std::shared_ptr<AssetManager>& assetManager,
    const std::string& PageName) {
    auto background = registry->spawnEntity();
    auto& bgTexture = assetManager->textureManager->get("bg_menu");
    bgTexture.setRepeated(true);
    registry->emplaceComponent<rtype::games::rtype::client::Image>(background,
                                                                   bgTexture);
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(
        background, 0, 0);
    registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        background, 0.2, true);
    registry->emplaceComponent<rtype::games::rtype::shared::BackgroundTag>(
        background);

    auto planet1 = registry->spawnEntity();
    auto& planet1Texture = assetManager->textureManager->get("bg_planet_1");
    planet1Texture.setRepeated(true);
    registry->emplaceComponent<rtype::games::rtype::client::Image>(
        planet1, planet1Texture);
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(planet1,
                                                                      0, 0);
    registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        planet1, 0.7, true);
    registry->emplaceComponent<rtype::games::rtype::shared::BackgroundTag>(
        planet1);

    auto planet2 = registry->spawnEntity();
    auto& planet2Texture = assetManager->textureManager->get("bg_planet_2");
    planet2Texture.setRepeated(true);
    registry->emplaceComponent<rtype::games::rtype::client::Image>(
        planet2, planet2Texture);
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(planet2,
                                                                      0, 0);
    registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        planet2, 0.4, true);
    registry->emplaceComponent<rtype::games::rtype::shared::BackgroundTag>(
        planet2);

    auto planet3 = registry->spawnEntity();
    auto& planet3Texture = assetManager->textureManager->get("bg_planet_3");
    planet3Texture.setRepeated(true);
    registry->emplaceComponent<rtype::games::rtype::client::Image>(
        planet3, planet3Texture);
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(planet3,
                                                                      0, 0);
    registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        planet3, 0.2, true);
    registry->emplaceComponent<rtype::games::rtype::shared::BackgroundTag>(
        planet3);

    if (PageName.empty()) return {planet1, planet2, planet3, background};
    auto appTitle = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::client::Text>(
        appTitle, assetManager->fontManager->get("title_font"),
        sf::Color::White, 72, PageName);
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(appTitle,
                                                                      50, 50);
    registry->emplaceComponent<rtype::games::rtype::shared::StaticTextTag>(
        appTitle);
    return {planet1, planet2, planet3, background, appTitle};
}

ECS::Entity createPlayer(const std::shared_ptr<ECS::Registry>& registry,
                         const std::shared_ptr<AssetManager>& assetsManager,
                         sf::Vector2i scale, bool isControllable) {
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
    registry->emplaceComponent<rtype::games::rtype::shared::PlayerTag>(ent);
    if (isControllable)
        registry
            ->emplaceComponent<rtype::games::rtype::shared::ControllableTag>(
                ent);
    return ent;
}

std::vector<ECS::Entity> createSection(
    const std::shared_ptr<ECS::Registry>& registry,
    const std::shared_ptr<AssetManager>& assets, const std::string& title,
    float x, float y, float width, float height) {
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
    registry->emplaceComponent<rtype::games::rtype::shared::StaticTextTag>(
        titleEnt);
    entities.push_back(titleEnt);
    return entities;
}

ECS::Entity createStaticText(const std::shared_ptr<ECS::Registry>& registry,
                             const std::shared_ptr<AssetManager>& assets,
                             const std::string& text, float x, float y) {
    auto titleEnt = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(titleEnt,
                                                                      x, y);
    registry->emplaceComponent<rtype::games::rtype::client::Text>(
        titleEnt, assets->fontManager->get("title_font"), sf::Color::White, 72,
        text);
    registry->emplaceComponent<rtype::games::rtype::shared::StaticTextTag>(
        titleEnt);
    return titleEnt;
}
