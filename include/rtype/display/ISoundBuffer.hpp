/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ISoundBuffer.hpp
*/

#ifndef R_TYPE_ISOUNDBUFFER_HPP
#define R_TYPE_ISOUNDBUFFER_HPP

#include <string>

namespace rtype::display {
    class ISoundBuffer {
    public:
        virtual ~ISoundBuffer() = default;

        virtual bool loadFromFile(const std::string& path) = 0;
    };
}

#endif //R_TYPE_ISOUNDBUFFER_HPP