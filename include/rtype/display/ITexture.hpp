/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ITexture.hpp
*/

#ifndef R_TYPE_ITEXTURE_HPP
#define R_TYPE_ITEXTURE_HPP

#include <string>
#include "DisplayTypes.hpp"

namespace rtype::display {
    class ITexture {
    public:
        virtual ~ITexture() = default;

        virtual bool loadFromFile(const std::string& path) = 0;
        virtual void setRepeated(bool repeated) = 0;
        virtual void setSmooth(bool smooth) = 0;
        virtual Vector2u getSize() const = 0;
    };
}
#endif //R_TYPE_ITEXTURE_HPP