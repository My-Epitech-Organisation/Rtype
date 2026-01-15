/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_boss_visual_components - Unit tests for boss visual components
** Covers: BossSerpentComponent, BossVisualComponent
*/

#include <gtest/gtest.h>

#include "games/rtype/client/Components/BossSerpentComponent.hpp"
#include "games/rtype/client/Components/BossVisualComponent.hpp"

using namespace rtype::games::rtype::client;

// =============================================================================
// BossSerpentVisual Tests
// =============================================================================

class BossSerpentVisualTest : public ::testing::Test {
   protected:
    BossSerpentVisual visual;
};

TEST_F(BossSerpentVisualTest, DefaultValues) {
    EXPECT_EQ(visual.state, BossSerpentState::MOVE);
    EXPECT_EQ(visual.partType, BossSerpentPartType::HEAD);
    EXPECT_FLOAT_EQ(visual.animationTimer, 0.0f);
    EXPECT_EQ(visual.currentFrame, 0);
    EXPECT_FALSE(visual.isAttacking);
    EXPECT_FALSE(visual.isDying);
}

TEST_F(BossSerpentVisualTest, SpriteSheetConstants) {
    EXPECT_EQ(BossSerpentVisual::SHEET_WIDTH, 677);
    EXPECT_EQ(BossSerpentVisual::SHEET_HEIGHT, 369);
    EXPECT_EQ(BossSerpentVisual::FRAME_COUNT, 5);
    EXPECT_EQ(BossSerpentVisual::FRAME_WIDTH, 135);
    EXPECT_EQ(BossSerpentVisual::FRAME_HEIGHT, 369);
    EXPECT_FLOAT_EQ(BossSerpentVisual::ANIMATION_SPEED, 0.1f);
}

TEST_F(BossSerpentVisualTest, GetTextureNameHead) {
    visual.partType = BossSerpentPartType::HEAD;
    visual.isAttacking = false;
    EXPECT_STREQ(visual.getTextureName(), BossSerpentVisual::TEXTURE_HEAD);
}

TEST_F(BossSerpentVisualTest, GetTextureNameHeadAttacking) {
    visual.partType = BossSerpentPartType::HEAD;
    visual.isAttacking = true;
    EXPECT_STREQ(visual.getTextureName(), BossSerpentVisual::TEXTURE_ATTACK);
}

TEST_F(BossSerpentVisualTest, GetTextureNameBody) {
    visual.partType = BossSerpentPartType::BODY;
    EXPECT_STREQ(visual.getTextureName(), BossSerpentVisual::TEXTURE_BODY);
}

TEST_F(BossSerpentVisualTest, GetTextureNameTail) {
    visual.partType = BossSerpentPartType::TAIL;
    EXPECT_STREQ(visual.getTextureName(), BossSerpentVisual::TEXTURE_TAIL);
}

TEST_F(BossSerpentVisualTest, GetTextureRectFrame0) {
    visual.currentFrame = 0;
    int x, y, w, h;
    visual.getTextureRect(x, y, w, h);
    EXPECT_EQ(x, 0);
    EXPECT_EQ(y, 0);
    EXPECT_EQ(w, BossSerpentVisual::FRAME_WIDTH);
    EXPECT_EQ(h, BossSerpentVisual::FRAME_HEIGHT);
}

TEST_F(BossSerpentVisualTest, GetTextureRectFrame3) {
    visual.currentFrame = 3;
    int x, y, w, h;
    visual.getTextureRect(x, y, w, h);
    EXPECT_EQ(x, 3 * BossSerpentVisual::FRAME_WIDTH);
    EXPECT_EQ(y, 0);
    EXPECT_EQ(w, BossSerpentVisual::FRAME_WIDTH);
    EXPECT_EQ(h, BossSerpentVisual::FRAME_HEIGHT);
}

