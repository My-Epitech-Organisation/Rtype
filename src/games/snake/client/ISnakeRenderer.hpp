/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Abstract renderer interface for Snake game - Graphics-agnostic
*/

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace rtype::games::snake::client {

/**
 * @brief Direction enum for input
 */
enum class Direction : uint8_t {
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3,
    NONE = 4
};

/**
 * @brief Snake segment data
 */
struct SnakeSegment {
    int gridX;
    int gridY;
};

/**
 * @brief Graphics-agnostic renderer interface
 *
 * This interface allows Snake to work with any graphics library.
 * Implementations: SFML, SDL2, Raylib, etc.
 */
class ISnakeRenderer {
   public:
    virtual ~ISnakeRenderer() = default;

    /**
     * @brief Initialize the renderer
     * @return true if successful
     */
    virtual bool initialize() = 0;

    /**
     * @brief Process window events and input
     * @return true if window should stay open
     */
    virtual bool processInput(Direction& inputDirection) = 0;

    /**
     * @brief Start frame rendering
     */
    virtual void beginFrame() = 0;

    /**
     * @brief End frame and display
     */
    virtual void endFrame() = 0;

    /**
     * @brief Render snake segments
     * @param segments Snake body segments
     * @param headColor Color of snake head (RGB packed in uint32_t)
     * @param bodyColor Color of snake body
     */
    virtual void renderSnake(const std::vector<SnakeSegment>& segments,
                             uint32_t headColor = 0x00FF00,
                             uint32_t bodyColor = 0x00AA00) = 0;

    /**
     * @brief Render food
     * @param gridX X position in grid
     * @param gridY Y position in grid
     * @param color Food color (RGB)
     */
    virtual void renderFood(int gridX, int gridY,
                            uint32_t color = 0xFF0000) = 0;

    /**
     * @brief Render score text
     * @param score Current score
     * @param x Screen X position
     * @param y Screen Y position
     */
    virtual void renderScore(int score, int x = 10, int y = 10) = 0;

    /**
     * @brief Render game over message
     * @param finalScore The final score
     */
    virtual void renderGameOver(int finalScore) = 0;

    /**
     * @brief Check if window should close
     * @return true if close requested
     */
    virtual bool shouldClose() const = 0;

    /**
     * @brief Get grid cell width in pixels
     */
    virtual int getCellSize() const = 0;
};

}  // namespace rtype::games::snake::client
