/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RenderSystem.cpp
*/

#include "RenderSystem.hpp"

#include "../Components/ButtonComponent.hpp"
#include "../Components/ImageComponent.hpp"
#include "../Components/RectangleComponent.hpp"
#include "../Components/SizeComponent.hpp"
#include "../Components/TagComponent.hpp"
#include "../Components/TextComponent.hpp"
#include "../Components/TextureRectComponent.hpp"
#include "../Components/ZIndexComponent.hpp"
#include "../Drawable/Image.hpp"
#include "Components/PositionComponent.hpp"
#include "ecs/ECS.hpp"
#include "src/games/rtype/client/Components/HiddenComponent.hpp"

bool RenderSystem::isEntityHidden(
    const std::shared_ptr<ECS::Registry>& registry, ECS::Entity entity) {
    if (registry->hasComponent<HiddenComponent>(entity)) {
        return registry->getComponent<HiddenComponent>(entity).isHidden;
    }
    return false;
}

void RenderSystem::draw(const std::shared_ptr<ECS::Registry>& registry,
                        const std::shared_ptr<sf::RenderWindow>& window) {
    std::vector<DrawableImage> drawableImages;

    registry->view<Image, Position, ZIndex>().each(
        [&drawableImages](auto entt, auto& img, auto& pos, auto& zindex) {
            drawableImages.push_back({entt, &img, &pos, &zindex});
        });

    std::sort(drawableImages.begin(), drawableImages.end(),
              [](const DrawableImage& a, const DrawableImage& b) {
                  return a.zindex->depth < b.zindex->depth;
              });

    for (auto& drawable : drawableImages) {
        drawable.img->sprite.setPosition({static_cast<float>(drawable.pos->x),
                                          static_cast<float>(drawable.pos->y)});
        try {
            auto& size = registry->getComponent<Size>(drawable.entity);
            drawable.img->sprite.setScale(sf::Vector2f({size.x, size.y}));
        } catch (...) {
        }
        try {
            auto& texture =
                registry->getComponent<TextureRect>(drawable.entity);
            drawable.img->sprite.setTextureRect(texture.rect);
        } catch (...) {
        }
        window->draw(drawable.img->sprite);
    }

    registry->view<Rectangle, Position>().each(
        [&window, &registry](auto entt, auto& rectData, auto& pos) {
            if (registry->hasComponent<ButtonTag>(entt)) return;

            rectData.rectangle.setPosition(
                {static_cast<float>(pos.x), static_cast<float>(pos.y)});
            rectData.rectangle.setSize(
                sf::Vector2f(rectData.size.first, rectData.size.second));
            rectData.rectangle.setOutlineThickness(rectData.outlineThickness);
            rectData.rectangle.setOutlineColor(rectData.outlineColor);
            rectData.rectangle.setFillColor(rectData.currentColor);

            if (isEntityHidden(registry, entt)) return;
            window->draw(rectData.rectangle);
        });

    registry->view<Rectangle, Text, Position, ButtonTag>().each(
        [&window, &registry](auto entt, auto& rectData, auto& textData,
                             auto& pos, auto __) {
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

            sf::FloatRect textBounds = textData.text.getLocalBounds();
            float textWidth = textBounds.size.x;
            float textHeight = textBounds.size.y;
            float textTop = textBounds.position.y;

            float centerX = rectX + (rectWidth / 2.0f) - (textWidth / 2.0f);
            float centerY =
                rectY + (rectHeight / 2.0f) - (textHeight / 2.0f) - textTop;

            textData.text.setPosition({centerX, centerY});
            textData.text.setCharacterSize(textData.size);
            textData.text.setFillColor(textData.color);
            textData.text.setString(textData.textContent);

            if (isEntityHidden(registry, entt)) return;
            window->draw(rectData.rectangle);
            window->draw(textData.text);
        });

    registry->view<Text, Position, StaticTextTag>().each(
        [&window, &registry](auto entt, auto& textData, auto& pos, auto __) {
            textData.text.setPosition(
                {static_cast<float>(pos.x), static_cast<float>(pos.y)});
            textData.text.setCharacterSize(textData.size);
            textData.text.setFillColor(textData.color);
            textData.text.setString(textData.textContent);

            if (isEntityHidden(registry, entt)) return;
            window->draw(textData.text);
        });
}
