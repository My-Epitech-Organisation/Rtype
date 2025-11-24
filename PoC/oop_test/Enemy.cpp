#include "Enemy.hpp"
#include <iostream>
#include <cmath>

Enemy::Enemy(float x, float y, std::string enemyType)
    : Movable(x, y, 50, 3.0f), scoreValue(100), fireRate(1.0f),
      timeSinceLastShot(0.0f), enemyType(enemyType) {
    type = "Enemy";
    std::cout << "[Enemy] Created " << enemyType << " enemy at (" << x << ", " << y << ")" << std::endl;
}

// NOTE: This code is DUPLICATED from Player::shoot()
// In OOP, it's hard to share behavior between Player and Enemy without complex inheritance
void Enemy::shoot() {
    if (timeSinceLastShot >= fireRate) {
        std::cout << "[Enemy] Enemy shooting from (" << x << ", " << y << ")" << std::endl;
        timeSinceLastShot = 0.0f;
    }
}

void Enemy::updateAI(float deltaTime) {
    // Simple AI: move towards left (towards player)
    velocityX = -speed;
    
    // Randomly shoot
    if (timeSinceLastShot >= fireRate) {
        shoot();
    }
}

void Enemy::update(float deltaTime) {
    Movable::update(deltaTime);
    timeSinceLastShot += deltaTime;
    updateAI(deltaTime);
}

void Enemy::render() const {
    std::cout << "[Enemy] Rendering " << enemyType << " enemy at (" << x << ", " << y << ")";
    std::cout << " | HP: " << health << " | Value: " << scoreValue << std::endl;
}

// ============================================================================
// Boss Implementation
// ============================================================================

Boss::Boss(float x, float y)
    : Enemy(x, y, "BOSS"), phase(1), hasShield(true), shieldStrength(100.0f) {
    health = 500;
    scoreValue = 5000;
    speed = 1.5f;
    fireRate = 0.5f;
    type = "Boss";
    std::cout << "[Boss] BOSS SPAWNED at (" << x << ", " << y << ")" << std::endl;
}

void Boss::updateAI(float deltaTime) {
    // More complex AI based on phase
    switch (phase) {
        case 1:
            // Phase 1: Move in a pattern
            velocityY = 2.0f * std::sin(x * 0.1f);
            velocityX = -speed * 0.5f;
            fireRate = 0.5f;
            break;
        case 2:
            // Phase 2: More aggressive
            velocityY = 3.0f * std::sin(x * 0.2f);
            velocityX = -speed;
            fireRate = 0.3f;
            break;
        case 3:
            // Phase 3: Desperate, very fast shooting
            velocityY = 4.0f * std::sin(x * 0.3f);
            velocityX = -speed * 1.5f;
            fireRate = 0.1f;
            break;
    }

    // Update phase based on health
    if (health < 300 && phase == 1) {
        phase = 2;
        std::cout << "[Boss] PHASE 2 ACTIVATED!" << std::endl;
    } else if (health < 150 && phase == 2) {
        phase = 3;
        std::cout << "[Boss] PHASE 3 - FINAL FORM!" << std::endl;
    }

    if (timeSinceLastShot >= fireRate) {
        shoot();
    }
}

void Boss::takeDamage(int damage) {
    if (hasShield && shieldStrength > 0) {
        shieldStrength -= damage;
        std::cout << "[Boss] Shield absorbed " << damage << " damage. Shield: " << shieldStrength << std::endl;
        if (shieldStrength <= 0) {
            hasShield = false;
            std::cout << "[Boss] SHIELD DESTROYED!" << std::endl;
        }
    } else {
        Enemy::takeDamage(damage);
    }
}

void Boss::render() const {
    std::cout << "[Boss] Rendering BOSS at (" << x << ", " << y << ")";
    std::cout << " | HP: " << health << " | Phase: " << phase;
    if (hasShield) {
        std::cout << " | Shield: " << shieldStrength;
    }
    std::cout << std::endl;
}
