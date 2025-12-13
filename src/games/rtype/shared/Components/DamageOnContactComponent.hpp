/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** DamageOnContactComponent - Deal damage when colliding
*/

#pragma once

#include <cstdint>

namespace rtype::games::rtype::shared {

struct DamageOnContactComponent {
    int32_t damage{10};
    bool destroySelf{false};
};

}  // namespace rtype::games::rtype::shared
