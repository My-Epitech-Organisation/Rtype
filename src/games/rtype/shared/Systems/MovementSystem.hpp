/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** MovementSystem
*/

#pragma once

#include "../Components.hpp"

namespace rtype::games::rtype::shared {

TransformComponent updateMovement(TransformComponent transform,
                                  const VelocityComponent& velocity,
                                  float deltaTime);

}  // namespace rtype::games::rtype::shared
