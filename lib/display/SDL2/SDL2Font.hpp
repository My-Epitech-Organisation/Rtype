/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SDL2Font.hpp
*/

#ifndef R_TYPE_SDL2FONT_HPP
#define R_TYPE_SDL2FONT_HPP

#include "include/rtype/display/IFont.hpp"
#include <SDL2/SDL_ttf.h>
#include <string>

namespace rtype::display {
    class SDL2Font : public IFont {
    public:
        SDL2Font();
        ~SDL2Font() override;

        bool openFromFile(const std::string& path) override;

        TTF_Font* getFont(unsigned int size);

    private:
        std::string _path;
    };
}

#endif //R_TYPE_SDL2FONT_HPP
