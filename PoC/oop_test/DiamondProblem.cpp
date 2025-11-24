#include "DiamondProblem.hpp"
#include <iostream>

// ============================================================================
// Shootable Implementation
// ============================================================================

Shootable::Shootable(float x, float y, float fireRate)
    : GameObject(x, y, 100), fireRate(fireRate), timeSinceLastShot(0.0f), ammunition(100) {
    type = "Shootable";
    std::cout << "[Shootable] Created with fire rate " << fireRate << std::endl;
}

void Shootable::shoot() {
    if (ammunition > 0 && timeSinceLastShot >= fireRate) {
        std::cout << "[Shootable] BANG! Ammo: " << ammunition << std::endl;
        ammunition--;
        timeSinceLastShot = 0.0f;
    } else if (ammunition <= 0) {
        std::cout << "[Shootable] Out of ammo! Reload needed." << std::endl;
    }
}

void Shootable::reload() {
    ammunition = 100;
    std::cout << "[Shootable] Reloaded! Ammo: " << ammunition << std::endl;
}

void Shootable::update(float deltaTime) {
    GameObject::update(deltaTime);
    timeSinceLastShot += deltaTime;
}

// ============================================================================
// Damageable Implementation
// ============================================================================

Damageable::Damageable(float x, float y, float armor)
    : GameObject(x, y, 100), armor(armor), canBeDestroyed(true) {
    type = "Damageable";
    std::cout << "[Damageable] Created with armor " << armor << std::endl;
}

void Damageable::takeDamage(int damage) {
    float actualDamage = damage * (1.0f - armor / 100.0f);
    health -= static_cast<int>(actualDamage);
    std::cout << "[Damageable] Took " << actualDamage << " damage (reduced by armor). Health: " << health << std::endl;
    
    if (!isAlive() && canBeDestroyed) {
        std::cout << "[Damageable] DESTROYED!" << std::endl;
    }
}

// ============================================================================
// ShootingPowerUp Implementation
// ============================================================================

ShootingPowerUp::ShootingPowerUp(float x, float y)
    : Shootable(x, y, 2.0f), armor(50.0f), canBeDestroyed(true) {
    type = "ShootingPowerUp";
    std::cout << "[ShootingPowerUp] Created - demonstrates composition workaround" << std::endl;
    std::cout << "[ShootingPowerUp] Note: We duplicated armor and canBeDestroyed!" << std::endl;
}

/**
 * COMPLEXITY ANALYSIS SUMMARY:
 * 
 * Lines of Code: ~200+ for this simple example
 * Inheritance Depth: Up to 4 levels (GameObject -> Movable -> Enemy -> Boss)
 * Code Duplication: shoot() method duplicated in Player and Enemy
 * 
 * Problems Encountered:
 * 1. Can't make PowerUp both Shootable AND Damageable without diamond problem
 * 2. Had to duplicate armor/canBeDestroyed fields
 * 3. Deep inheritance makes code hard to follow
 * 4. Can't change behavior at runtime
 * 5. Testing requires mocking entire inheritance chain
 * 
 * Maintenance Concerns:
 * - Adding a new feature (e.g., "Teleportable") requires new class in hierarchy
 * - Changing GameObject affects ALL entities
 * - Can't easily share behavior between unrelated classes
 * - Memory overhead from virtual function tables
 * 
 * vs ECS Benefits (theoretical):
 * - Components can be mixed freely
 * - No inheritance hierarchies
 * - Data-oriented design (better cache coherency)
 * - Runtime composition
 * - Easy to add/remove behaviors
 */
