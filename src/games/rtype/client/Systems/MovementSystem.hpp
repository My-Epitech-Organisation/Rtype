/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** MovementSystem.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_MOVEMENTSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_MOVEMENTSYSTEM_HPP_
#include <memory>

#include "ASystem.hpp"
#include "ecs/ECS.hpp"

namespace rtype::games::rtype::client {
class MovementSystem : public ::rtype::engine::ASystem {
   public:
    MovementSystem();
    void update(ECS::Registry& registry, float dt) override;
};
}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_MOVEMENTSYSTEM_HPP_
