#include "Projectile.hpp"
#include <cmath>

Projectile::Projectile(Type type, sf::Vector2f position, sf::Vector2f velocity)
    : m_type(type), m_position(position), m_velocity(velocity), m_animationTime(0.0f) {
}

void Projectile::update(float deltaTime) {
    m_position += m_velocity * deltaTime;
    m_animationTime += deltaTime;
}

void Projectile::draw(sf::RenderWindow& window, CVDType cvdType) {
    sf::ConvexShape shape;
    createShape(shape);

    // Apply colorblind transformation to base color
    sf::Color baseColor = ColorblindSimulator::transformColor(getBaseColor(), cvdType);
    sf::Color outlineColor = ColorblindSimulator::transformColor(getOutlineColor(), cvdType);

    // Add pulsing effect for missiles
    if (m_type == Type::Missile) {
        float pulse = std::sin(m_animationTime * 8.0f) * 0.3f + 0.7f;
        baseColor.r = static_cast<sf::Uint8>(baseColor.r * pulse);
        baseColor.g = static_cast<sf::Uint8>(baseColor.g * pulse);
        baseColor.b = static_cast<sf::Uint8>(baseColor.b * pulse);
    }

    shape.setFillColor(baseColor);
    shape.setOutlineColor(outlineColor);
    shape.setOutlineThickness(OUTLINE_THICKNESS);
    shape.setPosition(m_position);

    window.draw(shape);

    // Add additional visual cue for missiles (trailing glow)
    if (m_type == Type::Missile) {
        sf::CircleShape glow(15.0f);
        glow.setFillColor(sf::Color(255, 255, 0, 50));
        glow.setOrigin(15.0f, 15.0f);
        glow.setPosition(m_position);
        window.draw(glow);
    }
}

bool Projectile::isOffScreen(const sf::Vector2u& windowSize) const {
    return m_position.x < -50.0f || m_position.x > windowSize.x + 50.0f ||
           m_position.y < -50.0f || m_position.y > windowSize.y + 50.0f;
}

sf::Color Projectile::getBaseColor() const {
    switch (m_type) {
        case Type::PlayerBullet:
            return sf::Color(0, 200, 255);    // Cyan - high visibility
        case Type::EnemyBullet:
            return sf::Color(255, 120, 0);    // Orange - distinct from player
        case Type::Missile:
            return sf::Color(255, 255, 0);    // Yellow - warning color
        default:
            return sf::Color::White;
    }
}

sf::Color Projectile::getOutlineColor() const {
    // High-contrast outlines for all projectile types
    switch (m_type) {
        case Type::PlayerBullet:
            return sf::Color(255, 255, 255);  // White outline for cyan
        case Type::EnemyBullet:
            return sf::Color(100, 0, 0);      // Dark red outline for orange
        case Type::Missile:
            return sf::Color(150, 0, 0);      // Dark red for yellow
        default:
            return sf::Color::Black;
    }
}

void Projectile::createShape(sf::ConvexShape& shape) const {
    // Different shapes for different projectile types
    // This provides shape-based differentiation in addition to color
    
    switch (m_type) {
        case Type::PlayerBullet:
            // Elongated diamond shape
            shape.setPointCount(4);
            shape.setPoint(0, sf::Vector2f(0, -12));
            shape.setPoint(1, sf::Vector2f(6, 0));
            shape.setPoint(2, sf::Vector2f(0, 12));
            shape.setPoint(3, sf::Vector2f(-6, 0));
            shape.setOrigin(0, 0);
            break;

        case Type::EnemyBullet:
            // Square shape
            shape.setPointCount(4);
            shape.setPoint(0, sf::Vector2f(-8, -8));
            shape.setPoint(1, sf::Vector2f(8, -8));
            shape.setPoint(2, sf::Vector2f(8, 8));
            shape.setPoint(3, sf::Vector2f(-8, 8));
            shape.setOrigin(0, 0);
            break;

        case Type::Missile:
            // Arrow/triangle shape
            shape.setPointCount(3);
            shape.setPoint(0, sf::Vector2f(0, -15));
            shape.setPoint(1, sf::Vector2f(10, 10));
            shape.setPoint(2, sf::Vector2f(-10, 10));
            shape.setOrigin(0, 0);
            break;
    }
}
