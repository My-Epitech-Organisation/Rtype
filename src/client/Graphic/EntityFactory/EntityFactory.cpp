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
#include "Components/SizeComponent.hpp"
#include "Components/TagComponent.hpp"
#include "Components/Tags.hpp"
#include "Components/TextInputComponent.hpp"
#include "Components/TextureRectComponent.hpp"
#include "Components/VelocityComponent.hpp"
#include "Components/ZIndexComponent.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"

namespace cfg = ::rtype::games::rtype::client::GraphicsConfig;

std::vector<ECS::Entity> EntityFactory::createBackground(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> /*assetManager*/, std::string_view PageName) {
    auto background = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::client::Image>(background,
                                                                   "bg_menu");
    registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        background, 0, 0);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        background, cfg::ZINDEX_BACKGROUND);
    registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        background, cfg::PARALLAX_BACKGROUND, true);

    auto sun = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::client::Image>(sun,
                                                                   "bg_sun");
    registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        sun, 0, 0);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        sun, cfg::ZINDEX_SUN);

    auto bigAsteroids = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::client::Image>(
        bigAsteroids, "bg_big_asteroids");
    registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        bigAsteroids, cfg::PARALLAX_BIG_ASTEROIDS, true);
    registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        bigAsteroids, 0, 0);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        bigAsteroids, cfg::ZINDEX_BIG_ASTEROIDS);

    auto smallAsteroids = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::client::Image>(
        smallAsteroids, "bg_small_asteroids");
    registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        smallAsteroids, cfg::PARALLAX_SMALL_ASTEROIDS, true);
    registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        smallAsteroids, 0, 0);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        smallAsteroids, cfg::ZINDEX_SMALL_ASTEROIDS);

    auto firstPlanAsteroids = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::client::Image>(
        firstPlanAsteroids, "bg_fst_plan_asteroids");
    registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        firstPlanAsteroids, cfg::PARALLAX_ASTEROIDS_FST_PLAN, true);
    registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        firstPlanAsteroids, 0, 0);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        firstPlanAsteroids, cfg::ZINDEX_FST_PLAN_ASTEROIDS);

    auto secondPlanAsteroids = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::client::Image>(
        secondPlanAsteroids, "bg_snd_plan_asteroids");
    registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        secondPlanAsteroids, cfg::PARALLAX_ASTEROIDS_SND_PLAN, true);
    registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        secondPlanAsteroids, 0, 0);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        secondPlanAsteroids, cfg::ZINDEX_SND_PLAN_ASTEROIDS);

    auto planet1 = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::client::Image>(
        planet1, "bg_planet_1");
    registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        planet1, 0, 0);
    registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        planet1, cfg::PARALLAX_PLANET_1, true);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        planet1, cfg::ZINDEX_PLANETS);

    auto planet2 = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::client::Image>(
        planet2, "bg_planet_2");
    registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        planet2, 0, 0);
    registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        planet2, cfg::PARALLAX_PLANET_2, true);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        planet2, cfg::ZINDEX_PLANETS);

    auto planet3 = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::client::Image>(
        planet3, "bg_planet_3");
    registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        planet3, 0, 0);
    registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        planet3, cfg::PARALLAX_PLANET_3, true);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        planet3, cfg::ZINDEX_PLANETS);

    if (PageName.empty()) return {planet1, planet2, planet3, background};
    auto appTitle = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::client::Text>(
        appTitle, "title_font", ::rtype::display::Color::White(), 72,
        std::string(PageName));
    registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        appTitle, 50, 50);
    registry->emplaceComponent<rtype::games::rtype::client::StaticTextTag>(
        appTitle);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        appTitle, cfg::ZINDEX_APP_TITLE);
    return {planet1, planet2, planet3, background, appTitle};
}

