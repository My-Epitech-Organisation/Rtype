#pragma once

#include <string>

/**
 * @brief Simple game entity demonstrating time-scaled updates
 */
class GameEntity {
public:
    enum class Type {
        Player,
        Enemy,
        Projectile
    };

    GameEntity(const std::string& name, Type type, float speed);

    /**
     * @brief Update entity position based on scaled delta time
     * @param scaledDt Delta time affected by global time scale
     */
    void update(float scaledDt);

    /**
     * @brief Get entity's current position
     */
    float getPosition() const { return m_position; }

    /**
     * @brief Get entity's speed
     */
    float getSpeed() const { return m_speed; }

    /**
     * @brief Get entity's name
     */
    const std::string& getName() const { return m_name; }

    /**
     * @brief Get entity type
     */
    Type getType() const { return m_type; }

    /**
     * @brief Reset entity position
     */
    void reset() { m_position = 0.0f; }

private:
    std::string m_name;
    Type m_type;
    float m_position;
    float m_speed; // units per second
};
