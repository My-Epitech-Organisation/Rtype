/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SFMLTexture.hpp
*/

#ifndef R_TYPE_SFMLTEXTURE_HPP
#define R_TYPE_SFMLTEXTURE_HPP
#include <SFML/Graphics/Texture.hpp>
#include "include/rtype/display/IDisplay.hpp"


namespace rtype::display {
    class SFMLTexture : public rtype::display::ITexture {
    public:
        bool loadFromFile(const std::string& path) override;
        void setRepeated(bool repeated) override;
        void setSmooth(bool smooth) override;
        Vector2u getSize() const override;

        sf::Texture& getSFMLTexture() { return _texture; }

    private:
        sf::Texture _texture;
    };
}


#endif //R_TYPE_SFMLTEXTURE_HPP