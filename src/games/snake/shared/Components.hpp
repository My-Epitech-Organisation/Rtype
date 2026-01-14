/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Snake Game Components
*/

#pragma once

#include <cstdint>
#include <vector>

namespace rtype::games::snake::shared {

/**
 * @brief Direction enumeration for snake movement
 */
enum class Direction : uint8_t {
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3,
    NONE = 4
};

/**
 * @brief Position component - grid coordinates
 */
struct PositionComponent {
    int gridX = 0;
    int gridY = 0;
};

/**
 * @brief Velocity component - movement direction
 */
struct VelocityComponent {
    int dirX = 1;
    int dirY = 0;
};

/**
 * @brief Snake head component - marks the head of a snake
 */
struct SnakeHeadComponent {
    uint32_t playerId = 0;
};

/**
 * @brief Snake segment component - marks a body segment
 */
struct SnakeSegmentComponent {
    uint32_t playerId = 0;
    int segmentIndex = 0;
};

/**
 * @brief Food component
 */
struct FoodComponent {
    int value = 10;
};

/**
 * @brief Player input component
 */
struct PlayerInputComponent {
    uint32_t playerId = 0;
    Direction nextDirection = Direction::NONE;
};

/**
 * @brief Game state component
 */
struct GameStateComponent {
    int score = 0;
    bool isGameOver = false;
    float gameTime = 0.0F;
};

}  // namespace rtype::games::snake::shared
