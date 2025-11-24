#pragma once

#include <string>

/**
 * @brief Base class for all game objects in the R-Type game
 * 
 * This demonstrates the traditional OOP approach where all game entities
 * inherit from a common base class.
 */
class GameObject {
protected:
    float x;
    float y;
    float velocityX;
    float velocityY;
    int health;
    std::string type;

public:
    GameObject(float x = 0.0f, float y = 0.0f, int health = 100);
    virtual ~GameObject() = default;

    // Getters
    float getX() const { return x; }
    float getY() const { return y; }
    float getVelocityX() const { return velocityX; }
    float getVelocityY() const { return velocityY; }
    int getHealth() const { return health; }
    std::string getType() const { return type; }

    // Setters
    void setPosition(float newX, float newY);
    void setVelocity(float vx, float vy);
    void setHealth(int hp);

    // Virtual methods that subclasses may override
    virtual void update(float deltaTime);
    virtual void render() const;
    virtual void takeDamage(int damage);
    virtual bool isAlive() const;
};
