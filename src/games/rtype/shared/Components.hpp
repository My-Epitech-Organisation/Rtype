/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Components
*/

#pragma once

namespace rtype::games::rtype::shared {

struct TransformComponent {
    float x = 0.0f;
    float y = 0.0f;
    float rotation = 0.0f;
};

struct VelocityComponent {
    float vx = 0.0f;
    float vy = 0.0f;
};

struct NetworkIdComponent {
    unsigned int networkId = 0;
};

}  // namespace rtype::games::rtype::shared
