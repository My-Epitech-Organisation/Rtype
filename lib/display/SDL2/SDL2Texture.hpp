/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SDL2Texture.hpp
*/

#ifndef R_TYPE_SDL2TEXTURE_HPP
#define R_TYPE_SDL2TEXTURE_HPP

#include "include/rtype/display/IDisplay.hpp"
#include <SDL2/SDL.h>
#include <string>

namespace rtype::display {
    class SDL2Texture : public ITexture {
    public:
        SDL2Texture();
        ~SDL2Texture() override;

        bool loadFromFile(const std::string& path) override;
        void setRepeated(bool repeated) override;
        void setSmooth(bool smooth) override;
        Vector2u getSize() const override;

        SDL_Texture* getSDL2Texture() const { return _texture; }
        void setSDL2Texture(SDL_Texture* texture);

    private:
        SDL_Texture* _texture = nullptr;
        Vector2u _cachedSize{0, 0};
    };
}

#endif //R_TYPE_SDL2TEXTURE_HPP
