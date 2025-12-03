/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ParallaxScrolling.hpp
*/

#ifndef SRC_CLIENT_SYSTEM_PARALLAXSCROLLING_HPP_
#define SRC_CLIENT_SYSTEM_PARALLAXSCROLLING_HPP_

#include <memory>

#include <SFML/Graphics/View.hpp>

#include "ecs/ECS.hpp"

class ParallaxScrolling {
   public:
    static void update(const std::shared_ptr<ECS::Registry>& registry,
                       sf::View view);
};

#endif  // SRC_CLIENT_SYSTEM_PARALLAXSCROLLING_HPP_
