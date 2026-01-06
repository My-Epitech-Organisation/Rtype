/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** DisplayTypes.hpp
*/

#ifndef DISPLAYTYPES_HPP_
#define DISPLAYTYPES_HPP_

#include <cstdint>

namespace rtype::display {

    struct Color {
        uint8_t r, g, b, a;

        static Color White() { return {255, 255, 255, 255}; }
        static Color Black() { return {0, 0, 0, 255}; }
        static Color Red() { return {255, 0, 0, 255}; }
        static Color Cyan() { return {0, 255, 255, 255}; }
        static Color Green() { return {0, 255, 0, 255}; }
        static Color Blue() { return {0, 0, 255, 255}; }
        static Color Yellow() { return { 245,224,80, 255 }; }
        static Color Transparent() { return {0, 0, 0, 0}; }
    };

    template<typename T>
    struct Vector2 {
        T x, y;
    };

    template<typename T>
    struct Rect {
        T left, top, width, height;
    };

    using Vector2f = Vector2<float>;
    using Vector2i = Vector2<int>;
    using Vector2u = Vector2<unsigned int>;
    using IntRect = Rect<int>;
    using FloatRect = Rect<float>;

} // namespace rtype::display

#endif // DISPLAYTYPES_HPP_
