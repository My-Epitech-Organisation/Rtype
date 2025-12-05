/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ResetTriggersSystem.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_RESETTRIGGERSSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_RESETTRIGGERSSYSTEM_HPP_

#include <memory>

#include <SFML/Graphics.hpp>

#include "ASystem.hpp"
#include "ecs/ECS.hpp"

namespace rtype::games::rtype::client {
class ResetTriggersSystem : public ::rtype::engine::ASystem {
   public:
    ResetTriggersSystem();
    void update(ECS::Registry& registry, float dt) override;
};
}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_RESETTRIGGERSSYSTEM_HPP_
