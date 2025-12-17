/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RenderSystem.cpp
*/

#include "RenderSystem.hpp"

#include <algorithm>
#include <utility>
#include <vector>

#include "../AllComponents.hpp"
#include "../shared/Components/Tags.hpp"
#include "ECS.hpp"

// Use shorter aliases for readability
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

void RenderSystem::_renderImages(ECS::Registry& registry) {
    std::vector<ECS::Entity> drawableEntities;
    registry.view<Image, rs::TransformComponent, ZIndex>().each(
        [&drawableEntities](auto entt, auto, auto, auto) {
            drawableEntities.push_back(entt);
        });
    std::sort(drawableEntities.begin(), drawableEntities.end(),
              [&registry](ECS::Entity a, ECS::Entity b) {
                  auto& za = registry.getComponent<ZIndex>(a);
                  auto& zb = registry.getComponent<ZIndex>(b);
                  return za.depth < zb.depth;
              });
    for (auto entt : drawableEntities) {
        if (isEntityHidden(registry, entt)) continue;
        if (registry.hasComponent<rs::DestroyTag>(entt)) continue;

        auto& img = registry.getComponent<Image>(entt);
        const auto& pos = registry.getComponent<rs::TransformComponent>(entt);

        if (registry.hasComponent<Size>(entt)) {
            const auto& size = registry.getComponent<Size>(entt);
            img.sprite.setScale(sf::Vector2f({size.x, size.y}));
        }
        if (registry.hasComponent<TextureRect>(entt)) {
            const auto& texture = registry.getComponent<TextureRect>(entt);
            img.sprite.setTextureRect(texture.rect);
        }

        if (registry.hasComponent<GameTag>(entt)) {
            auto bounds = img.sprite.getGlobalBounds();
            float offsetX = pos.x - bounds.size.x / 2.0f;
            float offsetY = pos.y - bounds.size.y / 2.0f;
            img.sprite.setPosition({offsetX, offsetY});
        } else {
            img.sprite.setPosition(
                {static_cast<float>(pos.x), static_cast<float>(pos.y)});
        }

        _target->draw(img.sprite);
    }
}

void RenderSystem::_renderRectangles(ECS::Registry& registry) {
    registry.view<Rectangle, rs::TransformComponent>().each(
        [this, &registry](auto entt, auto& rectData, auto& pos) {
            if (registry.hasComponent<ButtonTag>(entt)) return;
            if (registry.hasComponent<HudTag>(entt)) return;
            if (isEntityHidden(registry, entt)) return;
            if (registry.hasComponent<rs::DestroyTag>(entt)) return;

            rectData.rectangle.setSize(
                sf::Vector2f(rectData.size.first, rectData.size.second));
            rectData.rectangle.setOutlineThickness(rectData.outlineThickness);
            rectData.rectangle.setOutlineColor(rectData.outlineColor);
            rectData.rectangle.setFillColor(rectData.currentColor);

            if (registry.hasComponent<GameTag>(entt)) {
                float offsetX = pos.x - rectData.size.first / 2.0f;
                float offsetY = pos.y - rectData.size.second / 2.0f;
                rectData.rectangle.setPosition({offsetX, offsetY});
            } else {
                rectData.rectangle.setPosition(
                    {static_cast<float>(pos.x), static_cast<float>(pos.y)});
            }

            _target->draw(rectData.rectangle);
        });
}

void RenderSystem::_renderHudRectangles(ECS::Registry& registry) {
    const sf::View savedView = _target->getView();
    _target->setView(_target->getDefaultView());

    registry.view<Rectangle, rs::TransformComponent, HudTag>().each(
        [this, &registry](auto entt, auto& rectData, auto& pos, auto /*tag*/) {
            if (isEntityHidden(registry, entt)) return;
            if (registry.hasComponent<rs::DestroyTag>(entt)) return;

            rectData.rectangle.setPosition(
                {static_cast<float>(pos.x), static_cast<float>(pos.y)});
            rectData.rectangle.setSize(
                sf::Vector2f(rectData.size.first, rectData.size.second));
            rectData.rectangle.setOutlineThickness(rectData.outlineThickness);
            rectData.rectangle.setOutlineColor(rectData.outlineColor);
            rectData.rectangle.setFillColor(rectData.currentColor);

            _target->draw(rectData.rectangle);
        });

    _target->setView(savedView);
}

void RenderSystem::_renderButtons(ECS::Registry& registry) {
    registry.view<Rectangle, Text, rs::TransformComponent, ButtonTag>().each(
        [this, &registry](auto entt, auto& rectData, auto& textData, auto& pos,
                          auto /*tag*/) {
            if (isEntityHidden(registry, entt)) return;
            if (registry.hasComponent<rs::DestroyTag>(entt)) return;

            rectData.rectangle.setPosition(
                {static_cast<float>(pos.x), static_cast<float>(pos.y)});
            rectData.rectangle.setSize(
                sf::Vector2f(rectData.size.first, rectData.size.second));
            rectData.rectangle.setOutlineThickness(rectData.outlineThickness);
            rectData.rectangle.setOutlineColor(rectData.outlineColor);
            rectData.rectangle.setFillColor(rectData.currentColor);

            float rectX = rectData.rectangle.getPosition().x;
            float rectY = rectData.rectangle.getPosition().y;
            float rectWidth = rectData.size.first;
            float rectHeight = rectData.size.second;

            textData.text.setCharacterSize(textData.size);
            textData.text.setFillColor(textData.color);
            textData.text.setString(textData.textContent);

            sf::FloatRect textBounds = textData.text.getLocalBounds();
            float textWidth = textBounds.size.x;
            float textHeight = textBounds.size.y;
            float textTop = textBounds.position.y;

            float centerX = rectX + (rectWidth / 2.0f) - (textWidth / 2.0f);
            float centerY =
                rectY + (rectHeight / 2.0f) - (textHeight / 2.0f) - textTop;

            textData.text.setPosition({centerX, centerY});

            _target->draw(rectData.rectangle);
            _target->draw(textData.text);
        });
}

void RenderSystem::_renderStaticText(ECS::Registry& registry) {
    registry.view<Text, rs::TransformComponent, StaticTextTag>().each(
        [this, &registry](auto entt, auto& textData, auto& pos, auto /*tag*/) {
            if (isEntityHidden(registry, entt)) return;
            if (registry.hasComponent<rs::DestroyTag>(entt)) return;

            textData.text.setPosition(
                {static_cast<float>(pos.x), static_cast<float>(pos.y)});
            textData.text.setCharacterSize(textData.size);
            textData.text.setFillColor(textData.color);
            textData.text.setString(textData.textContent);

            _target->draw(textData.text);
        });
}

void RenderSystem::update(ECS::Registry& registry, float /*dt*/) {
    _renderImages(registry);
    _renderRectangles(registry);
    _renderButtons(registry);
    _renderStaticText(registry);
    _renderHudRectangles(registry);
}

}  // namespace rtype::games::rtype::client
