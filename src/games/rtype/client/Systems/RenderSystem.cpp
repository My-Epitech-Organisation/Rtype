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
#include "Components/PositionComponent.hpp"
#include "ecs/ECS.hpp"

namespace rtype::games::rtype::client {

RenderSystem::RenderSystem(std::shared_ptr<sf::RenderWindow> window)
    : ::rtype::engine::ASystem("RenderSystem"), _window(std::move(window)) {}

void RenderSystem::update(ECS::Registry& registry, float dt) {
    registry
        .view<::rtype::games::rtype::client::Image,
              ::rtype::games::rtype::shared::Position>()
        .each([&registry, this](auto entt, auto& img, auto& pos) {
            img.sprite.setPosition(
                {static_cast<float>(pos.x), static_cast<float>(pos.y)});
            try {
                auto& size =
                    registry.getComponent<::rtype::games::rtype::client::Size>(
                        entt);
                img.sprite.setScale(sf::Vector2f({size.x, size.y}));
            } catch (...) {
            }
            try {
                auto& texture =
                    registry
                        .getComponent<::rtype::games::rtype::client::TextureRect>(
                            entt);
                img.sprite.setTextureRect(texture.rect);
            } catch (...) {
            }
            this->_window->draw(img.sprite);
        });

    registry
        .view<::rtype::games::rtype::client::Rectangle,
              ::rtype::games::rtype::shared::Position>()
        .each([this, &registry](auto entt, auto& rectData, auto& pos) {
            if (registry.hasComponent<::rtype::games::rtype::client::ButtonTag>(
                    entt))
                return;

            rectData.rectangle.setPosition(
                {static_cast<float>(pos.x), static_cast<float>(pos.y)});
            rectData.rectangle.setSize(
                sf::Vector2f(rectData.size.first, rectData.size.second));
            rectData.rectangle.setOutlineThickness(rectData.outlineThickness);
            rectData.rectangle.setOutlineColor(rectData.outlineColor);
            rectData.rectangle.setFillColor(rectData.currentColor);

            this->_window->draw(rectData.rectangle);
        });

    registry
        .view<::rtype::games::rtype::client::Rectangle,
              ::rtype::games::rtype::client::Text,
              ::rtype::games::rtype::shared::Position,
              ::rtype::games::rtype::client::ButtonTag>()
        .each([this](auto _, auto& rectData, auto& textData, auto& pos,
                     auto __) {
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

            this->_window->draw(rectData.rectangle);
            this->_window->draw(textData.text);
        });

    registry
        .view<::rtype::games::rtype::client::Text,
              ::rtype::games::rtype::shared::Position,
              ::rtype::games::rtype::client::StaticTextTag>()
        .each([this](auto _, auto& textData, auto& pos, auto __) {
            textData.text.setPosition(
                {static_cast<float>(pos.x), static_cast<float>(pos.y)});
            textData.text.setCharacterSize(textData.size);
            textData.text.setFillColor(textData.color);
            textData.text.setString(textData.textContent);

            this->_window->draw(textData.text);
        });
}
}  // namespace rtype::games::rtype::client
