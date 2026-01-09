#include <gtest/gtest.h>

#include "../../../lib/common/src/Logger/LogCategory.hpp"

using namespace rtype;

TEST(LogCategoryTest, BitwiseOperatorsAndEnableCheck) {
    LogCategory mask = LogCategory::None;
    mask |= LogCategory::Main;
    mask |= LogCategory::Network;

    EXPECT_TRUE(isCategoryEnabled(mask, LogCategory::Main));
    EXPECT_TRUE(isCategoryEnabled(mask, LogCategory::Network));
    EXPECT_FALSE(isCategoryEnabled(mask, LogCategory::Audio));

    auto combined = LogCategory::Main | LogCategory::Network;
    EXPECT_TRUE(isCategoryEnabled(combined, LogCategory::Main));
}

TEST(LogCategoryTest, ToStringAndParse) {
    EXPECT_EQ(toString(LogCategory::None), "None");
    EXPECT_EQ(toString(LogCategory::All), "All");
    EXPECT_EQ(toString(LogCategory::AI), "AI");

    EXPECT_EQ(categoryFromString("All"), LogCategory::All);
    EXPECT_EQ(categoryFromString("main"), LogCategory::Main);
    EXPECT_EQ(categoryFromString("NETWORK"), LogCategory::Network);
    EXPECT_EQ(categoryFromString("GFX"), LogCategory::Graphics);
    EXPECT_EQ(categoryFromString("unknown"), LogCategory::None);
}
