/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** IFont.hpp
*/

#ifndef R_TYPE_IFONT_HPP
#define R_TYPE_IFONT_HPP
#include <string>

namespace rtype::display {
    class IFont {
    public:
        virtual ~IFont() = default;

        virtual bool openFromFile(const std::string& path) = 0;
    };
}

#endif //R_TYPE_IFONT_HPP