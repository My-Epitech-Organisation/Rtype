#include "Movable.hpp"
#include <iostream>
#include <cmath>

Movable::Movable(float x, float y, int health, float speed)
    : GameObject(x, y, health), speed(speed), accelerationX(0.0f), accelerationY(0.0f) {
    type = "Movable";
    std::cout << "[Movable] Created with speed " << speed << std::endl;
}

void Movable::move(float deltaTime) {
    // Apply acceleration to velocity
    velocityX += accelerationX * deltaTime;
    velocityY += accelerationY * deltaTime;

    // Apply velocity to position
    x += velocityX * deltaTime;
    y += velocityY * deltaTime;
}

void Movable::moveLeft(float deltaTime) {
    velocityX = -speed;
    move(deltaTime);
}

void Movable::moveRight(float deltaTime) {
    velocityX = speed;
    move(deltaTime);
}

void Movable::moveUp(float deltaTime) {
    velocityY = -speed;
    move(deltaTime);
}

void Movable::moveDown(float deltaTime) {
    velocityY = speed;
    move(deltaTime);
}

void Movable::accelerate(float ax, float ay) {
    accelerationX = ax;
    accelerationY = ay;
}

void Movable::applyFriction(float friction) {
    velocityX *= (1.0f - friction);
    velocityY *= (1.0f - friction);

    // Stop completely if velocity is very small
    if (std::abs(velocityX) < 0.01f) velocityX = 0.0f;
    if (std::abs(velocityY) < 0.01f) velocityY = 0.0f;
}

void Movable::update(float deltaTime) {
    GameObject::update(deltaTime);
    move(deltaTime);
    applyFriction(0.1f); // Apply some friction
    std::cout << "[Movable] Position: (" << x << ", " << y << ")" << std::endl;
}
