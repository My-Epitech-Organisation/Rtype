/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** PowerUpSystem - Manages active power-up timers and cleanup
*/

#pragma once

#include <rtype/engine.hpp>

#include "../../Components/PowerUpComponent.hpp"
#include "../../Components/CooldownComponent.hpp"
#include "../../Components/Tags.hpp"

namespace rtype::games::rtype::shared {

class PowerUpSystem : public ::rtype::engine::ASystem {
   public:
    PowerUpSystem() : ASystem("PowerUpSystem") {}

    void update(ECS::Registry& registry, float deltaTime) override;
};

}  // namespace rtype::games::rtype::shared
