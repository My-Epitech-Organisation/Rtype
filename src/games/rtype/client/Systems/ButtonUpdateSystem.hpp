/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** UpdateSystem.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_BUTTONUPDATESYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_BUTTONUPDATESYSTEM_HPP_

#include <SFML/Graphics.hpp>

#include "ASystem.hpp"
#include "SFML/Window.hpp"
#include "ecs/ECS.hpp"

class ButtonUpdateSystem : public rtype::engine::ASystem {
   private:
    std::shared_ptr<sf::RenderWindow> _window;

   public:
    explicit ButtonUpdateSystem(std::shared_ptr<sf::RenderWindow> window);
    void update(ECS::Registry& registry, float dt) override;
};

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_BUTTONUPDATESYSTEM_HPP_
