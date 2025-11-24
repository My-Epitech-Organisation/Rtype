#include "GameObject.hpp"
#include <iostream>

GameObject::GameObject(float x, float y, int health)
    : x(x), y(y), velocityX(0.0f), velocityY(0.0f), health(health), type("GameObject") {
    std::cout << "[GameObject] Created at (" << x << ", " << y << ")" << std::endl;
}

void GameObject::setPosition(float newX, float newY) {
    x = newX;
    y = newY;
}

void GameObject::setVelocity(float vx, float vy) {
    velocityX = vx;
    velocityY = vy;
}

void GameObject::setHealth(int hp) {
    health = hp;
}

void GameObject::update(float deltaTime) {
    // Basic update - could be overridden by subclasses
    std::cout << "[GameObject] Update called for " << type << std::endl;
}

void GameObject::render() const {
    std::cout << "[GameObject] Rendering " << type << " at (" << x << ", " << y << ")" << std::endl;
}

void GameObject::takeDamage(int damage) {
    health -= damage;
    std::cout << "[GameObject] " << type << " took " << damage << " damage. Health: " << health << std::endl;
}

bool GameObject::isAlive() const {
    return health > 0;
}