TEST_F(BossSerpentVisualTest, UpdateAnimationNoAdvance) {
    visual.currentFrame = 0;
    visual.animationTimer = 0.0f;
    visual.updateAnimation(0.05f);  // Less than ANIMATION_SPEED
    EXPECT_EQ(visual.currentFrame, 0);
    EXPECT_FLOAT_EQ(visual.animationTimer, 0.05f);
}

TEST_F(BossSerpentVisualTest, UpdateAnimationAdvanceFrame) {
    visual.currentFrame = 0;
    visual.animationTimer = 0.0f;
    visual.updateAnimation(0.15f);  // Greater than ANIMATION_SPEED
    EXPECT_EQ(visual.currentFrame, 1);
}

TEST_F(BossSerpentVisualTest, UpdateAnimationWraparound) {
    visual.currentFrame = 4;  // Last frame
    visual.animationTimer = 0.0f;
    visual.updateAnimation(0.15f);
    EXPECT_EQ(visual.currentFrame, 0);  // Should wrap to first frame
}

TEST_F(BossSerpentVisualTest, ResetAnimation) {
    visual.currentFrame = 3;
    visual.animationTimer = 0.05f;
    visual.resetAnimation();
    EXPECT_EQ(visual.currentFrame, 0);
    EXPECT_FLOAT_EQ(visual.animationTimer, 0.0f);
}

// =============================================================================
// BossSerpentBodyVisual Tests
// =============================================================================

class BossSerpentBodyVisualTest : public ::testing::Test {
   protected:
    BossSerpentBodyVisual bodyVisual;
};

TEST_F(BossSerpentBodyVisualTest, DefaultValues) {
    EXPECT_EQ(bodyVisual.partType, BossSerpentPartType::BODY);
    EXPECT_EQ(bodyVisual.segmentIndex, 0);
    EXPECT_FLOAT_EQ(bodyVisual.animationTimer, 0.0f);
    EXPECT_EQ(bodyVisual.currentFrame, 0);
}

TEST_F(BossSerpentBodyVisualTest, GetTextureNameBody) {
    bodyVisual.partType = BossSerpentPartType::BODY;
    EXPECT_STREQ(bodyVisual.getTextureName(), BossSerpentVisual::TEXTURE_BODY);
}

TEST_F(BossSerpentBodyVisualTest, GetTextureNameTail) {
    bodyVisual.partType = BossSerpentPartType::TAIL;
    EXPECT_STREQ(bodyVisual.getTextureName(), BossSerpentVisual::TEXTURE_TAIL);
}

TEST_F(BossSerpentBodyVisualTest, GetTextureRect) {
    bodyVisual.currentFrame = 2;
    int x, y, w, h;
    bodyVisual.getTextureRect(x, y, w, h);
    EXPECT_EQ(x, 2 * BossSerpentVisual::FRAME_WIDTH);
    EXPECT_EQ(y, 0);
    EXPECT_EQ(w, BossSerpentVisual::FRAME_WIDTH);
    EXPECT_EQ(h, BossSerpentVisual::FRAME_HEIGHT);
}

TEST_F(BossSerpentBodyVisualTest, UpdateAnimation) {
    bodyVisual.currentFrame = 0;
    bodyVisual.animationTimer = 0.0f;
    bodyVisual.updateAnimation(0.15f);
    EXPECT_EQ(bodyVisual.currentFrame, 1);
}

// =============================================================================
// BossVisualComponent Tests
// =============================================================================

class BossVisualComponentTest : public ::testing::Test {
   protected:
    BossVisualComponent visual;

    void SetUp() override {
        visual.moveTexture = "boss_move";
        visual.idleTexture = "boss_idle";
        visual.attackTexture = "boss_attack";
        visual.deathTexture = "boss_death";
    }
};

