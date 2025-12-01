/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** EntityFactory.hpp
*/

#ifndef R_TYPE_ENTITYFACTORY_HPP
#define R_TYPE_ENTITYFACTORY_HPP
#include <SFML/System/Vector2.hpp>
#include "ecs/ECS.hpp"
#include "Components/Graphic/TextComponent.hpp"
#include "Components/Common/PositionComponent.hpp"
#include "Components/Graphic/ButtonComponent.hpp"
#include "Components/Graphic/RectangleComponent.hpp"
#include "Components/Graphic/UserEventComponent.hpp"


namespace EntityFactory {
    template <typename... Args>
    static ECS::Entity createButton(
        ECS::Registry& registry,
        const Text &text,
        const Position &position,
        const Rectangle &rectangle,
        std::function<void(Args...)> onClick
        ) {
            auto entity = registry.spawnEntity();
            registry.emplaceComponent<Text>(entity, text);
            registry.emplaceComponent<Position>(entity, position);
            registry.emplaceComponent<Rectangle>(entity, rectangle);
            registry.emplaceComponent<Button<Args...>>(entity, onClick);
            registry.emplaceComponent<UserEvent>(entity);
            registry.emplaceComponent<ButtonTag>(entity);
            return entity;
        };
};

#endif //R_TYPE_ENTITYFACTORY_HPP