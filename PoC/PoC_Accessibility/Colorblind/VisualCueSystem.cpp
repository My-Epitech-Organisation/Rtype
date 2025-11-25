#include "VisualCueSystem.hpp"
#include <cmath>

void VisualCueSystem::triggerCue(VisualCueType type, sf::Vector2f position, 
                                sf::Vector2f direction) {
    float lifetime = 0.5f; // Default lifetime

    switch (type) {
        case VisualCueType::HitIndicator:
            lifetime = 0.3f;
            break;
        case VisualCueType::MissileWarning:
            lifetime = 1.0f;
            break;
        case VisualCueType::PowerUpSpawn:
            lifetime = 0.8f;
            break;
    }

    m_activeCues.emplace_back(type, position, lifetime, direction);
}

void VisualCueSystem::update(float deltaTime) {
    // Update and remove expired cues
    m_activeCues.erase(
        std::remove_if(m_activeCues.begin(), m_activeCues.end(),
            [deltaTime](VisualCue& cue) {
                cue.lifetime -= deltaTime;
                return cue.lifetime <= 0.0f;
            }),
        m_activeCues.end()
    );
}

void VisualCueSystem::draw(sf::RenderWindow& window) {
    for (const auto& cue : m_activeCues) {
        switch (cue.type) {
            case VisualCueType::HitIndicator:
                drawHitIndicator(window, cue);
                break;
            case VisualCueType::MissileWarning:
                drawMissileWarning(window, cue);
                break;
            case VisualCueType::PowerUpSpawn:
                drawPowerUpSpawn(window, cue);
                break;
        }
    }
}

void VisualCueSystem::clear() {
    m_activeCues.clear();
}

void VisualCueSystem::drawHitIndicator(sf::RenderWindow& window, const VisualCue& cue) {
    // Red flash on screen edges
    float alpha = (cue.lifetime / cue.maxLifetime) * 150.0f;
    sf::Vector2u windowSize = window.getSize();

    // Create semi-transparent red overlay
    sf::RectangleShape overlay(sf::Vector2f(windowSize.x, windowSize.y));
    overlay.setFillColor(sf::Color(255, 0, 0, static_cast<sf::Uint8>(alpha)));
    
    window.draw(overlay);

    // Add pulsing edge borders
    float thickness = 10.0f;
    sf::RectangleShape border;
    border.setFillColor(sf::Color(255, 50, 50, static_cast<sf::Uint8>(alpha * 1.5f)));

    // Top border
    border.setSize(sf::Vector2f(windowSize.x, thickness));
    border.setPosition(0, 0);
    window.draw(border);

    // Bottom border
    border.setPosition(0, windowSize.y - thickness);
    window.draw(border);

    // Left border
    border.setSize(sf::Vector2f(thickness, windowSize.y));
    border.setPosition(0, 0);
    window.draw(border);

    // Right border
    border.setPosition(windowSize.x - thickness, 0);
    window.draw(border);
}

void VisualCueSystem::drawMissileWarning(sf::RenderWindow& window, const VisualCue& cue) {
    // Animated arrow from threat direction
    float alpha = std::min((cue.lifetime / cue.maxLifetime) * 255.0f, 255.0f);
    float pulse = std::sin(cue.lifetime * 10.0f) * 0.3f + 0.7f; // Pulsing effect

    // Calculate angle from direction
    float angle = std::atan2(cue.direction.y, cue.direction.x) * 180.0f / 3.14159f;

    // Draw warning icon at position
    sf::CircleShape warning(30.0f, 3); // Triangle
    warning.setFillColor(sf::Color(255, 255, 0, static_cast<sf::Uint8>(alpha * pulse)));
    warning.setOutlineThickness(3.0f);
    warning.setOutlineColor(sf::Color(255, 100, 0, static_cast<sf::Uint8>(alpha)));
    warning.setOrigin(30.0f, 30.0f);
    warning.setPosition(cue.position);
    warning.setRotation(angle);

    window.draw(warning);

    // Add "!" symbol
    sf::Font font;
    // Note: In production, load a font. For PoC, we'll use shapes
    sf::RectangleShape exclamation(sf::Vector2f(6.0f, 20.0f));
    exclamation.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(alpha)));
    exclamation.setPosition(cue.position.x - 3.0f, cue.position.y - 15.0f);
    window.draw(exclamation);

    sf::CircleShape dot(3.0f);
    dot.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(alpha)));
    dot.setPosition(cue.position.x - 3.0f, cue.position.y + 8.0f);
    window.draw(dot);
}

void VisualCueSystem::drawPowerUpSpawn(sf::RenderWindow& window, const VisualCue& cue) {
    // Radial glow with expanding rings
    float alpha = (cue.lifetime / cue.maxLifetime) * 200.0f;
    float expansion = (1.0f - (cue.lifetime / cue.maxLifetime)) * 50.0f;

    // Draw multiple rings
    for (int i = 0; i < 3; ++i) {
        float ringRadius = 20.0f + expansion + (i * 15.0f);
        float ringAlpha = alpha * (1.0f - (i * 0.3f));

        sf::CircleShape ring(ringRadius);
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineThickness(3.0f);
        ring.setOutlineColor(sf::Color(0, 255, 255, static_cast<sf::Uint8>(ringAlpha)));
        ring.setOrigin(ringRadius, ringRadius);
        ring.setPosition(cue.position);

        window.draw(ring);
    }

    // Central sparkle
    sf::CircleShape center(8.0f);
    center.setFillColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(alpha)));
    center.setOrigin(8.0f, 8.0f);
    center.setPosition(cue.position);
    window.draw(center);
}