TEST_F(BossVisualComponentTest, DefaultValues) {
    BossVisualComponent defaultVisual;
    EXPECT_TRUE(defaultVisual.bossTypeId.empty());
    EXPECT_EQ(defaultVisual.partType, BossPartType::HEAD);
    EXPECT_EQ(defaultVisual.state, BossVisualState::MOVE);
    EXPECT_EQ(defaultVisual.frameWidth, 64);
    EXPECT_EQ(defaultVisual.frameHeight, 64);
    EXPECT_EQ(defaultVisual.frameCount, 1);
    EXPECT_FLOAT_EQ(defaultVisual.frameDuration, 0.1F);
    EXPECT_TRUE(defaultVisual.loop);
    EXPECT_FALSE(defaultVisual.isAttacking);
    EXPECT_FALSE(defaultVisual.isDying);
}

TEST_F(BossVisualComponentTest, GetCurrentTextureMoveState) {
    visual.state = BossVisualState::MOVE;
    EXPECT_EQ(visual.getCurrentTexture(), "boss_move");
}

TEST_F(BossVisualComponentTest, GetCurrentTextureIdleState) {
    visual.state = BossVisualState::IDLE;
    EXPECT_EQ(visual.getCurrentTexture(), "boss_idle");
}

TEST_F(BossVisualComponentTest, GetCurrentTextureIdleStateFallback) {
    visual.idleTexture = "";
    visual.state = BossVisualState::IDLE;
    EXPECT_EQ(visual.getCurrentTexture(), "boss_move");
}

TEST_F(BossVisualComponentTest, GetCurrentTextureAttackState) {
    visual.state = BossVisualState::ATTACK;
    EXPECT_EQ(visual.getCurrentTexture(), "boss_attack");
}

TEST_F(BossVisualComponentTest, GetCurrentTextureAttackStateFallback) {
    visual.attackTexture = "";
    visual.state = BossVisualState::ATTACK;
    EXPECT_EQ(visual.getCurrentTexture(), "boss_move");
}

TEST_F(BossVisualComponentTest, GetCurrentTextureDieState) {
    visual.state = BossVisualState::DIE;
    EXPECT_EQ(visual.getCurrentTexture(), "boss_death");
}

TEST_F(BossVisualComponentTest, GetCurrentTextureDieStateFallback) {
    visual.deathTexture = "";
    visual.state = BossVisualState::DIE;
    EXPECT_EQ(visual.getCurrentTexture(), "boss_move");
}

TEST_F(BossVisualComponentTest, GetCurrentTextureIsDyingOverride) {
    visual.state = BossVisualState::MOVE;
    visual.isDying = true;
    EXPECT_EQ(visual.getCurrentTexture(), "boss_death");
}

TEST_F(BossVisualComponentTest, GetCurrentTextureIsAttackingOverride) {
    visual.state = BossVisualState::MOVE;
    visual.isAttacking = true;
    EXPECT_EQ(visual.getCurrentTexture(), "boss_attack");
}

TEST_F(BossVisualComponentTest, GetTextureRect) {
    visual.frameWidth = 100;
    visual.frameHeight = 80;
    visual.currentFrame = 2;
    visual.spriteOffsetX = 10;

    int x, y, w, h;
    visual.getTextureRect(x, y, w, h);
    EXPECT_EQ(x, 10 + 2 * 100);  // spriteOffsetX + currentFrame * frameWidth
    EXPECT_EQ(y, 0);
    EXPECT_EQ(w, 100);
    EXPECT_EQ(h, 80);
}

TEST_F(BossVisualComponentTest, UpdateAnimationNoAdvance) {
    visual.currentFrame = 0;
    visual.animationTimer = 0.0F;
    visual.frameDuration = 0.1F;
    visual.updateAnimation(0.05F);
    EXPECT_EQ(visual.currentFrame, 0);
}

TEST_F(BossVisualComponentTest, UpdateAnimationAdvanceFrame) {
    visual.currentFrame = 0;
    visual.animationTimer = 0.0F;
    visual.frameDuration = 0.1F;
    visual.frameCount = 5;
    visual.loop = true;
    visual.updateAnimation(0.15F);
    EXPECT_EQ(visual.currentFrame, 1);
}

