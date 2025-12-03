/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** MovementSystem.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_MOVEMENTSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_MOVEMENTSYSTEM_HPP_
#include <memory>

#include "ecs/ECS.hpp"

class MovementSystem {
   public:
    static void update(const std::shared_ptr<ECS::Registry>& registry,
                       float dt);
};

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_MOVEMENTSYSTEM_HPP_
