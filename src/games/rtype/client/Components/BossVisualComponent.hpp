#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_BOSSVISUALCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_BOSSVISUALCOMPONENT_HPP_

#include <cstdint>
#include <string>

namespace rtype::games::rtype::client {

enum class BossVisualState : uint8_t {
    IDLE = 0,
    MOVE = 1,
    ATTACK = 2,
    DIE = 3
};

enum class BossPartType : uint8_t { HEAD = 0, BODY = 1, TAIL = 2, CUSTOM = 3 };

struct BossVisualComponent {
    std::string bossTypeId;
    BossPartType partType = BossPartType::HEAD;
    BossVisualState state = BossVisualState::MOVE;

    std::string idleTexture;
    std::string moveTexture;
    std::string attackTexture;
    std::string deathTexture;

    int32_t frameWidth = 64;
    int32_t frameHeight = 64;
    int32_t frameCount = 1;
    float frameDuration = 0.1F;
    bool loop = true;

    float animationTimer = 0.0F;
    int32_t currentFrame = 0;

    float scaleX = 1.0F;
    float scaleY = 1.0F;

    bool enableRotation = true;
    float rotationSmoothing = 0.15F;
    float rotationOffset = 0.0F;

    bool isAttacking = false;
    bool isDying = false;

    int32_t spriteOffsetX =
        0;  // Offset to compensate for left padding in spritesheet

    int32_t segmentIndex = 0;
    std::string customPartId;

    [[nodiscard]] const std::string& getCurrentTexture() const {
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

    void getTextureRect(int& x, int& y, int& w, int& h) const {
        x = spriteOffsetX + (currentFrame * frameWidth);
        y = 0;
        w = frameWidth;
        h = frameHeight;
    }

    void updateAnimation(float deltaTime) {
        animationTimer += deltaTime;
        if (animationTimer >= frameDuration) {
            animationTimer -= frameDuration;
            if (loop || currentFrame < frameCount - 1) {
                currentFrame = (currentFrame + 1) % frameCount;
            }
        }
    }

    void resetAnimation() {
        currentFrame = 0;
        animationTimer = 0.0F;
    }

    void setState(BossVisualState newState) {
        if (state != newState) {
            state = newState;
            resetAnimation();
        }
    }

    void setAttacking(bool attacking) {
        if (isAttacking != attacking) {
            isAttacking = attacking;
            resetAnimation();
        }
    }

    void setDying(bool dying) {
        if (isDying != dying) {
            isDying = dying;
            loop = false;
            resetAnimation();
        }
    }
};

}  // namespace rtype::games::rtype::client

#endif
