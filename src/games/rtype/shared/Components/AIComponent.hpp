/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** AIComponent - AI behavior configuration
*/

#pragma once

#include <cstdint>

namespace rtype::games::rtype::shared {

/**
 * @enum AIBehavior
 * @brief Available AI behavior patterns
 */
enum class AIBehavior : uint8_t {
    MoveLeft = 0,  ///< Simple left movement (basic enemies)
    SineWave,      ///< Sine wave movement pattern
    Chase,         ///< Chase nearest player
    Patrol,        ///< Patrol between points
    Stationary     ///< Stay in place (turrets)
};

/**
 * @struct AIComponent
 * @brief Component for enemy AI behavior
 *
 * Defines how an AI-controlled entity behaves.
 * Processed by AISystem to update velocity based on behavior.
 */
struct AIComponent {
    AIBehavior behavior = AIBehavior::MoveLeft;  ///< Current behavior pattern
    float speed = 100.0F;     ///< Movement speed (pixels/second)
    float stateTimer = 0.0F;  ///< Timer for state-based behaviors
    float targetX = 0.0F;     ///< Target X for chase/patrol
    float targetY = 0.0F;     ///< Target Y for chase/patrol
};

}  // namespace rtype::games::rtype::shared
