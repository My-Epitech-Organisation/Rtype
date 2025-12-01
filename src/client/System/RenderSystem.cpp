/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RenderSystem.cpp
*/

#include "RenderSystem.hpp"
#include "ecs/ECS.hpp"
#include "Components/Graphic/ImageComponent.hpp"
#include "Components/Graphic/TextComponent.hpp"
#include "Components/Common/PositionComponent.hpp"
#include "Components/Graphic/TagComponent.hpp"
#include "Components/Graphic/RectangleComponent.hpp"
#include "Components/Graphic/ButtonComponent.hpp"

void RenderSystem::draw(const std::shared_ptr<ECS::Registry> &registry, sf::RenderWindow &window)
{
    registry->view<Image, Position>().each([&window](auto _, auto& img, auto& pos) {
        img.sprite.setPosition({static_cast<float>(pos.x), static_cast<float>(pos.y)});

        window.draw(img.sprite);
    });
    registry->view<Text, Position, StaticTextTag>().each([&window](auto _, auto& textData, auto& pos, auto __) {
        textData.text.setPosition({static_cast<float>(pos.x), static_cast<float>(pos.y)});
        textData.text.setCharacterSize(textData.size);
        textData.text.setFillColor(textData.color);
        textData.text.setString(textData.textContent);

        window.draw(textData.text);
    });
    registry->view<Rectangle, Text, Position, ButtonTag>().each([&window](auto _, auto &rectData, auto &textData, auto &pos, auto __) {
        rectData.rectangle.setPosition({static_cast<float>(pos.x), static_cast<float>(pos.y)});
        rectData.rectangle.setSize(sf::Vector2f(rectData.size.first, rectData.size.second));
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
        float centerY = rectY + (rectHeight / 2.0f) - (textHeight / 2.0f) - textTop;

        textData.text.setPosition({centerX, centerY});
        textData.text.setCharacterSize(textData.size);
        textData.text.setFillColor(textData.color);
        textData.text.setString(textData.textContent);

        window.draw(rectData.rectangle);
        window.draw(textData.text);
    });
}
