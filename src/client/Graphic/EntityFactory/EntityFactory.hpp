/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** EntityFactory.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_ENTITYFACTORY_ENTITYFACTORY_HPP_
#define SRC_CLIENT_GRAPHIC_ENTITYFACTORY_ENTITYFACTORY_HPP_

#include <memory>
#include <vector>

#include <SFML/System/Vector2.hpp>

#include "AssetManager/AssetManager.hpp"
#include "Components/ButtonComponent.hpp"
#include "Components/PositionComponent.hpp"
#include "Components/RectangleComponent.hpp"
#include "Components/TagComponent.hpp"
#include "Components/TextComponent.hpp"
#include "Components/UserEventComponent.hpp"
#include "ecs/ECS.hpp"

// namespace EntityFactory {
template <typename... Args>
static ECS::Entity createButton(
    std::shared_ptr<ECS::Registry> registry,
    const rtype::games::rtype::client::Text& text,
    const rtype::games::rtype::shared::Position& position,
    const rtype::games::rtype::client::Rectangle& rectangle,
    std::function<void(Args...)> onClick) {
    auto entity = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::client::Text>(entity, text);
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(entity,
                                                                      position);
    registry->emplaceComponent<rtype::games::rtype::client::Rectangle>(
        entity, rectangle);
    registry->emplaceComponent<rtype::games::rtype::client::Button<Args...>>(
        entity, onClick);
    registry->emplaceComponent<rtype::games::rtype::client::UserEvent>(entity);
    registry->emplaceComponent<rtype::games::rtype::client::ButtonTag>(entity);
    return entity;
}

std::vector<ECS::Entity> createBackground(
    const std::shared_ptr<ECS::Registry>& registry,
    const std::shared_ptr<AssetManager>& assetManager,
    const std::string& PageName);

ECS::Entity createPlayer(const std::shared_ptr<ECS::Registry>& registry,
                         const std::shared_ptr<AssetManager>& assetsManager,
                         sf::Vector2i scale, bool isControllable);

std::vector<ECS::Entity> createSection(
    const std::shared_ptr<ECS::Registry>& registry,
    const std::shared_ptr<AssetManager>& assets, const std::string& title,
    float x, float y, float width, float height);

ECS::Entity createStaticText(const std::shared_ptr<ECS::Registry>& registry,
                             const std::shared_ptr<AssetManager>& assets,
                             const std::string& text, float x, float y);

#endif  // SRC_CLIENT_GRAPHIC_ENTITYFACTORY_ENTITYFACTORY_HPP_
