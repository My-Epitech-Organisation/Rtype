#pragma once

#include "Movable.hpp"

/**
 * @brief Enemy class representing enemy spaceships in R-Type
 * 
 * This demonstrates the diamond inheritance problem:
 * - Enemy needs to be Movable (inherits from Movable)
 * - But what if we want to share shooting behavior with Player?
 * - We'd need a Shootable class, but then both Player and Enemy would inherit from it
 * - This creates complex inheritance hierarchies
 */
class Enemy : public Movable {
protected:
    int scoreValue;
    float fireRate;
    float timeSinceLastShot;
    std::string enemyType;

public:
    Enemy(float x = 0.0f, float y = 0.0f, std::string enemyType = "basic");
    virtual ~Enemy() = default;

    // Enemy-specific methods
    void shoot(); // NOTE: This is duplicated from Player! OOP Problem #1
    int getScoreValue() const { return scoreValue; }

    // AI behavior
    virtual void updateAI(float deltaTime);

    // Override methods
    void update(float deltaTime) override;
    void render() const override;
};

/**
 * @brief Boss enemy with more complex behavior
 * 
 * Demonstrates the "fragile base class" problem:
 * - If we change Movable or GameObject, it affects Boss
 * - Deep inheritance makes it hard to understand what Boss actually does
 */
class Boss : public Enemy {
private:
    int phase;
    bool hasShield;
    float shieldStrength;

public:
    Boss(float x = 0.0f, float y = 0.0f);
    virtual ~Boss() = default;

    void updateAI(float deltaTime) override;
    void takeDamage(int damage) override;
    void render() const override;
};
