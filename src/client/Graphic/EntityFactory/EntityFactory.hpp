/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** EntityFactory.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_ENTITYFACTORY_ENTITYFACTORY_HPP_
#define SRC_CLIENT_GRAPHIC_ENTITYFACTORY_ENTITYFACTORY_HPP_

#include <memory>
#include <string>
#include <vector>

#include <SFML/System/Vector2.hpp>

#include "AssetManager/AssetManager.hpp"
#include "Components/ButtonComponent.hpp"
#include "Components/PositionComponent.hpp"
#include "Components/RectangleComponent.hpp"
#include "Components/TagComponent.hpp"
#include "Components/TextComponent.hpp"
#include "Components/TextInputComponent.hpp"
#include "Components/UserEventComponent.hpp"
#include "ECS.hpp"

namespace EntityFactory {
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

/**
 * @brief Create a text input field entity
 * @param registry ECS registry
 * @param assetManager Asset manager for fonts
 * @param position Position of the input field
 * @param width Width of the input field
 * @param height Height of the input field
 * @param placeholder Placeholder text
 * @param initialValue Initial text value
 * @param maxLength Maximum number of characters (0 = unlimited)
 * @param isNumericOnly Only allow numeric input
 * @return Created entity
 */
ECS::Entity createTextInput(std::shared_ptr<ECS::Registry> registry,
                            std::shared_ptr<AssetManager> assetManager,
                            float x, float y, float width, float height,
                            const std::string& placeholder = "",
                            const std::string& initialValue = "",
                            std::size_t maxLength = 0,
                            bool isNumericOnly = false);

std::vector<ECS::Entity> createBackground(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> assetManager, const std::string& PageName);

ECS::Entity createPlayer(std::shared_ptr<ECS::Registry> registry,
                         std::shared_ptr<AssetManager> assetManager,
                         sf::Vector2i scale = sf::Vector2i(1, 1),
                         bool isControllable = false);

std::vector<ECS::Entity> createSection(std::shared_ptr<ECS::Registry> registry,
                                       std::shared_ptr<AssetManager> assets,
                                       const std::string& title, float x,
                                       float y, float width, float height);

ECS::Entity createStaticText(std::shared_ptr<ECS::Registry> registry,
                             std::shared_ptr<AssetManager> assets,
                             const std::string& title,
                             const std::string& fontId, float posX, float posY,
                             float size);
};  // namespace EntityFactory

#endif  // SRC_CLIENT_GRAPHIC_ENTITYFACTORY_ENTITYFACTORY_HPP_
