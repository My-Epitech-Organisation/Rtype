/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Common event types for the R-Type game engine
*/

#pragma once

#include <cstdint>

#include "core/Entity.hpp"
#include "IEvent.hpp"

namespace rtype::engine::events {

/**
 * @brief Event fired when two entities collide
 */
struct CollisionEvent : public IEvent {
    IMPLEMENT_EVENT_TYPE()

    ECS::Entity entityA;
    ECS::Entity entityB;
    float impactForce = 0.0f;

    CollisionEvent() = default;
    CollisionEvent(ECS::Entity a, ECS::Entity b, float force = 0.0f)
        : entityA(a), entityB(b), impactForce(force) {}
};

/**
 * @brief Event fired when an entity takes damage
 */
struct DamageEvent : public IEvent {
    IMPLEMENT_EVENT_TYPE()

    ECS::Entity entity;
    int32_t damage = 0;
    int32_t healthBefore = 0;
    int32_t healthAfter = 0;
    uint32_t networkId = 0;

    DamageEvent() = default;
    DamageEvent(ECS::Entity e, int32_t dmg, int32_t before, int32_t after,
                uint32_t netId = 0)
        : entity(e),
          damage(dmg),
          healthBefore(before),
          healthAfter(after),
          networkId(netId) {}
};

/**
 * @brief Event fired when an entity is destroyed
 */
struct EntityDestroyedEvent : public IEvent {
    IMPLEMENT_EVENT_TYPE()

    ECS::Entity entity;
    uint32_t networkId = 0;
    uint8_t entityType = 0;

    EntityDestroyedEvent() = default;
    EntityDestroyedEvent(ECS::Entity e, uint32_t netId = 0, uint8_t type = 0)
        : entity(e), networkId(netId), entityType(type) {}
};

/**
 * @brief Event fired when an entity is spawned
 */
struct EntitySpawnedEvent : public IEvent {
    IMPLEMENT_EVENT_TYPE()

    ECS::Entity entity;
    uint32_t networkId = 0;
    uint8_t entityType = 0;
    float x = 0.0f;
    float y = 0.0f;

    EntitySpawnedEvent() = default;
    EntitySpawnedEvent(ECS::Entity e, uint32_t netId, uint8_t type, float posX,
                       float posY)
        : entity(e), networkId(netId), entityType(type), x(posX), y(posY) {}
};

/**
 * @brief Event fired when a player picks up a power-up
 */
struct PowerUpPickedEvent : public IEvent {
    IMPLEMENT_EVENT_TYPE()

    ECS::Entity player;
    ECS::Entity powerUp;
    uint32_t playerNetworkId = 0;
    uint8_t powerUpType = 0;

    PowerUpPickedEvent() = default;
    PowerUpPickedEvent(ECS::Entity p, ECS::Entity pu, uint32_t netId,
                       uint8_t type)
        : player(p), powerUp(pu), playerNetworkId(netId), powerUpType(type) {}
};

/**
 * @brief Event fired when network input is received
 */
struct NetworkInputEvent : public IEvent {
    IMPLEMENT_EVENT_TYPE()

    uint32_t userId = 0;
    uint8_t inputType = 0;
    float deltaX = 0.0f;
    float deltaY = 0.0f;
    bool isPressed = false;

    NetworkInputEvent() = default;
    NetworkInputEvent(uint32_t user, uint8_t type, float dx, float dy,
                      bool pressed)
        : userId(user),
          inputType(type),
          deltaX(dx),
          deltaY(dy),
          isPressed(pressed) {}
};

/**
 * @brief Event fired when an entity moves out of bounds
 */
struct OutOfBoundsEvent : public IEvent {
    IMPLEMENT_EVENT_TYPE()

    ECS::Entity entity;
    float x = 0.0f;
    float y = 0.0f;

    OutOfBoundsEvent() = default;
    OutOfBoundsEvent(ECS::Entity e, float posX, float posY)
        : entity(e), x(posX), y(posY) {}
};

/**
 * @brief Event fired when a projectile is spawned
 */
struct ProjectileSpawnedEvent : public IEvent {
    IMPLEMENT_EVENT_TYPE()

    ECS::Entity projectile;
    ECS::Entity owner;
    uint32_t ownerNetworkId = 0;
    float x = 0.0f;
    float y = 0.0f;
    float velocityX = 0.0f;
    float velocityY = 0.0f;

    ProjectileSpawnedEvent() = default;
    ProjectileSpawnedEvent(ECS::Entity proj, ECS::Entity own, uint32_t netId,
                           float posX, float posY, float velX, float velY)
        : projectile(proj),
          owner(own),
          ownerNetworkId(netId),
          x(posX),
          y(posY),
          velocityX(velX),
          velocityY(velY) {}
};

}  // namespace rtype::engine::events
