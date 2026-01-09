/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SFMLTexture.cpp
*/
#include <string>

#include "SFMLTexture.hpp"

bool rtype::display::SFMLTexture::loadFromFile(const std::string &path) {
    return _texture.loadFromFile(path);
}

void rtype::display::SFMLTexture::setRepeated(bool repeated) {
    _texture.setRepeated(repeated);
}

void rtype::display::SFMLTexture::setSmooth(bool smooth) {
    _texture.setSmooth(smooth);
}

rtype::display::Vector2u rtype::display::SFMLTexture::getSize() const {
    auto size = _texture.getSize();
    return {size.x, size.y};
}
