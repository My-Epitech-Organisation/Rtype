/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ColorTintSystem - Applies color tints to sprites
*/

#include "ColorTintSystem.hpp"

#include "../AllComponents.hpp"

namespace rc = ::rtype::games::rtype::client;

namespace rtype::games::rtype::client {

ColorTintSystem::ColorTintSystem()
    : ::rtype::engine::ASystem("ColorTintSystem") {}

void ColorTintSystem::update(ECS::Registry& registry, float dt) {
    registry.view<Image, ColorTint>().each([](auto /*entity*/, auto& spriteData,
                                              auto& tint) {
    });
}

}  // namespace rtype::games::rtype::client
