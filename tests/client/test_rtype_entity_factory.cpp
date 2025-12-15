/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for RtypeEntityFactory
*/

#include <gtest/gtest.h>

#include "../../src/games/rtype/client/GraphicsConstants.hpp"
#include "../../src/games/rtype/shared/Config/GameConfig/RTypeGameConfig.hpp"

namespace rtype::games::rtype::client {

static std::pair<int, int> getPlayerSpriteOffset(uint32_t playerId) {
    const int SPRITE_HEIGHT = 17;
    uint32_t rowIndex = 0;
    if (playerId >= 1 && playerId <= ::rtype::game::config::MAX_PLAYER_COUNT) {
        rowIndex = playerId - 1;
    }
    int yOffset = static_cast<int>(rowIndex) * SPRITE_HEIGHT;

    return {0, yOffset};
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
    auto offset = rtype::games::rtype::client::getPlayerSpriteOffset(0);
    EXPECT_EQ(offset.first, 0);
    EXPECT_EQ(offset.second, 0);  // defaults to row 0
}

TEST(RtypeEntityFactoryTest, GetPlayerSpriteOffset_InvalidPlayerIdHigh) {
    auto offset = rtype::games::rtype::client::getPlayerSpriteOffset(5);
    EXPECT_EQ(offset.first, 0);
    EXPECT_EQ(offset.second, 0);  // defaults to row 0
}