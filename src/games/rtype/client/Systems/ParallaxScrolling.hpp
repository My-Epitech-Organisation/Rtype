/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ParallaxScrolling.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_PARALLAXSCROLLING_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_PARALLAXSCROLLING_HPP_

#include <memory>

#include <SFML/Graphics/View.hpp>

#include "ASystem.hpp"
#include "ecs/ECS.hpp"

namespace rtype::games::rtype::client {
class ParallaxScrolling : public ::rtype::engine::ASystem {
   private:
    std::shared_ptr<sf::View> _view;

   public:
    explicit ParallaxScrolling(std::shared_ptr<sf::View> view);
    void update(ECS::Registry& registry, float dt) override;
};
}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_PARALLAXSCROLLING_HPP_
