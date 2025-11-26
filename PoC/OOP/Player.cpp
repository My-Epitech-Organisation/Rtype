#include "Player.hpp"
#include <iostream>

Player::Player(float x, float y)
    : Movable(x, y, 100, 10.0f), score(0), lives(3), fireRate(0.2f),
      timeSinceLastShot(0.0f), isInvincible(false), invincibilityTimer(0.0f) {
    type = "Player";
    std::cout << "[Player] Created at (" << x << ", " << y << ") with " << lives << " lives" << std::endl;
}

void Player::shoot() {
    if (timeSinceLastShot >= fireRate) {
        std::cout << "[Player] PEW PEW! Shooting from (" << x << ", " << y << ")" << std::endl;
        timeSinceLastShot = 0.0f;
    } else {
        std::cout << "[Player] Weapon cooling down... " << (fireRate - timeSinceLastShot) << "s remaining" << std::endl;
    }
}

void Player::addScore(int points) {
    score += points;
    std::cout << "[Player] Score increased by " << points << ". Total: " << score << std::endl;
}

void Player::loseLife() {
    if (lives > 0) {
        lives--;
        std::cout << "[Player] Lost a life! Remaining lives: " << lives << std::endl;
        if (lives > 0) {
            activateInvincibility(2.0f); // 2 seconds of invincibility
            health = 100; // Restore health
        } else {
            std::cout << "[Player] GAME OVER!" << std::endl;
        }
    }
}

void Player::gainLife() {
    lives++;
    std::cout << "[Player] Gained a life! Total lives: " << lives << std::endl;
}

void Player::activateInvincibility(float duration) {
    isInvincible = true;
    invincibilityTimer = duration;
    std::cout << "[Player] Invincibility activated for " << duration << " seconds!" << std::endl;
}

void Player::update(float deltaTime) {
    Movable::update(deltaTime);

    // Update shooting cooldown
    timeSinceLastShot += deltaTime;

    // Update invincibility
    if (isInvincible) {
        invincibilityTimer -= deltaTime;
        if (invincibilityTimer <= 0.0f) {
            isInvincible = false;
            std::cout << "[Player] Invincibility expired!" << std::endl;
        }
    }
}

void Player::takeDamage(int damage) {
    if (isInvincible) {
        std::cout << "[Player] Invincible! No damage taken." << std::endl;
        return;
    }

    GameObject::takeDamage(damage);

    if (!isAlive()) {
        loseLife();
    }
}

void Player::render() const {
    std::cout << "[Player] Rendering player at (" << x << ", " << y << ")";
    std::cout << " | HP: " << health << " | Lives: " << lives << " | Score: " << score;
    if (isInvincible) {
        std::cout << " | INVINCIBLE!";
    }
    std::cout << std::endl;
}
