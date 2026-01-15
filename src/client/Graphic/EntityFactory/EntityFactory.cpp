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
    std::shared_ptr<AssetManager> /*assetManager*/, std::string_view PageName,
    std::unique_ptr<IBackground> backgroundLib) {
    if (backgroundLib) backgroundLib->createEntitiesBackground();
    if (PageName.empty()) return {};
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
    return {appTitle};
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

ECS::Entity EntityFactory::createStaticImage(
    std::shared_ptr<ECS::Registry> registry, std::string_view textureId,
    const ::rtype::display::Vector2f& position, float scale) {
    auto entity = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::client::Image>(
        entity, std::string(textureId));
    registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        entity, position.x, position.y);
    registry->emplaceComponent<rtype::games::rtype::client::Size>(entity, scale,
                                                                  scale);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(entity, 2);
    return entity;
}
