/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** DisplayTypes.hpp
*/

#ifndef DISPLAYTYPES_HPP_
#define DISPLAYTYPES_HPP_

#include <cstdint>
#include "DisplayEnum.hpp"

namespace rtype::display {

    /**
     * @brief Representation of an RGBA color.
     */
    struct Color {
        uint8_t r = 0, g = 0, b = 0;
        uint8_t a = 255;

        static Color White() { return {255, 255, 255, 255}; }
        static Color Black() { return {0, 0, 0, 255}; }
        static Color Red() { return {255, 0, 0, 255}; }
        static Color Cyan() { return {0, 255, 255, 255}; }
        static Color Green() { return {0, 255, 0, 255}; }
        static Color Blue() { return {0, 0, 255, 255}; }
        static Color Yellow() { return { 245,224,80, 255 }; }
        static Color Transparent() { return {0, 0, 0, 0}; }
    };

    /**
     * @brief Generic 2D vector structure.
     */
    template<typename T>
    struct Vector2 {
        T x, y;
    };

    /**
     * @brief Generic rectangle structure.
     */
    template<typename T>
    struct Rect {
        T left, top, width, height;
    };

    using Vector2f = Vector2<float>;
    using Vector2i = Vector2<int>;
    using Vector2u = Vector2<unsigned int>;
    using IntRect = Rect<int>;
    using FloatRect = Rect<float>;

    /**
     * @brief Structure representing a polled event.
     * Uses a union to store data specific to the event type.
     */
    struct Event {
        EventType type;
        union {
            struct {
                Key code;
                bool alt;
                bool control;
                bool shift;
                bool system;
            } key;
            struct {
                MouseButton button;
                int x;
                int y;
            } mouseButton;
            struct {
                int x;
                int y;
            } mouseMove;
            struct {
                float delta;
                int x;
                int y;
            } mouseWheel;
            struct {
                unsigned int joystickId;
                unsigned int button;
            } joystickButton;
            struct {
                unsigned int joystickId;
                JoystickAxis axis;
                float position;
            } joystickMove;
            struct {
                uint32_t unicode;
            } text;
        };
    };

} // namespace rtype::display

#endif // DISPLAYTYPES_HPP_