std::vector<ECS::Entity> EntityFactory::createSection(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> /*assets*/, std::string_view title,
    const ::rtype::display::Rect<float>& bounds, int ZindexRect) {
    std::vector<ECS::Entity> entities;
    auto bg = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        bg, bounds.left, bounds.top);
    registry->emplaceComponent<rtype::games::rtype::client::Rectangle>(
        bg, std::pair<float, float>{bounds.width, bounds.height},
        ::rtype::display::Color(0, 0, 0, 150),
        ::rtype::display::Color(0, 0, 0, 150));

    if (registry->hasComponent<rtype::games::rtype::client::Rectangle>(bg)) {
        auto& rect =
            registry->getComponent<rtype::games::rtype::client::Rectangle>(bg);
        rect.outlineThickness = cfg::UI_OUTLINE_THICKNESS;
        rect.outlineColor = ::rtype::display::Color::White();
    }
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(bg,
                                                                    ZindexRect);

    if (title.empty()) return {bg};

    entities.push_back(bg);

    auto titleEnt = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        titleEnt, bounds.left + 20.f,
        bounds.top + cfg::SECTION_TITLE_OFFSET_Y +
            cfg::SECTION_TITLE_FONT_SIZE / 2.0f);
    registry->emplaceComponent<rtype::games::rtype::client::Text>(
        titleEnt, "title_font", ::rtype::display::Color::White(),
        cfg::SECTION_TITLE_FONT_SIZE, std::string(title));
    registry->emplaceComponent<rtype::games::rtype::client::StaticTextTag>(
        titleEnt);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        titleEnt, ZindexRect + 1);
    entities.push_back(titleEnt);
    return entities;
}

ECS::Entity EntityFactory::createRectangle(
    std::shared_ptr<ECS::Registry> registry, ::rtype::display::Vector2i size,
    ::rtype::display::Color fill, ::rtype::display::Vector2f position) {
    auto entt = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::client::Rectangle>(
        entt,
        std::pair<float, float>{static_cast<float>(size.x),
                                static_cast<float>(size.y)},
        fill, fill);
    registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        entt, position.x, position.y);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(entt, 1);
    return entt;
}

ECS::Entity EntityFactory::createLobbyPlayer(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> assetManager,
    ::rtype::display::Vector2<float> position,
    ::rtype::display::Vector2<int> scale, bool isControllable) {
    auto entt = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::client::Image>(
        entt, "player_vessel");
    registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        entt, position.x, position.y);
    registry->emplaceComponent<rtype::games::rtype::client::Size>(
        entt, static_cast<float>(scale.x), static_cast<float>(scale.y));
    if (isControllable) {
        registry
            ->emplaceComponent<rtype::games::rtype::client::ControllableTag>(
                entt);
    }
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(entt, 1);
    return entt;
}

ECS::Entity EntityFactory::createStaticText(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> /*assets*/, std::string_view title,
    std::string_view fontId, const ::rtype::display::Vector2f& position,
    float size) {
    auto titleEnt = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        titleEnt, position.x, position.y);
    registry->emplaceComponent<rtype::games::rtype::client::Text>(
        titleEnt, std::string(fontId), ::rtype::display::Color::White(), size,
        std::string(title));
    registry->emplaceComponent<rtype::games::rtype::client::StaticTextTag>(
        titleEnt);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(titleEnt,
                                                                    1);
    return titleEnt;
}

ECS::Entity EntityFactory::createTextInput(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> /*assetManager*/,
    const ::rtype::display::Vector2f& position,
    const ::rtype::display::Vector2f& size, std::string_view placeholder,
    std::string_view initialValue, std::size_t maxLength, bool isNumericOnly) {
    auto entity = registry->spawnEntity();

    registry->emplaceComponent<rtype::games::rtype::client::TextInput>(
        entity, "main_font", size.x, size.y, placeholder, initialValue,
        maxLength, isNumericOnly);

    registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        entity, position.x, position.y);

    registry->emplaceComponent<rtype::games::rtype::client::TextInputTag>(
        entity);

    registry->emplaceComponent<rtype::games::rtype::client::UserEvent>(entity);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(entity, 1);
    return entity;
}
