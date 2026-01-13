/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** SFML implementation of Snake renderer
*/

#pragma once

#include <memory>

#include <SFML/Graphics.hpp>

#include "../ISnakeRenderer.hpp"

namespace rtype::games::snake::client::sfml {

/**
 * @brief SFML implementation of Snake renderer
 */
class SFMLSnakeRenderer : public ISnakeRenderer {
   public:
    explicit SFMLSnakeRenderer(int width = 1280, int height = 720,
                               int cellSize = 32);
    ~SFMLSnakeRenderer() override = default;

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
    bool shouldClose() const override { return !window_.isOpen(); }
    int getCellSize() const override { return cellSize_; }

   private:
    int width_;
    int height_;
    int cellSize_;

    sf::RenderWindow window_;
    sf::Font font_;
};

}  // namespace rtype::games::snake::client::sfml
