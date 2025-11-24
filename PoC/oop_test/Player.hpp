#pragma once

#include "Movable.hpp"

/**
 * @brief Player class representing the player's spaceship in R-Type
 *
 * This demonstrates deeper inheritance hierarchy.
 * Player inherits from Movable which inherits from GameObject.
 */
class Player : public Movable {
private:
    int score;
    int lives;
    float fireRate;
    float timeSinceLastShot;
    bool isInvincible;
    float invincibilityTimer;

public:
    Player(float x = 0.0f, float y = 0.0f);
    virtual ~Player() = default;

    // Player-specific methods
    void shoot();
    void addScore(int points);
    void loseLife();
    void gainLife();
    void activateInvincibility(float duration);

    // Getters
    int getScore() const { return score; }
    int getLives() const { return lives; }
    bool getIsInvincible() const { return isInvincible; }

    // Override methods
    void update(float deltaTime) override;
    void takeDamage(int damage) override;
    void render() const override;
};
