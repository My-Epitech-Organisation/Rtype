/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RenderSystem.cpp
*/

#include "RenderSystem.hpp"

#include <algorithm>
#include <vector>

#include "../AllComponents.hpp"
#include "../shared/Components/Tags.hpp"
#include "ECS.hpp"

namespace rc = ::rtype::games::rtype::client;
namespace rs = ::rtype::games::rtype::shared;

namespace rtype::games::rtype::client {
RenderSystem::RenderSystem(std::shared_ptr<::rtype::display::IDisplay> display)
    : ::rtype::engine::ASystem("RenderSystem"), _display(std::move(display)) {}

bool RenderSystem::isEntityHidden(ECS::Registry& registry, ECS::Entity entity) {
    if (registry.hasComponent<HiddenComponent>(entity)) {
        return registry.getComponent<HiddenComponent>(entity).isHidden;
    }
    return false;
}

void RenderSystem::_renderImages(ECS::Registry& registry, ECS::Entity entity) {
    if (!registry.hasComponent<Image>(entity) ||
        !registry.hasComponent<rs::TransformComponent>(entity) ||
        registry.hasComponent<rs::DestroyTag>(entity))
        return;

    auto& img = registry.getComponent<Image>(entity);
    const auto& pos = registry.getComponent<rs::TransformComponent>(entity);

    display::Vector2f scale = {1.0f, 1.0f};
    if (registry.hasComponent<Size>(entity)) {
        const auto& size = registry.getComponent<Size>(entity);
        scale = {size.x, size.y};
    }

    display::IntRect rect = {0, 0, 0, 0};
    if (registry.hasComponent<TextureRect>(entity)) {
        const auto& texture = registry.getComponent<TextureRect>(entity);
        rect = texture.rect;
    }

    display::Vector2f position = {static_cast<float>(pos.x),
                                  static_cast<float>(pos.y)};

    // Note: GameTag logic for centering might need adjustment if we don't have
    // sprite bounds here. For now, let's assume the display implementation
    // handles it or we pass it. Actually, let's just pass the position.

    this->_display->drawSprite(img.textureName, position, rect, scale,
                               img.color);
}

void RenderSystem::_renderRectangles(ECS::Registry& registry,
                                     ECS::Entity entity) {
    if (!registry.hasComponent<Rectangle>(entity) ||
        !registry.hasComponent<rs::TransformComponent>(entity) ||
        registry.hasComponent<ButtonTag>(entity) ||
        registry.hasComponent<HudTag>(entity) ||
        registry.hasComponent<rs::DestroyTag>(entity))
        return;
    auto& rectData = registry.getComponent<Rectangle>(entity);  // Référence !
    const auto& pos = registry.getComponent<rs::TransformComponent>(entity);

    display::Vector2f position = {static_cast<float>(pos.x),
                                  static_cast<float>(pos.y)};
    display::Vector2f size = {rectData.size.first, rectData.size.second};

    this->_display->drawRectangle(position, size, rectData.currentColor,
                                  rectData.outlineColor,
                                  rectData.outlineThickness);
}

void RenderSystem::_renderHudRectangles(ECS::Registry& registry,
                                        ECS::Entity entity) {
    if (!registry.hasComponent<Rectangle>(entity) ||
        !registry.hasComponent<rs::TransformComponent>(entity) ||
        !registry.hasComponent<HudTag>(entity) ||
        registry.hasComponent<rs::DestroyTag>(entity))
        return;

    auto center = this->_display->getViewCenter();
    auto viewSize = this->_display->getViewSize();
    this->_display->resetView();

    auto& rectData = registry.getComponent<Rectangle>(entity);
    const auto& pos = registry.getComponent<rs::TransformComponent>(entity);

    display::Vector2f position = {static_cast<float>(pos.x),
                                  static_cast<float>(pos.y)};
    display::Vector2f size = {rectData.size.first, rectData.size.second};

    this->_display->drawRectangle(position, size, rectData.currentColor,
                                  rectData.outlineColor,
                                  rectData.outlineThickness);

    this->_display->setView(center, viewSize);
}

void RenderSystem::_renderTextInputs(ECS::Registry& registry,
                                    ECS::Entity entity) {
    if (!registry.hasComponent<TextInput>(entity) ||
        !registry.hasComponent<rs::TransformComponent>(entity) ||
        registry.hasComponent<rs::DestroyTag>(entity))
        return;

    auto center = this->_display->getViewCenter();
    auto viewSize = this->_display->getViewSize();
    this->_display->resetView();

    auto& input = registry.getComponent<TextInput>(entity);
    const auto& pos = registry.getComponent<rs::TransformComponent>(entity);

    display::Vector2f position = {static_cast<float>(pos.x),
                                  static_cast<float>(pos.y)};
    display::Vector2f size = {input.size.x, input.size.y};

    // Draw background
    this->_display->drawRectangle(
        position, size, input.backgroundColor,
        input.isFocused ? input.focusedBorderColor : input.unfocusedBorderColor,
        3.0f);

    // Draw text or placeholder
    std::string textToDraw;
    display::Color colorToUse = input.textColor;

    if (input.content.empty() && !input.isFocused) {
        textToDraw = input.placeholder;
        colorToUse = display::Color(150, 150, 150, 255);
    } else {
        textToDraw = input.content + (input.isFocused ? "_" : "");
    }

    display::Vector2f textBounds = this->_display->getTextBounds(
        textToDraw, input.fontName, input.fontSize);
    float posX = position.x + kOffsetTextInput;
    float posY = position.y + ((size.y / 2) - (textBounds.y / 2));

    this->_display->drawText(textToDraw, input.fontName, {posX, posY},
                             input.fontSize, colorToUse);

    this->_display->setView(center, viewSize);
}

void RenderSystem::_renderButtons(ECS::Registry& registry, ECS::Entity entity) {
    if (!registry.hasComponent<ButtonTag>(entity) ||
        !registry.hasComponent<Rectangle>(entity) ||
        !registry.hasComponent<Text>(entity) ||
        !registry.hasComponent<rs::TransformComponent>(entity) ||
        registry.hasComponent<rs::DestroyTag>(entity))
        return;

    auto center = this->_display->getViewCenter();
    auto viewSize = this->_display->getViewSize();
    this->_display->resetView();

    auto& rectData = registry.getComponent<Rectangle>(entity);
    auto& textData = registry.getComponent<Text>(entity);
    const auto& pos = registry.getComponent<rs::TransformComponent>(entity);

    display::Vector2f position = {static_cast<float>(pos.x),
                                  static_cast<float>(pos.y)};
    display::Vector2f size = {rectData.size.first, rectData.size.second};

    this->_display->drawRectangle(position, size, rectData.currentColor,
                                  rectData.outlineColor,
                                  rectData.outlineThickness);

    display::Vector2f textBounds = this->_display->getTextBounds(
        textData.textContent, textData.fontName, textData.size);

    float centerX = position.x + (size.x / 2.0f) - (textBounds.x / 2.0f);
    float centerY = position.y + (size.y / 2.0f) - (textBounds.y / 2.0f);

    this->_display->drawText(textData.textContent, textData.fontName,
                             {centerX, centerY}, textData.size, textData.color);

    this->_display->setView(center, viewSize);
}

void RenderSystem::_renderStaticText(ECS::Registry& registry,
                                     ECS::Entity entity) {
    if (!registry.hasComponent<StaticTextTag>(entity) ||
        !registry.hasComponent<Text>(entity) ||
        !registry.hasComponent<rs::TransformComponent>(entity) ||
        registry.hasComponent<rs::DestroyTag>(entity))
        return;

    auto center = this->_display->getViewCenter();
    auto viewSize = this->_display->getViewSize();
    this->_display->resetView();

    auto& textData = registry.getComponent<Text>(entity);
    const auto& pos = registry.getComponent<rs::TransformComponent>(entity);

    display::Vector2f position = {static_cast<float>(pos.x),
                                  static_cast<float>(pos.y)};

    // Center the text if tag is present
    if (registry.hasComponent<CenteredTextTag>(entity)) {
        display::Vector2f textBounds = this->_display->getTextBounds(
            textData.textContent, textData.fontName, textData.size);
        position.x -= (textBounds.x / 2.0f);
        position.y -= (textBounds.y / 2.0f);
    }

    this->_display->drawText(textData.textContent, textData.fontName, position,
                             textData.size, textData.color);

    this->_display->setView(center, viewSize);
}

void RenderSystem::update(ECS::Registry& registry, float /*dt*/) {
    std::vector<ECS::Entity> sortedEntities;

    auto view = registry.view<ZIndex>();

    view.each([&sortedEntities](auto entt, auto&) {
        sortedEntities.push_back(entt);
    });

    std::sort(sortedEntities.begin(), sortedEntities.end(),
              [&registry](ECS::Entity a, ECS::Entity b) {
                  if (!registry.isAlive(a) || !registry.hasComponent<ZIndex>(a))
                      return true;
                  if (!registry.isAlive(b) || !registry.hasComponent<ZIndex>(b))
                      return false;
                  const auto& za = registry.getComponent<ZIndex>(a);
                  const auto& zb = registry.getComponent<ZIndex>(b);
                  return za.depth < zb.depth;
              });

    for (auto entity : sortedEntities) {
        if (!registry.isAlive(entity)) continue;
        if (isEntityHidden(registry, entity)) continue;

        if (registry.hasComponent<Image>(entity))
            this->_renderImages(registry, entity);
        if (registry.hasComponent<Rectangle>(entity))
            this->_renderRectangles(registry, entity);
        if (registry.hasComponent<ButtonTag>(entity))
            this->_renderButtons(registry, entity);
        if (registry.hasComponent<StaticTextTag>(entity))
            this->_renderStaticText(registry, entity);
        if (registry.hasComponent<TextInput>(entity))
            this->_renderTextInputs(registry, entity);
        if (registry.hasComponent<HudTag>(entity))
            this->_renderHudRectangles(registry, entity);
    }
}

}  // namespace rtype::games::rtype::client
