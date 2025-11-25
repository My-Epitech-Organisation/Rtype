#pragma once

#include <SFML/Graphics.hpp>
#include "ColorblindSimulator.hpp"

/**
 * @brief Represents a projectile with colorblind-safe visuals
 *
 * This class demonstrates the accessibility guidelines:
 * - High-contrast outline
 * - Shape-based differentiation
 * - Pattern/motion cues
 */
class Projectile {
public:
    /**
     * @brief Projectile types with different visual characteristics
     */
    enum class Type {
        PlayerBullet,    // Cyan with thick outline
        EnemyBullet,     // Orange with thick outline
        Missile,         // Yellow with pulsing effect
    };

    Projectile(Type type, sf::Vector2f position, sf::Vector2f velocity);

    /**
     * @brief Update projectile position
     */
    void update(float deltaTime);

    /**
     * @brief Draw projectile with high-contrast outline
     * @param window Target window
     * @param cvdType Current colorblind simulation mode
     */
    void draw(sf::RenderWindow& window, CVDType cvdType);

    /**
     * @brief Get current position
     */
    sf::Vector2f getPosition() const { return m_position; }

    /**
     * @brief Check if projectile is off-screen
     */
    bool isOffScreen(const sf::Vector2u& windowSize) const;

private:
    Type m_type;
    sf::Vector2f m_position;
    sf::Vector2f m_velocity;
    float m_animationTime;

    // Visual properties for accessibility
    static constexpr float OUTLINE_THICKNESS = 3.0f;

    /**
     * @brief Get base color for projectile type (before CVD transformation)
     */
    sf::Color getBaseColor() const;

    /**
     * @brief Get outline color for high contrast
     */
    sf::Color getOutlineColor() const;

    /**
     * @brief Get shape for projectile type
     */
    void createShape(sf::ConvexShape& shape) const;
};
