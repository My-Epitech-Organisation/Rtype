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
    /**
     * @brief Interface for sound data storage (SFX).
     */
    class ISoundBuffer {
    public:
        virtual ~ISoundBuffer() = default;

        /**
         * @brief Load sound data from a file into memory.
         * @param path Path to the audio file.
         * @return true if loading succeeded.
         */
        virtual bool loadFromFile(const std::string& path) = 0;
    };
}

#endif //R_TYPE_ISOUNDBUFFER_HPP