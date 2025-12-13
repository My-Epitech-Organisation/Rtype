/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for RtypeEntityFactory
*/

#include <gtest/gtest.h>
#include <ECS.hpp>
#include "../src/games/rtype/client/GameScene/RtypeEntityFactory.cpp"  // Include the implementation to test static functions
#include "../src/games/rtype/client/GameScene/RtypeEntityFactory.hpp"
#include "games/rtype/shared/Components/PlayerIdComponent.hpp"
#include "games/rtype/client/Components/ImageComponent.hpp"
#include "games/rtype/client/Components/TextureRectComponent.hpp"
#include "games/rtype/client/Components/SizeComponent.hpp"
#include "games/rtype/client/Components/BoxingComponent.hpp"
#include "games/rtype/client/Components/ZIndexComponent.hpp"
#include "games/rtype/shared/Components/Tags.hpp"
#include "games/rtype/shared/Components/PositionComponent.hpp"
#include "games/rtype/shared/Components/HealthComponent.hpp"
#include "client/Graphic/AssetManager/AssetManager.hpp"

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