/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** LifetimeComponent - Component storing entity lifetime
*/

#pragma once

namespace rtype::games::rtype::shared {

/**
 * @struct LifetimeComponent
 * @brief Component storing the remaining lifetime of an entity in seconds.
 *
 * The LifetimeSystem will decrement this value and destroy the entity
 * when it reaches zero.
 */
struct LifetimeComponent {
    float remainingTime = 5.0F;

    LifetimeComponent() = default;
    explicit LifetimeComponent(float time) : remainingTime(time) {}
};

}  // namespace rtype::games::rtype::shared
