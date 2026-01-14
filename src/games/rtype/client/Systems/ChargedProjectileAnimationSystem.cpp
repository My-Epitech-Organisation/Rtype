/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ChargedProjectileAnimationSystem - Implementation
*/

#include "ChargedProjectileAnimationSystem.hpp"

#include "../Components/TextureRectComponent.hpp"
#include "Logger/Macros.hpp"
#include "games/rtype/shared/Components/ChargedProjectileComponent.hpp"

namespace rtype::games::rtype::client {

namespace shared = ::rtype::games::rtype::shared;

namespace {
constexpr int kNumFrames = 10;

struct FrameData {
    int x;       // X position in spritesheet
    int y;       // Y position in spritesheet
    int width;   // Frame width
    int height;  // Frame height
};

// Actual frame positions detected from the spritesheet
constexpr FrameData kFrames[kNumFrames] = {
    {6, 168, 37, 33},    // Frame 0: 37x33
    {47, 163, 50, 43},   // Frame 1: 50x43
    {101, 158, 62, 53},  // Frame 2: 62x53
    {166, 150, 78, 68},  // Frame 3: 78x68
    {246, 146, 90, 77},  // Frame 4: 90x77 (max)
    {337, 146, 91, 77},  // Frame 5: 91x77 (max)
    {429, 150, 78, 68},  // Frame 6: 78x68
    {510, 158, 62, 53},  // Frame 7: 62x53
    {577, 163, 49, 43},  // Frame 8: 49x43
    {631, 168, 36, 33},  // Frame 9: 36x33
};
}  // namespace

ChargedProjectileAnimationSystem::ChargedProjectileAnimationSystem()
    : ASystem("ChargedProjectileAnimationSystem") {}

void ChargedProjectileAnimationSystem::update(ECS::Registry& registry,
                                              float dt) {
    auto view =
        registry.view<shared::ChargedProjectileComponent, TextureRect>();

    size_t count = 0;
    view.each([&count, dt](ECS::Entity entity,
                           shared::ChargedProjectileComponent& charged,
                           TextureRect& texRect) {
        ++count;
        charged.updateAnimation(dt);

        int32_t frame = charged.getFrame() % kNumFrames;

        const auto& frameData = kFrames[frame];

        texRect.rect.left = frameData.x;
        texRect.rect.top = frameData.y;
        texRect.rect.width = frameData.width;
        texRect.rect.height = frameData.height;
    });

    if (count == 0) {
        auto chargedOnly = registry.view<shared::ChargedProjectileComponent>();
        size_t chargedCount = 0;
        chargedOnly.each(
            [&chargedCount](ECS::Entity, shared::ChargedProjectileComponent&) {
                ++chargedCount;
            });
    }
}

}  // namespace rtype::games::rtype::client
