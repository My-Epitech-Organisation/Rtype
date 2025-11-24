/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Bézier Curve Movement PoC
*/

#ifndef BEZIER_MOVEMENT_HPP
    #define BEZIER_MOVEMENT_HPP

#include <cmath>
#include <vector>

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
     * @brief 2D Point for Bézier control points
     */
    struct Point {
        float x = 0.0f;
        float y = 0.0f;

        Point() = default;
        Point(float x_, float y_) : x(x_), y(y_) {}
    };

    /**
     * @brief Quadratic Bézier curve parameters
     * A quadratic Bézier uses 3 control points: start (P0), control (P1), end (P2)
     */
    struct QuadraticBezier {
        Point p0;    // Start point
        Point p1;    // Control point
        Point p2;    // End point
        float speed; // Movement speed along curve (0-1 per second)
        float t;     // Current position on curve (0.0 to 1.0)

        QuadraticBezier() : speed(1.0f), t(0.0f) {}
        QuadraticBezier(const Point& start, const Point& control, const Point& end, float spd)
            : p0(start), p1(control), p2(end), speed(spd), t(0.0f) {}

        /**
         * @brief Calculate position at parameter t using quadratic Bézier formula
         * Formula: B(t) = (1-t)²P0 + 2(1-t)tP1 + t²P2
         * @param t Parameter (0.0 to 1.0)
         * @return Point on curve
         */
        Point evaluate(float t) const {
            float oneMinusT = 1.0f - t;
            float oneMinusT2 = oneMinusT * oneMinusT;
            float t2 = t * t;
            
            return Point(
                oneMinusT2 * p0.x + 2.0f * oneMinusT * t * p1.x + t2 * p2.x,
                oneMinusT2 * p0.y + 2.0f * oneMinusT * t * p1.y + t2 * p2.y
            );
        }

        /**
         * @brief Check if curve traversal is complete
         */
        bool isComplete() const {
            return t >= 1.0f;
        }
    };

    /**
     * @brief Cubic Bézier curve parameters (optional, more complex curves)
     * Uses 4 control points for more intricate paths
     */
    struct CubicBezier {
        Point p0;    // Start point
        Point p1;    // First control point
        Point p2;    // Second control point
        Point p3;    // End point
        float speed; // Movement speed along curve
        float t;     // Current position on curve

        CubicBezier() : speed(1.0f), t(0.0f) {}
        CubicBezier(const Point& start, const Point& ctrl1, const Point& ctrl2, const Point& end, float spd)
            : p0(start), p1(ctrl1), p2(ctrl2), p3(end), speed(spd), t(0.0f) {}

        /**
         * @brief Calculate position using cubic Bézier formula
         * Formula: B(t) = (1-t)³P0 + 3(1-t)²tP1 + 3(1-t)t²P2 + t³P3
         */
        Point evaluate(float t) const {
            float oneMinusT = 1.0f - t;
            float oneMinusT2 = oneMinusT * oneMinusT;
            float oneMinusT3 = oneMinusT2 * oneMinusT;
            float t2 = t * t;
            float t3 = t2 * t;
            
            return Point(
                oneMinusT3 * p0.x + 3.0f * oneMinusT2 * t * p1.x + 
                3.0f * oneMinusT * t2 * p2.x + t3 * p3.x,
                oneMinusT3 * p0.y + 3.0f * oneMinusT2 * t * p1.y + 
                3.0f * oneMinusT * t2 * p2.y + t3 * p3.y
            );
        }

        bool isComplete() const {
            return t >= 1.0f;
        }
    };

    /**
     * @brief Quadratic Bézier movement system
     */
    class QuadraticBezierSystem {
    public:
        template<typename Registry>
        static void update(Registry& registry, float deltaTime) {
            registry.template view<Position, QuadraticBezier>().each([deltaTime](
                auto entity, Position& pos, QuadraticBezier& bezier
            ) {
                if (!bezier.isComplete()) {
                    // Update parameter t
                    bezier.t += bezier.speed * deltaTime;
                    if (bezier.t > 1.0f) bezier.t = 1.0f;

                    // Calculate new position
                    Point newPos = bezier.evaluate(bezier.t);
                    pos.x = newPos.x;
                    pos.y = newPos.y;
                }
            });
        }
    };

    /**
     * @brief Cubic Bézier movement system
     */
    class CubicBezierSystem {
    public:
        template<typename Registry>
        static void update(Registry& registry, float deltaTime) {
            registry.template view<Position, CubicBezier>().each([deltaTime](
                auto entity, Position& pos, CubicBezier& bezier
            ) {
                if (!bezier.isComplete()) {
                    bezier.t += bezier.speed * deltaTime;
                    if (bezier.t > 1.0f) bezier.t = 1.0f;

                    Point newPos = bezier.evaluate(bezier.t);
                    pos.x = newPos.x;
                    pos.y = newPos.y;
                }
            });
        }
    };

} // namespace Movement

#endif // BEZIER_MOVEMENT_HPP
