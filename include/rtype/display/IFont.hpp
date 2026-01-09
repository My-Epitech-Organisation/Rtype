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
    /**
     * @brief Interface for font resources.
     */
    class IFont {
    public:
        virtual ~IFont() = default;

        /**
         * @brief Load the font from a file.
         * @param path Path to the font file.
         * @return true if loading succeeded, false otherwise.
         */
        virtual bool openFromFile(const std::string& path) = 0;
    };
}

#endif //R_TYPE_IFONT_HPP
