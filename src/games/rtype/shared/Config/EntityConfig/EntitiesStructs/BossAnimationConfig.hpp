#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace rtype::games::rtype::shared {

struct BossSpriteConfig {
    std::string textureName;
    int32_t frameWidth = 64;
    int32_t frameHeight = 64;
    int32_t frameCount = 1;
    float frameDuration = 0.1F;
    bool loop = true;
    int32_t spriteOffsetX = 0;  // Left padding offset in spritesheet
};

struct BossPartAnimationConfig {
    std::string partId;
    std::string partType;
    BossSpriteConfig idleSprite;
    BossSpriteConfig moveSprite;
    BossSpriteConfig attackSprite;
    BossSpriteConfig deathSprite;
    float scaleX = 1.0F;
    float scaleY = 1.0F;
    bool enableRotation = true;
    float rotationSmoothing = 0.15F;
    float rotationOffset = 0.0F;
};

struct BossMovementConfig {
    float amplitude = 150.0F;
    float frequency = 0.5F;
    float horizontalAmplitude = 100.0F;
    float verticalAmplitude = 200.0F;
    bool enablePositionHistory = false;
    int32_t maxPositionHistory = 500;
    float minRecordDistance = 3.0F;
    float segmentSpacing = 100.0F;
};

struct BossAnimationConfig {
    std::string bossId;
    BossPartAnimationConfig headAnimation;
    BossPartAnimationConfig bodyAnimation;
    BossPartAnimationConfig tailAnimation;
    std::unordered_map<std::string, BossPartAnimationConfig> customParts;
    BossMovementConfig movement;
    float spawnX = 0.0F;
    float spawnY = 0.0F;
    bool useRelativeSpawn = true;
    float spawnOffsetX = -200.0F;
    float spawnOffsetY = 0.0F;

    [[nodiscard]] bool hasBodySegments() const noexcept {
        return !bodyAnimation.partId.empty();
    }

    [[nodiscard]] bool hasTail() const noexcept {
        return !tailAnimation.partId.empty();
    }

    [[nodiscard]] const BossPartAnimationConfig* getPartConfig(
        const std::string& partType) const noexcept {
        if (partType == "head") return &headAnimation;
        if (partType == "body") return &bodyAnimation;
        if (partType == "tail") return &tailAnimation;
        auto it = customParts.find(partType);
        if (it != customParts.end()) return &it->second;
        return nullptr;
    }
};

}  // namespace rtype::games::rtype::shared
