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

// Note: Passage de l'entité, plus besoin de try-catch
void RenderSystem::_renderImages(ECS::Registry& registry, ECS::Entity entity) {
    // Vérification préalable
    if (!registry.hasComponent<Image>(entity) || !registry.hasComponent<rs::Position>(entity))
        return;

    // Utilisation de auto& (Référence) pour éviter la copie inutile
    auto& img = registry.getComponent<Image>(entity);
    const auto& pos = registry.getComponent<rs::Position>(entity);

    img.sprite.setPosition(
        {static_cast<float>(pos.x), static_cast<float>(pos.y)});

    if (registry.hasComponent<Size>(entity)) {
        const auto& size = registry.getComponent<Size>(entity);
        img.sprite.setScale(sf::Vector2f({size.x, size.y}));
    }
    if (registry.hasComponent<TextureRect>(entity)) {
        const auto& texture = registry.getComponent<TextureRect>(entity);
        img.sprite.setTextureRect(texture.rect);
    }

    _target->draw(img.sprite);
}

void RenderSystem::_renderRectangles(ECS::Registry& registry, ECS::Entity entity) {
    // Vérifications : doit avoir Rectangle, Position, et NE PAS être un Button
    if (!registry.hasComponent<Rectangle>(entity) ||
        !registry.hasComponent<rs::Position>(entity) ||
        registry.hasComponent<ButtonTag>(entity))
        return;

    auto& rectData = registry.getComponent<Rectangle>(entity); // Référence !
    const auto& pos = registry.getComponent<rs::Position>(entity);

    rectData.rectangle.setPosition(
        {static_cast<float>(pos.x), static_cast<float>(pos.y)});
    rectData.rectangle.setSize(
        sf::Vector2f(rectData.size.first, rectData.size.second));
    rectData.rectangle.setOutlineThickness(rectData.outlineThickness);
    rectData.rectangle.setOutlineColor(rectData.outlineColor);
    rectData.rectangle.setFillColor(rectData.currentColor);

    _target->draw(rectData.rectangle);
}

void RenderSystem::_renderButtons(ECS::Registry& registry, ECS::Entity entity) {
    // Vérifications strictes pour le bouton
    if (!registry.hasComponent<ButtonTag>(entity) ||
        !registry.hasComponent<Rectangle>(entity) ||
        !registry.hasComponent<Text>(entity) ||
        !registry.hasComponent<rs::Position>(entity))
        return;

    auto& rectData = registry.getComponent<Rectangle>(entity);
    auto& textData = registry.getComponent<Text>(entity);
    const auto& pos = registry.getComponent<rs::Position>(entity);

    // Setup Rectangle
    rectData.rectangle.setPosition(
        {static_cast<float>(pos.x), static_cast<float>(pos.y)});
    rectData.rectangle.setSize(
        sf::Vector2f(rectData.size.first, rectData.size.second));
    rectData.rectangle.setOutlineThickness(rectData.outlineThickness);
    rectData.rectangle.setOutlineColor(rectData.outlineColor);
    rectData.rectangle.setFillColor(rectData.currentColor);

    // Setup Text
    textData.text.setCharacterSize(textData.size);
    textData.text.setFillColor(textData.color);
    textData.text.setString(textData.textContent);

    // Centering Logic
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

    _target->draw(rectData.rectangle);
    _target->draw(textData.text);
}

void RenderSystem::_renderStaticText(ECS::Registry& registry, ECS::Entity entity) {
    if (!registry.hasComponent<StaticTextTag>(entity) ||
        !registry.hasComponent<Text>(entity) ||
        !registry.hasComponent<rs::Position>(entity))
        return;

    auto& textData = registry.getComponent<Text>(entity);
    const auto& pos = registry.getComponent<rs::Position>(entity);

    textData.text.setPosition(
        {static_cast<float>(pos.x), static_cast<float>(pos.y)});
    textData.text.setCharacterSize(textData.size);
    textData.text.setFillColor(textData.color);
    textData.text.setString(textData.textContent);

    _target->draw(textData.text);
}

void RenderSystem::_renderTextInputs(ECS::Registry& registry, ECS::Entity entity) {
    if (!registry.hasComponent<TextInputTag>(entity) ||
        !registry.hasComponent<TextInput>(entity) ||
        !registry.hasComponent<rs::Position>(entity))
        return;

    auto& textInput = registry.getComponent<TextInput>(entity);
    const auto& pos = registry.getComponent<rs::Position>(entity);

    textInput.background.setPosition({static_cast<float>(pos.x), static_cast<float>(pos.y)});
    textInput.text.setPosition({static_cast<float>(pos.x) + 10.f, static_cast<float>(pos.y) + 5.f});

    _target->draw(textInput.background);
    _target->draw(textInput.text);
}

void RenderSystem::update(ECS::Registry& registry, float /*dt*/) {
    std::vector<ECS::Entity> sortedEntities;

    auto view = registry.view<ZIndex>();

    view.each([&sortedEntities](auto entt, auto&) {
        sortedEntities.push_back(entt);
    });

    // 2. Trier par profondeur
    std::sort(sortedEntities.begin(), sortedEntities.end(),
              [&registry](ECS::Entity a, ECS::Entity b) {
                  // Pas besoin de try/catch ici car on sait qu'elles viennent de la view<ZIndex>
                  const auto& za = registry.getComponent<ZIndex>(a);
                  const auto& zb = registry.getComponent<ZIndex>(b);
                  return za.depth < zb.depth;
              });

    // 3. Rendu séquentiel
    for (auto entity : sortedEntities) {
        if (isEntityHidden(registry, entity)) continue;

        // On appelle les sous-fonctions. Chacune vérifie avec hasComponent
        // si elle doit dessiner quelque chose.

        // Note: L'ordre ici n'a plus d'importance pour la superposition
        // car c'est le tri du vecteur `sortedEntities` qui gère ça.
        // On vérifie juste les types.

        _renderImages(registry, entity);

        // Note: _renderRectangles contient déjà le check "n'est pas un bouton"
        _renderRectangles(registry, entity);

        _renderButtons(registry, entity);
        _renderStaticText(registry, entity);
        _renderTextInputs(registry, entity);
    }
}

}  // namespace rtype::games::rtype::client