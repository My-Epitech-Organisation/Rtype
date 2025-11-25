/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Event definitions for Event Bus PoC
*/

#ifndef EVENTS_HPP
    #define EVENTS_HPP

    #include "ECS.hpp"

namespace PoC {

    /**
     * @brief Base event class for type safety
     */
    struct Event {
        virtual ~Event() = default;
    };

    /**
     * @brief Event triggered when two entities collide
     */
    struct CollisionEvent : public Event {
        ECS::Entity entityA;
        ECS::Entity entityB;
        float posX;
        float posY;

        CollisionEvent(ECS::Entity a, ECS::Entity b, float x, float y)
            : entityA(a), entityB(b), posX(x), posY(y) {}
    };

    /**
     * @brief Event for entity creation (could be used for other systems)
     */
    struct EntitySpawnedEvent : public Event {
        ECS::Entity entity;

        EntitySpawnedEvent(ECS::Entity e) : entity(e) {}
    };

    /**
     * @brief Event for entity destruction
     */
    struct EntityDestroyedEvent : public Event {
        ECS::Entity entity;

        EntityDestroyedEvent(ECS::Entity e) : entity(e) {}
    };

} // namespace PoC

#endif // EVENTS_HPP
