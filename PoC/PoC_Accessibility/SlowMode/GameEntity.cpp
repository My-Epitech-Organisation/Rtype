#include "GameEntity.hpp"

GameEntity::GameEntity(const std::string& name, Type type, float speed)
    : m_name(name)
    , m_type(type)
    , m_position(0.0f)
    , m_speed(speed)
{
}

void GameEntity::update(float scaledDt) {
    // Movement is affected by scaled delta time
    // This means slow mode will slow down all entities proportionally
    m_position += m_speed * scaledDt;
}