TEST_F(BossVisualComponentTest, UpdateAnimationLoopWraparound) {
    visual.currentFrame = 4;
    visual.animationTimer = 0.0F;
    visual.frameDuration = 0.1F;
    visual.frameCount = 5;
    visual.loop = true;
    visual.updateAnimation(0.15F);
    EXPECT_EQ(visual.currentFrame, 0);
}

TEST_F(BossVisualComponentTest, UpdateAnimationNoLoopStaysAtEnd) {
    visual.currentFrame = 4;
    visual.animationTimer = 0.0F;
    visual.frameDuration = 0.1F;
    visual.frameCount = 5;
    visual.loop = false;
    visual.updateAnimation(0.15F);
    EXPECT_EQ(visual.currentFrame, 4);  // Should stay at last frame
}

TEST_F(BossVisualComponentTest, ResetAnimation) {
    visual.currentFrame = 3;
    visual.animationTimer = 0.05F;
    visual.resetAnimation();
    EXPECT_EQ(visual.currentFrame, 0);
    EXPECT_FLOAT_EQ(visual.animationTimer, 0.0F);
}

TEST_F(BossVisualComponentTest, SetStateDifferent) {
    visual.state = BossVisualState::MOVE;
    visual.currentFrame = 2;
    visual.setState(BossVisualState::ATTACK);
    EXPECT_EQ(visual.state, BossVisualState::ATTACK);
    EXPECT_EQ(visual.currentFrame, 0);  // Animation should reset
}

TEST_F(BossVisualComponentTest, SetStateSame) {
    visual.state = BossVisualState::MOVE;
    visual.currentFrame = 2;
    visual.setState(BossVisualState::MOVE);
    EXPECT_EQ(visual.state, BossVisualState::MOVE);
    EXPECT_EQ(visual.currentFrame, 2);  // Should not reset
}

TEST_F(BossVisualComponentTest, SetAttackingTrue) {
    visual.isAttacking = false;
    visual.currentFrame = 2;
    visual.setAttacking(true);
    EXPECT_TRUE(visual.isAttacking);
    EXPECT_EQ(visual.currentFrame, 0);  // Animation should reset
}

TEST_F(BossVisualComponentTest, SetAttackingFalse) {
    visual.isAttacking = true;
    visual.currentFrame = 2;
    visual.setAttacking(false);
    EXPECT_FALSE(visual.isAttacking);
    EXPECT_EQ(visual.currentFrame, 0);  // Animation should reset
}

TEST_F(BossVisualComponentTest, SetAttackingSameValue) {
    visual.isAttacking = true;
    visual.currentFrame = 2;
    visual.setAttacking(true);
    EXPECT_TRUE(visual.isAttacking);
    EXPECT_EQ(visual.currentFrame, 2);  // Should not reset
}

TEST_F(BossVisualComponentTest, SetDyingTrue) {
    visual.isDying = false;
    visual.loop = true;
    visual.currentFrame = 2;
    visual.setDying(true);
    EXPECT_TRUE(visual.isDying);
    EXPECT_FALSE(visual.loop);  // Loop should be disabled
    EXPECT_EQ(visual.currentFrame, 0);  // Animation should reset
}

TEST_F(BossVisualComponentTest, SetDyingFalse) {
    visual.isDying = true;
    visual.loop = false;
    visual.currentFrame = 2;
    visual.setDying(false);
    EXPECT_FALSE(visual.isDying);
    EXPECT_FALSE(visual.loop);  // Loop stays false
    EXPECT_EQ(visual.currentFrame, 0);
}

TEST_F(BossVisualComponentTest, SetDyingSameValue) {
    visual.isDying = true;
    visual.currentFrame = 2;
    visual.setDying(true);
    EXPECT_TRUE(visual.isDying);
    EXPECT_EQ(visual.currentFrame, 2);  // Should not reset
}
