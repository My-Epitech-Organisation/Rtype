/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Graphic.hpp
*/

#ifndef SRC_CLIENT_GRAPHICSFML_GRAPHIC_HPP_
#define SRC_CLIENT_GRAPHICSFML_GRAPHIC_HPP_

#include <SFML/Graphics.hpp>

namespace RTypeClient {
class Graphic {
   private:
    sf::Texture _vesselTexture;
    sf::Sprite _vessel;
    sf::RenderWindow _window;
    bool _appRunning = true;

   public:
    void pollEvents();

    void display();

    void loop();

    Graphic();
    ~Graphic();
};
}  // namespace RTypeClient

#endif  // SRC_CLIENT_GRAPHICSFML_GRAPHIC_HPP_
