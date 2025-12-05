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

#include "ecs/ECS.hpp"

class ParallaxScrolling {
   public:
    static void update(std::shared_ptr<ECS::Registry> registry, sf::View view);
};

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_PARALLAXSCROLLING_HPP_
