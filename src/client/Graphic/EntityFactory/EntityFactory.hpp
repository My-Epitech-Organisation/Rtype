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

#include "../../../include/rtype/display/DisplayTypes.hpp"
#include "AssetManager/AssetManager.hpp"
#include "Components/ButtonComponent.hpp"
#include "Components/RectangleComponent.hpp"
#include "Components/SoundComponent.hpp"
#include "Components/TagComponent.hpp"
#include "Components/TextComponent.hpp"
#include "Components/TextInputComponent.hpp"
#include "Components/UserEventComponent.hpp"
#include "Components/ZIndexComponent.hpp"
#include "ECS.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"
#include "lib/background/IBackground.hpp"

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
    registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        entity, position);
    registry->emplaceComponent<rtype::games::rtype::client::Rectangle>(
        entity, rectangle);
    registry->emplaceComponent<rtype::games::rtype::client::Button<Args...> >(
        entity, onClick);
    registry->emplaceComponent<rtype::games::rtype::client::UserEvent>(entity);
    registry->emplaceComponent<rtype::games::rtype::client::ButtonTag>(entity);
    registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(entity, 1);
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
 * @param position Position of the input field
 * @param size Size of the input field
 * @param placeholder Placeholder text
 * @param initialValue Initial text value
 * @param maxLength Maximum number of characters (0 = unlimited)
 * @param isNumericOnly Only allow numeric input
 * @return Created entity
 */
ECS::Entity createTextInput(std::shared_ptr<ECS::Registry> registry,
                            std::shared_ptr<AssetManager> assetManager,
                            const ::rtype::display::Vector2f& position,
                            const ::rtype::display::Vector2f& size,
                            std::string_view placeholder = "",
                            std::string_view initialValue = "",
                            std::size_t maxLength = 0,
                            bool isNumericOnly = false);

std::vector<ECS::Entity> createBackground(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> assetManager, std::string_view PageName,
    std::unique_ptr<IBackground> backgroundLib);

ECS::Entity createLobbyPlayer(std::shared_ptr<ECS::Registry> registry,
                              std::shared_ptr<AssetManager> assetManager,
                              ::rtype::display::Vector2<float> positon,
                              ::rtype::display::Vector2<int> scale = {1, 1},
                              bool isControllable = false);

/**
 * @brief Create a rectangle entity
 * @param registry ECS registry
 * @param size a Vector2i defining the size of the rectangle
 * @param fill Fill color of the rectangle
 * @param position Position of the rectangle
 * @return Created entity
 **/

ECS::Entity createRectangle(
    std::shared_ptr<ECS::Registry> registry,
    ::rtype::display::Vector2i size = {1, 1},
    ::rtype::display::Color fill = ::rtype::display::Color::White(),
    ::rtype::display::Vector2f position = {0, 0});

std::vector<ECS::Entity> createSection(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> assets, std::string_view title,
    const ::rtype::display::Rect<float>& bounds, int ZindexRect = 0);

ECS::Entity createStaticText(std::shared_ptr<ECS::Registry> registry,
                             std::shared_ptr<AssetManager> assets,
                             std::string_view title, std::string_view fontId,
                             const ::rtype::display::Vector2f& position,
                             float size);

/**
 * @brief Create a static image/sprite entity for UI display
 * @param registry ECS registry
 * @param textureId ID of the texture (must be pre-loaded in TextureManager)
 * @param position Position of the image
 * @param scale Scale of the image (1.0 = original size)
 * @return Created entity
 */
ECS::Entity createStaticImage(std::shared_ptr<ECS::Registry> registry,
                              std::string_view textureId,
                              const ::rtype::display::Vector2f& position,
                              float scale = 1.0f);
};  // namespace EntityFactory

#endif  // SRC_CLIENT_GRAPHIC_ENTITYFACTORY_ENTITYFACTORY_HPP_
