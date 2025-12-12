/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** PowerUpSystem - Manages active power-up timers and cleanup
*/

#pragma once

#include <rtype/engine.hpp>

#include "../../Components/CooldownComponent.hpp"
#include "../../Components/PowerUpComponent.hpp"
#include "../../Components/Tags.hpp"

namespace rtype::games::rtype::shared {

/**
 * @class PowerUpSystem
 * @brief Updates power-up timers and removes expired effects
 *
 * When a player picks up a new power-up while one is already active,
 * the previous power-up is immediately replaced (effects removed and
 * timer reset). Only one power-up can be active per player at a time.
 *
 * @note Power-up effects are automatically cleaned up when the timer expires:
 * - Shield: Removes InvincibleTag
 * - RapidFire: Restores original weapon cooldown
 * - Other effects: Removed with the ActivePowerUpComponent
 */
class PowerUpSystem : public ::rtype::engine::ASystem {
   public:
    PowerUpSystem() : ASystem("PowerUpSystem") {}

    void update(ECS::Registry& registry, float deltaTime) override;
};

}  // namespace rtype::games::rtype::shared
