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
#include <string_view>
#include <vector>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

#include "AssetManager/AssetManager.hpp"
#include "Components/ButtonComponent.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"
#include "Components/RectangleComponent.hpp"
#include "Components/SoundComponent.hpp"
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
    const rtype::games::rtype::shared::TransformComponent& position,
    const rtype::games::rtype::client::Rectangle& rectangle,
    std::shared_ptr<AssetManager> assetsManager,
    std::function<void(Args...)> onClick) {
    auto entity = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::client::Text>(entity, text);
    registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(entity,
                                                                      position);
    registry->emplaceComponent<rtype::games::rtype::client::Rectangle>(
        entity, rectangle);
    registry->emplaceComponent<rtype::games::rtype::client::Button<Args...>>(
        entity, onClick);
    registry->emplaceComponent<rtype::games::rtype::client::UserEvent>(entity);
    registry->emplaceComponent<rtype::games::rtype::client::ButtonTag>(entity);
    if (assetsManager) {
        registry->emplaceComponent<
            rtype::games::rtype::client::ButtonSoundComponent>(
            entity, assetsManager->soundManager->get("hover_button"),
            assetsManager->soundManager->get("click_button"));
    }
    return entity;
}

/**
 * @brief Create a text input field entity
 * @param registry ECS registry
 * @param assetManager Asset manager for fonts
 * @param bounds Bounding rectangle (position and size) of the input field
 * @param placeholder Placeholder text
 * @param initialValue Initial text value
 * @param maxLength Maximum number of characters (0 = unlimited)
 * @param isNumericOnly Only allow numeric input
 * @return Created entity
 */
ECS::Entity createTextInput(std::shared_ptr<ECS::Registry> registry,
                            std::shared_ptr<AssetManager> assetManager,
                            const sf::FloatRect& bounds,
                            std::string_view placeholder = "",
                            std::string_view initialValue = "",
                            std::size_t maxLength = 0,
                            bool isNumericOnly = false);

std::vector<ECS::Entity> createBackground(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> assetManager, std::string_view PageName);

ECS::Entity createPlayer(std::shared_ptr<ECS::Registry> registry,
                         std::shared_ptr<AssetManager> assetManager,
                         sf::Vector2i scale = sf::Vector2i(1, 1),
                         bool isControllable = false);

std::vector<ECS::Entity> createSection(std::shared_ptr<ECS::Registry> registry,
                                       std::shared_ptr<AssetManager> assets,
                                       std::string_view title,
                                       const sf::FloatRect& bounds);

ECS::Entity createStaticText(std::shared_ptr<ECS::Registry> registry,
                             std::shared_ptr<AssetManager> assets,
                             std::string_view title, std::string_view fontId,
                             const sf::Vector2f& position, float size);
};  // namespace EntityFactory

#endif  // SRC_CLIENT_GRAPHIC_ENTITYFACTORY_ENTITYFACTORY_HPP_
