#pragma once

#include "GameObject.hpp"

/**
 * @brief Demonstrates the Diamond Inheritance Problem
 * 
 * Scenario: We want both Player and some Enemies to have shooting capabilities.
 * We also want some objects to be Damageable (can take damage from environment).
 * 
 * The Diamond Problem occurs when:
 *    GameObject
 *      /  \
 *  Shootable  Damageable
 *      \  /
 *    PowerUp (can shoot AND take environmental damage?)
 * 
 * Both Shootable and Damageable might inherit from GameObject,
 * causing ambiguity and duplication.
 */

// Attempt 1: Create a Shootable class
class Shootable : public GameObject {
protected:
    float fireRate;
    float timeSinceLastShot;
    int ammunition;

public:
    Shootable(float x = 0.0f, float y = 0.0f, float fireRate = 0.5f);
    virtual void shoot();
    virtual void reload();
    void update(float deltaTime) override;
};

// Attempt 2: Create a Damageable class (for environmental hazards)
class Damageable : public GameObject {
protected:
    float armor;
    bool canBeDestroyed;

public:
    Damageable(float x = 0.0f, float y = 0.0f, float armor = 0.0f);
    void takeDamage(int damage) override;
    void setArmor(float newArmor) { armor = newArmor; }
};

/**
 * @brief PowerUp that can shoot (?)  and be damaged (?)
 * 
 * Problem: We can't inherit from both Shootable and Damageable
 * without getting into the diamond inheritance problem:
 * - Both inherit from GameObject
 * - PowerUp would have TWO copies of GameObject's members
 * - Which x, y, health should we use?
 * 
 * Solution in OOP: Virtual inheritance (complex and error-prone)
 */

// This would cause issues:
// class ShootingPowerUp : public Shootable, public Damageable {
//     // Diamond problem! Which GameObject::x and GameObject::y do we use?
// };

/**
 * @brief Example of trying to avoid the problem with composition
 * 
 * But this leads to wrapper hell - we need to forward all methods
 */
class ShootingPowerUp : public Shootable {
private:
    // We can't inherit Damageable, so we compose it
    // But now we need to manually forward all methods!
    float armor;  // Duplicated from Damageable
    bool canBeDestroyed;  // Duplicated from Damageable

public:
    ShootingPowerUp(float x = 0.0f, float y = 0.0f);
    
    // Need to reimplement or wrap Damageable functionality
    void setArmor(float newArmor) { armor = newArmor; }
    // ... and many more forwarding methods
};

/**
 * @brief Complexity Analysis Notes
 * 
 * PROBLEMS IDENTIFIED:
 * 
 * 1. CODE DUPLICATION
 *    - Player::shoot() and Enemy::shoot() are nearly identical
 *    - Can't easily share behavior without complex inheritance
 * 
 * 2. FRAGILE BASE CLASS
 *    - Changes to GameObject affect all subclasses
 *    - Deep hierarchies (GameObject -> Movable -> Player) are hard to understand
 *    - Boss has 4 levels of inheritance!
 * 
 * 3. DIAMOND INHERITANCE
 *    - Can't combine behaviors (Shootable + Damageable) easily
 *    - Virtual inheritance is complex and has performance costs
 *    - Forces awkward design decisions
 * 
 * 4. INFLEXIBILITY
 *    - Hard to add new behaviors to existing objects at runtime
 *    - Can't make a non-moving GameObject suddenly movable
 *    - All capabilities must be determined at compile-time
 * 
 * 5. TIGHT COUPLING
 *    - Player depends on Movable depends on GameObject
 *    - Hard to test in isolation
 *    - Changes ripple through the hierarchy
 * 
 * 6. BLOATED CLASSES
 *    - Player has ALL GameObject and Movable members, even if not needed
 *    - Memory waste for unused features
 *    - Large vtables for virtual functions
 */
