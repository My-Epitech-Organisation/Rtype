/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** SFML implementation of Snake renderer - Implementation
*/

#include "SFMLSnakeRenderer.hpp"

namespace rtype::games::snake::client::sfml {

SFMLSnakeRenderer::SFMLSnakeRenderer(int width, int height, int cellSize)
    : width_(width),
      height_(height),
      cellSize_(cellSize),
      window_(sf::VideoMode(width_, height_), "Snake Game - SFML") {}

bool SFMLSnakeRenderer::initialize() {
    window_.setFramerateLimit(60);
    return window_.isOpen();
}

bool SFMLSnakeRenderer::processInput(Direction& inputDirection) {
    while (auto event = window_.pollEvent()) {
        switch (event->getType()) {
            case sf::Event::Type::Closed:
                window_.close();
                return false;
            case sf::Event::Type::KeyPressed: {
                auto keyEvent = event->getIf<sf::Event::KeyPressed>();
                if (keyEvent.has_value()) {
                    switch (keyEvent->code) {
                        case sf::Keyboard::Key::Up:
                        case sf::Keyboard::Key::W:
                            inputDirection = Direction::UP;
                            break;
                        case sf::Keyboard::Key::Down:
                        case sf::Keyboard::Key::S:
                            inputDirection = Direction::DOWN;
                            break;
                        case sf::Keyboard::Key::Left:
                        case sf::Keyboard::Key::A:
                            inputDirection = Direction::LEFT;
                            break;
                        case sf::Keyboard::Key::Right:
                        case sf::Keyboard::Key::D:
                            inputDirection = Direction::RIGHT;
                            break;
                        case sf::Keyboard::Key::Escape:
                            window_.close();
                            return false;
                        default:
                            break;
                    }
                }
                break;
            }
            default:
                break;
        }
    }
    return true;
}

void SFMLSnakeRenderer::beginFrame() { window_.clear(sf::Color::Black); }

void SFMLSnakeRenderer::endFrame() { window_.display(); }

void SFMLSnakeRenderer::renderSnake(const std::vector<SnakeSegment>& segments,
                                    uint32_t headColor, uint32_t bodyColor) {
    if (segments.empty()) {
        return;
    }

    sf::RectangleShape head(sf::Vector2f(cellSize_, cellSize_));
    head.setPosition(sf::Vector2f(segments[0].gridX * cellSize_,
                                  segments[0].gridY * cellSize_));
    head.setFillColor(
        sf::Color(headColor >> 16, (headColor >> 8) & 0xFF, headColor & 0xFF));
    window_.draw(head);

    sf::RectangleShape body(sf::Vector2f(cellSize_ - 2, cellSize_ - 2));
    for (size_t i = 1; i < segments.size(); i++) {
        body.setPosition(sf::Vector2f(segments[i].gridX * cellSize_ + 1,
                                      segments[i].gridY * cellSize_ + 1));
        body.setFillColor(sf::Color(bodyColor >> 16, (bodyColor >> 8) & 0xFF,
                                    bodyColor & 0xFF));
        window_.draw(body);
    }
}

void SFMLSnakeRenderer::renderFood(int gridX, int gridY, uint32_t color) {
    sf::RectangleShape food(sf::Vector2f(cellSize_, cellSize_));
    food.setPosition(gridX * cellSize_, gridY * cellSize_);
    food.setFillColor(
        sf::Color(color >> 16, (color >> 8) & 0xFF, color & 0xFF));
    window_.draw(food);
}

void SFMLSnakeRenderer::renderScore(int score, int x, int y) {
    // Score rendering would require font loading
    // Simplified for now
}

void SFMLSnakeRenderer::renderGameOver(int finalScore) {
    // Game over message
}

}  // namespace rtype::games::snake::client::sfml
