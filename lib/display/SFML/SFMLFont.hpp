/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SFMLFont.hpp
*/

#ifndef R_TYPE_SFMLFONT_HPP
#define R_TYPE_SFMLFONT_HPP
#include <SFML/Graphics/Font.hpp>
#include "include/rtype/display/IDisplay.hpp"

namespace rtype::display {
    class SFMLFont : public rtype::display::IFont {
    public:
        bool openFromFile(const std::string& path) override;
        sf::Font& getSFMLFont() { return _font; }

    private:
        sf::Font _font;
    };
}

#endif //R_TYPE_SFMLFONT_HPP
