/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** SDL2 implementation of Snake renderer
*/

#pragma once

#include <memory>
#include <string>

#include <SDL2/SDL.h>

#include "../ISnakeRenderer.hpp"

namespace rtype::games::snake::client::sdl2 {

/**
 * @brief SDL2 implementation of Snake renderer
 *
 * Uses SDL2 for low-level graphics rendering.
 * Proves that the engine works with different graphics libraries.
 */
class SDL2SnakeRenderer : public ISnakeRenderer {
   public:
    explicit SDL2SnakeRenderer(int width = 1280, int height = 720,
                               int cellSize = 32);
    ~SDL2SnakeRenderer() override;

    SDL2SnakeRenderer(const SDL2SnakeRenderer&) = delete;
    SDL2SnakeRenderer& operator=(const SDL2SnakeRenderer&) = delete;

    bool initialize() override;
    bool processInput(Direction& inputDirection) override;
    void beginFrame() override;
    void endFrame() override;
    void renderSnake(const std::vector<SnakeSegment>& segments,
                     uint32_t headColor = 0x00FF00,
                     uint32_t bodyColor = 0x00AA00) override;
    void renderFood(int gridX, int gridY, uint32_t color = 0xFF0000) override;
    void renderScore(int score, int x = 10, int y = 10) override;
    void renderGameOver(int finalScore) override;
    void drawGrid(int gridWidth, int gridHeight, uint32_t gridColor = 0x333333);
    void renderText(const std::string& text, int x, int y,
                    uint32_t color = 0xFFFFFF);
    bool shouldClose() const override { return shouldClose_; }
    int getCellSize() const override { return cellSize_; }

   private:
    int width_;
    int height_;
    int cellSize_;
    bool shouldClose_ = false;

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* texture_ = nullptr;
    uint32_t* pixels_ = nullptr;

    /**
     * @brief Draw a filled rectangle
     * @param x Grid X
     * @param y Grid Y
     * @param color RGB color
     */
    void drawCell(int gridX, int gridY, uint32_t color);

    /**
     * @brief Convert RGB to SDL format
     */
    uint32_t rgbToSDL(uint32_t rgb) const;
};

}  // namespace rtype::games::snake::client::sdl2
