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

namespace EntityFactory {
template <typename... Args>
static ECS::Entity createButton(const std::shared_ptr<ECS::Registry>& registry,
                                const Text& text, const Position& position,
                                const Rectangle& rectangle,
                                std::function<void(Args...)> onClick) {
    auto entity = registry->spawnEntity();
    registry->emplaceComponent<Text>(entity, text);
    registry->emplaceComponent<Position>(entity, position);
    registry->emplaceComponent<Rectangle>(entity, rectangle);
    registry->emplaceComponent<Button<Args...>>(entity, onClick);
    registry->emplaceComponent<UserEvent>(entity);
    registry->emplaceComponent<ButtonTag>(entity);
    return entity;
}

std::vector<ECS::Entity> createBackground(
    const std::shared_ptr<ECS::Registry>& registry,
    const std::shared_ptr<AssetManager>& assetManager,
    const std::string& PageName);
};  // namespace EntityFactory

#endif  // SRC_CLIENT_GRAPHIC_ENTITYFACTORY_ENTITYFACTORY_HPP_
