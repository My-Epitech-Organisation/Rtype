/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Components for Event Bus PoC
*/

#ifndef COMPONENTS_HPP
    #define COMPONENTS_HPP

namespace PoC {

    /**
     * @brief Position component with x, y coordinates
     */
    struct Position {
        float x;
        float y;

        Position(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
    };

    /**
     * @brief Velocity component with dx, dy speed
     */
    struct Velocity {
        float dx;
        float dy;

        Velocity(float dx = 0.0f, float dy = 0.0f) : dx(dx), dy(dy) {}
    };

    /**
     * @brief Collider component for collision detection
     */
    struct Collider {
        float radius;

        Collider(float radius = 1.0f) : radius(radius) {}
    };

    /**
     * @brief Tag component to mark entities that have collided
     */
    struct CollisionTag {};

    /**
     * @brief Component to track if collision sound has been played
     */
    struct AudioPlayed {
        int soundId;
        AudioPlayed(int id = -1) : soundId(id) {}
    };

} // namespace PoC

#endif // COMPONENTS_HPP
