/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Components
*/

#pragma once

namespace rtype::games::rtype::shared {

struct TransformComponent {
    float x = 0.0F;
    float y = 0.0F;
    float rotation = 0.0F;
};

struct VelocityComponent {
    float vx = 0.0F;
    float vy = 0.0F;
};

struct NetworkIdComponent {
    unsigned int networkId = 0;
};

}  // namespace rtype::games::rtype::shared
