#pragma once

#include "GameObject.hpp"

/**
 * @brief Movable class adds movement capabilities to GameObject
 *
 * This class demonstrates the first level of inheritance.
 * Problem: What if we want some GameObjects to be movable and others not?
 */
class Movable : public GameObject {
protected:
    float speed;
    float accelerationX;
    float accelerationY;

public:
    Movable(float x = 0.0f, float y = 0.0f, int health = 100, float speed = 5.0f);
    virtual ~Movable() = default;

    // Movement methods
    void move(float deltaTime);
    void moveLeft(float deltaTime);
    void moveRight(float deltaTime);
    void moveUp(float deltaTime);
    void moveDown(float deltaTime);

    void setSpeed(float newSpeed) { speed = newSpeed; }
    float getSpeed() const { return speed; }

    void accelerate(float ax, float ay);
    void applyFriction(float friction);

    // Override update to include movement
    void update(float deltaTime) override;
};
