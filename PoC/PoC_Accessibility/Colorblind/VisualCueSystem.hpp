#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

/**
 * @brief Types of visual cues for audio events
 */
enum class VisualCueType {
    HitIndicator,       // Player took damage
    MissileWarning,     // Missile incoming
    PowerUpSpawn,       // Power-up appeared
};

/**
 * @brief Represents a single visual cue instance
 */
struct VisualCue {
    VisualCueType type;
    sf::Vector2f position;
    float lifetime;      // Time remaining in seconds
    float maxLifetime;   // Original lifetime for fade calculation
    sf::Vector2f direction; // For directional indicators

    VisualCue(VisualCueType t, sf::Vector2f pos, float life, sf::Vector2f dir = {0, 0})
        : type(t), position(pos), lifetime(life), maxLifetime(life), direction(dir) {}
};

/**
 * @brief Manages visual cues for important audio events
 *
 * This system provides visual alternatives to sound effects, helping
 * players who cannot perceive audio cues.
 */
class VisualCueSystem {
public:
    VisualCueSystem() = default;

    /**
     * @brief Trigger a visual cue
     * @param type Type of cue to display
     * @param position World position where the event occurred
     * @param direction Optional direction vector (for directional cues)
     */
    void triggerCue(VisualCueType type, sf::Vector2f position,
                   sf::Vector2f direction = {0, 0});

    /**
     * @brief Update all active cues
     * @param deltaTime Time elapsed since last update in seconds
     */
    void update(float deltaTime);

    /**
     * @brief Draw all active cues
     * @param window Target window
     */
    void draw(sf::RenderWindow& window);

    /**
     * @brief Clear all active cues
     */
    void clear();

private:
    std::vector<VisualCue> m_activeCues;

    /**
     * @brief Draw a hit indicator (red flash on screen edges)
     */
    void drawHitIndicator(sf::RenderWindow& window, const VisualCue& cue);

    /**
     * @brief Draw a missile warning (animated arrow from threat direction)
     */
    void drawMissileWarning(sf::RenderWindow& window, const VisualCue& cue);

    /**
     * @brief Draw a power-up spawn indicator (radial glow)
     */
    void drawPowerUpSpawn(sf::RenderWindow& window, const VisualCue& cue);
};
