/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** BoxingSystem.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_BOXINGSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_BOXINGSYSTEM_HPP_

#include <memory>

#include <SFML/Graphics/RenderWindow.hpp>

#include "ASystem.hpp"
#include "ecs/ECS.hpp"

namespace rtype::games::rtype::client {
    class BoxingSystem : public ::rtype::engine::ASystem {
    private:
        std::shared_ptr<sf::RenderWindow> _window;

    public:
        explicit BoxingSystem(std::shared_ptr<sf::RenderWindow> window);
        void update(ECS::Registry& registry, float dt) override;
    };
}  // namespace rtype::games::rtype::client
#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_BOXINGSYSTEM_HPP_
