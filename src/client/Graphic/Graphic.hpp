/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Graphic.hpp
*/

#ifndef R_TYPE_GRAPHIC_HPP
#define R_TYPE_GRAPHIC_HPP

#include "./SFML/Graphics.hpp"

namespace RTypeClient {
    class Graphic {
    private:
        sf::RenderWindow _window;
        bool _appRunning = true;

    public:
        void pollEvents();

        void display();

        void loop();

        Graphic();
        ~Graphic();
    };
}

#endif //R_TYPE_GRAPHIC_HPP