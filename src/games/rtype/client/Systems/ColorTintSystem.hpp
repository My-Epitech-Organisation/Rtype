/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ColorTintSystem - Applies color tints to sprites
*/

#pragma once

#include "ASystem.hpp"
#include "ECS.hpp"

namespace rtype::games::rtype::client {

/**
 * @class ColorTintSystem
 * @brief Applies color tints to sprites based on ColorTint component
 *
 * This system updates sprite colors when a ColorTint component is present,
 * enabling visual distinction between entity types (e.g., different enemy
 * variants).
 */
class ColorTintSystem : public ::rtype::engine::ASystem {
   public:
    ColorTintSystem();
    ~ColorTintSystem() override = default;

    void update(ECS::Registry& registry, float dt) override;
};

}  // namespace rtype::games::rtype::client
