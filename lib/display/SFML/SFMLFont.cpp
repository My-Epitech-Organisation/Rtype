/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SFMLFont.cpp
*/
#include "SFMLFont.hpp"

bool rtype::display::SFMLFont::openFromFile(const std::string &path) {
    return _font.openFromFile(path);
}
