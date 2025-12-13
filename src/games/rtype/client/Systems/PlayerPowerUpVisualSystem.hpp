/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** PlayerPowerUpVisualSystem - Adds simple visual cues for active power-ups
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_PLAYERPOWERUPVISUALSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_PLAYERPOWERUPVISUALSYSTEM_HPP_

#include <rtype/engine.hpp>

#include "../../shared/Components/PowerUpComponent.hpp"

namespace rtype::games::rtype::client {

class PlayerPowerUpVisualSystem : public ::rtype::engine::ASystem {
   public:
    PlayerPowerUpVisualSystem();
    ~PlayerPowerUpVisualSystem() override = default;

    void update(ECS::Registry& registry, float dt) override;
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_PLAYERPOWERUPVISUALSYSTEM_HPP_
