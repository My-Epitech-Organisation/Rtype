#include <gtest/gtest.h>

#include "../../../lib/common/src/Logger/Timestamp.hpp"
#include <string>

using namespace rtype;

TEST(TimestampTest, NowFormatLooksReasonable) {
    std::string s = Timestamp::now();
    // Expect something like YYYY-MM-DD HH:MM:SS.mmm
    EXPECT_NE(s.find('-'), std::string::npos);
    EXPECT_NE(s.find(':'), std::string::npos);
    EXPECT_NE(s.find('.'), std::string::npos);
    EXPECT_GT(s.size(), 18u);
}
