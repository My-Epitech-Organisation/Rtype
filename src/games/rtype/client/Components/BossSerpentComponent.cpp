/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** BossSerpentComponent.cpp - Implementation of boss serpent visual methods
*/

#include "games/rtype/client/Components/BossSerpentComponent.hpp"

const char* BossSerpentVisual::getTextureName() const {
    if (partType == BossSerpentPartType::HEAD) {
        return isAttacking ? TEXTURE_ATTACK : TEXTURE_HEAD;
    } else if (partType == BossSerpentPartType::TAIL) {
        return TEXTURE_TAIL;
    }
    return TEXTURE_BODY;
}

void BossSerpentVisual::getTextureRect(int& x, int& y, int& w, int& h) const {
    x = currentFrame * FRAME_WIDTH;
    y = 0;
    w = FRAME_WIDTH;
    h = FRAME_HEIGHT;
}

void BossSerpentVisual::updateAnimation(float deltaTime) {
    animationTimer += deltaTime;
    if (animationTimer >= ANIMATION_SPEED) {
        animationTimer -= ANIMATION_SPEED;
        currentFrame = (currentFrame + 1) % FRAME_COUNT;
    }
}

void BossSerpentVisual::resetAnimation() {
    currentFrame = 0;
    animationTimer = 0.0f;
}

const char* BossSerpentBodyVisual::getTextureName() const {
    return (partType == BossSerpentPartType::TAIL)
               ? BossSerpentVisual::TEXTURE_TAIL
               : BossSerpentVisual::TEXTURE_BODY;
}

void BossSerpentBodyVisual::getTextureRect(int& x, int& y, int& w, int& h) const {
    x = currentFrame * BossSerpentVisual::FRAME_WIDTH;
    y = 0;
    w = BossSerpentVisual::FRAME_WIDTH;
    h = BossSerpentVisual::FRAME_HEIGHT;
}

void BossSerpentBodyVisual::updateAnimation(float deltaTime) {
    animationTimer += deltaTime;
    if (animationTimer >= BossSerpentVisual::ANIMATION_SPEED) {
        animationTimer -= BossSerpentVisual::ANIMATION_SPEED;
        currentFrame = (currentFrame + 1) % BossSerpentVisual::FRAME_COUNT;
    }
}
