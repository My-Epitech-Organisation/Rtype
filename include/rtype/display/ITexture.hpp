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
    /**
     * @brief Interface for texture resources used in rendering.
     */
    class ITexture {
    public:
        virtual ~ITexture() = default;

        /**
         * @brief Load a texture from an image file.
         * @param path Path to the image file.
         * @return true if loading succeeded.
         */
        virtual bool loadFromFile(const std::string& path) = 0;

        /**
         * @brief Enable or disable texture repeating (tiling).
         * @param repeated True to enable repeating.
         */
        virtual void setRepeated(bool repeated) = 0;

        /**
         * @brief Enable or disable bilinear filtering (smoothing).
         * @param smooth True to enable smoothing.
         */
        virtual void setSmooth(bool smooth) = 0;

        /**
         * @brief Get the dimensions of the texture.
         * @return Vector2u containing width and height.
         */
        virtual Vector2u getSize() const = 0;
    };
}
#endif //R_TYPE_ITEXTURE_HPP
