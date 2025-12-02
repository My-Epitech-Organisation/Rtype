/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** EntityFactory.hpp
*/


#include "EntityFactory.hpp"

#include "Components/Common/PositionComponent.hpp"
#include "Components/Graphic/ImageComponent.hpp"
#include "Components/Graphic/TagComponent.hpp"
#include "Graphic/PrallaxComponent.hpp"

std::vector<ECS::Entity> EntityFactory::createBackground(
    const std::shared_ptr<ECS::Registry>& registry, const std::shared_ptr<AssetManager> &assetManager,  const std::string &PageName
) {
    auto background = registry->spawnEntity();
    registry->emplaceComponent<Image>(
        background, assetManager->textureManager->get("bg_menu"));
    registry->emplaceComponent<Position>(background, 0, 0);
    registry->emplaceComponent<Parallax>(background, 0.2, true);

    auto planet1 = registry->spawnEntity();
    registry->emplaceComponent<Image>(
        planet1, assetManager->textureManager->get("bg_planet_1"));
    registry->emplaceComponent<Position>(planet1, 0, 0);
    registry->emplaceComponent<Parallax>(planet1, 0.5, true);

    auto planet2 = registry->spawnEntity();
    registry->emplaceComponent<Image>(
        planet2, assetManager->textureManager->get("bg_planet_2"));
    registry->emplaceComponent<Position>(planet2, 0, 0);
    registry->emplaceComponent<Parallax>(planet2, 0.4, true);

    auto planet3 = registry->spawnEntity();
    registry->emplaceComponent<Image>(
        planet3, assetManager->textureManager->get("bg_planet_3"));
    registry->emplaceComponent<Position>(planet3, 0, 0);
    registry->emplaceComponent<Parallax>(planet3, 0.4, true);

    auto appTitle = registry->spawnEntity();
     registry->emplaceComponent<Text>(
        appTitle, assetManager->fontManager->get("title_font"),
        sf::Color::White, 72, PageName);
     registry->emplaceComponent<Position>(appTitle, 50, 50);
     registry->emplaceComponent<StaticTextTag>(appTitle);
    return {planet1, planet2, planet3, background, appTitle};
}
