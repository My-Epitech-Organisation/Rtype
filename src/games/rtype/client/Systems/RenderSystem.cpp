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
RenderSystem::RenderSystem(std::shared_ptr<sf::RenderTarget> target)
    : ::rtype::engine::ASystem("RenderSystem"), _target(std::move(target)) {}

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

    if (registry.hasComponent<Size>(entity)) {
        const auto& size = registry.getComponent<Size>(entity);
        img.sprite.setScale(sf::Vector2f({size.x, size.y}));
    }
    if (registry.hasComponent<TextureRect>(entity)) {
        const auto& texture = registry.getComponent<TextureRect>(entity);
        img.sprite.setTextureRect(texture.rect);
    }

    if (registry.hasComponent<GameTag>(entity)) {
        auto bounds = img.sprite.getGlobalBounds();
        float offsetX = pos.x - bounds.size.x / 2.0f;
        float offsetY = pos.y - bounds.size.y / 2.0f;
        img.sprite.setPosition({offsetX, offsetY});
    } else {
        img.sprite.setPosition(
            {static_cast<float>(pos.x), static_cast<float>(pos.y)});
    }

    this->_target->draw(img.sprite);
}

void RenderSystem::_renderRectangles(ECS::Registry& registry,
                                     ECS::Entity entity) {
    if (!registry.hasComponent<Rectangle>(entity) ||
        !registry.hasComponent<rs::TransformComponent>(entity) ||
        registry.hasComponent<ButtonTag>(entity) ||
        registry.hasComponent<HudTag>(entity) ||
        registry.hasComponent<rs::DestroyTag>(entity))
        return;
    auto& rectData = registry.getComponent<Rectangle>(entity);
    const auto& pos = registry.getComponent<rs::TransformComponent>(entity);

    rectData.rectangle.setSize(
        sf::Vector2f(rectData.size.first, rectData.size.second));
    rectData.rectangle.setOutlineThickness(rectData.outlineThickness);
    rectData.rectangle.setOutlineColor(rectData.outlineColor);
    rectData.rectangle.setFillColor(rectData.currentColor);

    if (registry.hasComponent<GameTag>(entity)) {
        float offsetX = pos.x - rectData.size.first / 2.0f;
        float offsetY = pos.y - rectData.size.second / 2.0f;
        rectData.rectangle.setPosition({offsetX, offsetY});
    } else {
        rectData.rectangle.setPosition(
            {static_cast<float>(pos.x), static_cast<float>(pos.y)});
    }

    _target->draw(rectData.rectangle);
}

void RenderSystem::_renderHudRectangles(ECS::Registry& registry,
                                        ECS::Entity entity) {
    const sf::View savedView = _target->getView();
    _target->setView(_target->getDefaultView());
    if (!registry.hasComponent<Rectangle>(entity) ||
        !registry.hasComponent<rs::TransformComponent>(entity) ||
        !registry.hasComponent<HudTag>(entity) ||
        registry.hasComponent<rs::DestroyTag>(entity))
        return;

    auto& rectData = registry.getComponent<Rectangle>(entity);
    const auto& pos = registry.getComponent<rs::TransformComponent>(entity);

    rectData.rectangle.setPosition(
        {static_cast<float>(pos.x), static_cast<float>(pos.y)});
    rectData.rectangle.setSize(
        sf::Vector2f(rectData.size.first, rectData.size.second));
    rectData.rectangle.setOutlineThickness(rectData.outlineThickness);
    rectData.rectangle.setOutlineColor(rectData.outlineColor);
    rectData.rectangle.setFillColor(rectData.currentColor);

    this->_target->draw(rectData.rectangle);
}

void RenderSystem::_renderButtons(ECS::Registry& registry, ECS::Entity entity) {
    if (!registry.hasComponent<ButtonTag>(entity) ||
        !registry.hasComponent<Rectangle>(entity) ||
        !registry.hasComponent<Text>(entity) ||
        !registry.hasComponent<rs::TransformComponent>(entity) ||
        registry.hasComponent<rs::DestroyTag>(entity))
        return;

    auto& rectData = registry.getComponent<Rectangle>(entity);
    auto& textData = registry.getComponent<Text>(entity);
    const auto& pos = registry.getComponent<rs::TransformComponent>(entity);

    rectData.rectangle.setPosition(
        {static_cast<float>(pos.x), static_cast<float>(pos.y)});
    rectData.rectangle.setSize(
        sf::Vector2f(rectData.size.first, rectData.size.second));
    rectData.rectangle.setOutlineThickness(rectData.outlineThickness);
    rectData.rectangle.setOutlineColor(rectData.outlineColor);
    rectData.rectangle.setFillColor(rectData.currentColor);

    textData.text.setCharacterSize(textData.size);
    textData.text.setFillColor(textData.color);
    textData.text.setString(textData.textContent);

    float rectX = rectData.rectangle.getPosition().x;
    float rectY = rectData.rectangle.getPosition().y;
    float rectWidth = rectData.size.first;
    float rectHeight = rectData.size.second;

    sf::FloatRect textBounds = textData.text.getLocalBounds();
    float textWidth = textBounds.size.x;
    float textHeight = textBounds.size.y;
    float textTop = textBounds.position.y;

    float centerX = rectX + (rectWidth / 2.0f) - (textWidth / 2.0f);
    float centerY = rectY + (rectHeight / 2.0f) - (textHeight / 2.0f) - textTop;

    textData.text.setPosition({centerX, centerY});

    this->_target->draw(rectData.rectangle);
    this->_target->draw(textData.text);
}

void RenderSystem::_renderStaticText(ECS::Registry& registry,
                                     ECS::Entity entity) {
    if (!registry.hasComponent<StaticTextTag>(entity) ||
        !registry.hasComponent<Text>(entity) ||
        !registry.hasComponent<rs::TransformComponent>(entity) ||
        registry.hasComponent<rs::DestroyTag>(entity))
        return;

    auto& textData = registry.getComponent<Text>(entity);
    const auto& pos = registry.getComponent<rs::TransformComponent>(entity);

    textData.text.setPosition(
        {static_cast<float>(pos.x), static_cast<float>(pos.y)});
    textData.text.setCharacterSize(textData.size);
    textData.text.setFillColor(textData.color);
    textData.text.setString(textData.textContent);

    this->_target->draw(textData.text);
}

void RenderSystem::update(ECS::Registry& registry, float /*dt*/) {
    std::vector<ECS::Entity> sortedEntities;

    auto view = registry.view<ZIndex>();

    view.each([&sortedEntities](auto entt, auto&) {
        sortedEntities.push_back(entt);
    });

    std::sort(sortedEntities.begin(), sortedEntities.end(),
              [&registry](ECS::Entity a, ECS::Entity b) {
                  const auto& za = registry.getComponent<ZIndex>(a);
                  const auto& zb = registry.getComponent<ZIndex>(b);
                  return za.depth < zb.depth;
              });

    for (auto entity : sortedEntities) {
        if (isEntityHidden(registry, entity)) continue;

        if (registry.hasComponent<Image>(entity))
            this->_renderImages(registry, entity);
        if (registry.hasComponent<Rectangle>(entity))
            this->_renderRectangles(registry, entity);
        if (registry.hasComponent<ButtonTag>(entity))
            this->_renderButtons(registry, entity);
        if (registry.hasComponent<StaticTextTag>(entity))
            this->_renderStaticText(registry, entity);
        if (registry.hasComponent<HudTag>(entity))
            this->_renderHudRectangles(registry, entity);
    }
}

}  // namespace rtype::games::rtype::client
