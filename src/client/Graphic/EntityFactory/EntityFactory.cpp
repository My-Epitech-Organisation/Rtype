/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** EntityFactory.hpp
*/

#include "EntityFactory.hpp"

#include "Components/ImageComponent.hpp"
#include "Components/ParallaxComponent.hpp"
#include "Components/PositionComponent.hpp"
#include "Components/TagComponent.hpp"

std::vector<ECS::Entity> EntityFactory::createBackground(
    const std::shared_ptr<ECS::Registry>& registry,
    const std::shared_ptr<AssetManager>& assetManager,
    const std::string& PageName) {
    auto background = registry->spawnEntity();
    auto& bgTexture = assetManager->textureManager->get("bg_menu");
    bgTexture.setRepeated(true);
    registry->emplaceComponent<Image>(background, bgTexture);
    registry->emplaceComponent<Position>(background, 0, 0);
    registry->emplaceComponent<Parallax>(background, 0.2, true);

    auto planet1 = registry->spawnEntity();
    auto& planet1Texture = assetManager->textureManager->get("bg_planet_1");
    planet1Texture.setRepeated(true);
    registry->emplaceComponent<Image>(planet1, planet1Texture);
    registry->emplaceComponent<Position>(planet1, 0, 0);
    registry->emplaceComponent<Parallax>(planet1, 0.7, true);

    auto planet2 = registry->spawnEntity();
    auto& planet2Texture = assetManager->textureManager->get("bg_planet_2");
    planet2Texture.setRepeated(true);
    registry->emplaceComponent<Image>(planet2, planet2Texture);
    registry->emplaceComponent<Position>(planet2, 0, 0);
    registry->emplaceComponent<Parallax>(planet2, 0.4, true);

    auto planet3 = registry->spawnEntity();
    auto& planet3Texture = assetManager->textureManager->get("bg_planet_3");
    planet3Texture.setRepeated(true);
    registry->emplaceComponent<Image>(planet3, planet3Texture);
    registry->emplaceComponent<Position>(planet3, 0, 0);
    registry->emplaceComponent<Parallax>(planet3, 0.2, true);

    if (PageName.empty()) return {planet1, planet2, planet3, background};
    auto appTitle = registry->spawnEntity();
    registry->emplaceComponent<Text>(
        appTitle, assetManager->fontManager->get("title_font"),
        sf::Color::White, 72, PageName);
    registry->emplaceComponent<Position>(appTitle, 50, 50);
    registry->emplaceComponent<StaticTextTag>(appTitle);
    return {planet1, planet2, planet3, background, appTitle};
}
