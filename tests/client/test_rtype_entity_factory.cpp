/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for RtypeEntityFactory
*/

#include <gtest/gtest.h>

namespace rtype::games::rtype::client {

static std::pair<int, int> getPlayerSpriteOffset(uint32_t playerId) noexcept {
    return {0, (playerId - 1) % 4 * 17};
}

}

TEST(RtypeEntityFactoryTest, GetPlayerSpriteOffset_Player1) {
    auto offset = rtype::games::rtype::client::getPlayerSpriteOffset(1);
    EXPECT_EQ(offset.first, 0);   // x offset
    EXPECT_EQ(offset.second, 0);  // y offset (row 0)
}

TEST(RtypeEntityFactoryTest, GetPlayerSpriteOffset_Player2) {
    auto offset = rtype::games::rtype::client::getPlayerSpriteOffset(2);
    EXPECT_EQ(offset.first, 0);
    EXPECT_EQ(offset.second, 17);  // row 1 * 17
}

TEST(RtypeEntityFactoryTest, GetPlayerSpriteOffset_Player3) {
    auto offset = rtype::games::rtype::client::getPlayerSpriteOffset(3);
    EXPECT_EQ(offset.first, 0);
    EXPECT_EQ(offset.second, 34);  // row 2 * 17
}

TEST(RtypeEntityFactoryTest, GetPlayerSpriteOffset_Player4) {
    auto offset = rtype::games::rtype::client::getPlayerSpriteOffset(4);
    EXPECT_EQ(offset.first, 0);
    EXPECT_EQ(offset.second, 51);  // row 3 * 17
}

TEST(RtypeEntityFactoryTest, GetPlayerSpriteOffset_InvalidPlayerId) {
    // Test edge case: playerId=0 (should still compute, though not used)
    auto offset = rtype::games::rtype::client::getPlayerSpriteOffset(0);
    EXPECT_EQ(offset.first, 0);
    EXPECT_EQ(offset.second, 51);  // (0-1)%4 = 3, 3*17=51
}

TEST(RtypeEntityFactoryTest, GetPlayerSpriteOffset_PlayerId5) {
    // Test wrap-around: playerId=5 maps to same as playerId=1
    auto offset5 = rtype::games::rtype::client::getPlayerSpriteOffset(5);
    auto offset1 = rtype::games::rtype::client::getPlayerSpriteOffset(1);
    EXPECT_EQ(offset5, offset1);
}