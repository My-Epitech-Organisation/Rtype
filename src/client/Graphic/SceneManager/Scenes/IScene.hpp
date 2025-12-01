/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** IScene.hpp
*/

#ifndef R_TYPE_ISCENE_HPP
#define R_TYPE_ISCENE_HPP
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

class IScene {
public:
    virtual void pollEvents(const sf::Event &e) = 0;
    virtual void update() = 0;
    virtual void render(sf::RenderWindow &window) = 0;

    virtual ~IScene() = default;
};

#endif //R_TYPE_ISCENE_HPP