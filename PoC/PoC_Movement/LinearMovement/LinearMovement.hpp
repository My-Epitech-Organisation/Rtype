/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Linear Movement PoC
*/

#ifndef LINEAR_MOVEMENT_HPP
    #define LINEAR_MOVEMENT_HPP

#include <cmath>

namespace Movement {

    /**
     * @brief Position component for entities in 2D space
     */
    struct Position {
        float x = 0.0f;
        float y = 0.0f;

        Position() = default;
        Position(float x_, float y_) : x(x_), y(y_) {}
    };

    /**
     * @brief Direction vector for linear movement
     */
    struct Direction {
        float dx = 0.0f;
        float dy = 0.0f;

        Direction() = default;
        Direction(float dx_, float dy_) : dx(dx_), dy(dy_) {}

        /**
         * @brief Normalize the direction vector
         */
        void normalize() {
            float length = std::sqrt(dx * dx + dy * dy);
            if (length > 0.0f) {
                dx /= length;
                dy /= length;
            }
        }
    };

    /**
     * @brief Speed component for movement velocity
     */
    struct Speed {
        float value = 0.0f;

        Speed() = default;
        Speed(float v) : value(v) {}
    };

    /**
     * @brief Linear movement system
     * Formula: pos += dir * speed * dt
     * 
     * This is the most basic movement pattern, suitable for:
     * - Bullets
     * - Simple enemies
     * - Projectiles
     * - Particles
     */
    class LinearMovementSystem {
    public:
        /**
         * @brief Update all entities with linear movement
         * @param registry ECS registry
         * @param deltaTime Time elapsed since last frame
         */
        template<typename Registry>
        static void update(Registry& registry, float deltaTime) {
            registry.template view<Position, Direction, Speed>().each([deltaTime](
                auto entity, Position& pos, const Direction& dir, const Speed& speed
            ) {
                // Formula: pos += dir * speed * dt
                pos.x += dir.dx * speed.value * deltaTime;
                pos.y += dir.dy * speed.value * deltaTime;
            });
        }
    };

} // namespace Movement

#endif // LINEAR_MOVEMENT_HPP
