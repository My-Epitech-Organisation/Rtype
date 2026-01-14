/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** BossVisualComponent.cpp - Implementation of boss visual component methods
*/

#include "BossVisualComponent.hpp"

namespace rtype::games::rtype::client {

const std::string& BossVisualComponent::getCurrentTexture() const {
    if (isDying && !deathTexture.empty()) return deathTexture;
    if (isAttacking && !attackTexture.empty()) return attackTexture;
    switch (state) {
        case BossVisualState::IDLE:
            return idleTexture.empty() ? moveTexture : idleTexture;
        case BossVisualState::ATTACK:
            return attackTexture.empty() ? moveTexture : attackTexture;
        case BossVisualState::DIE:
            return deathTexture.empty() ? moveTexture : deathTexture;
        case BossVisualState::MOVE:
        default:
            return moveTexture;
    }
}

void BossVisualComponent::getTextureRect(int& x, int& y, int& w, int& h) const {
    x = spriteOffsetX + (currentFrame * frameWidth);
    y = 0;
    w = frameWidth;
    h = frameHeight;
}

void BossVisualComponent::updateAnimation(float deltaTime) {
    animationTimer += deltaTime;
    if (animationTimer >= frameDuration) {
        animationTimer -= frameDuration;
        if (loop || currentFrame < frameCount - 1) {
            currentFrame = (currentFrame + 1) % frameCount;
        }
    }
}

void BossVisualComponent::resetAnimation() {
    currentFrame = 0;
    animationTimer = 0.0F;
}

void BossVisualComponent::setState(BossVisualState newState) {
    if (state != newState) {
        state = newState;
        resetAnimation();
    }
}

void BossVisualComponent::setAttacking(bool attacking) {
    if (isAttacking != attacking) {
        isAttacking = attacking;
        resetAnimation();
    }
}

void BossVisualComponent::setDying(bool dying) {
    if (isDying != dying) {
        isDying = dying;
        loop = false;
        resetAnimation();
    }
}

}  // namespace rtype::games::rtype::client
