#include <gtest/gtest.h>

#include "server/shared/BanManager.hpp"

using namespace rtype::server;

TEST(BanManagerTest, BanAndUnbanEndpoint) {
    BanManager bm;
    Endpoint ep{"127.0.0.1", 12345};

    EXPECT_FALSE(bm.isEndpointBanned(ep));

    bm.banEndpoint(ep, "player", "cheating");
    EXPECT_TRUE(bm.isEndpointBanned(ep));

    auto list = bm.getBannedList();
    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list[0].ip, "127.0.0.1");
    EXPECT_EQ(list[0].playerName, "player");

    bm.unbanEndpoint(ep);
    EXPECT_FALSE(bm.isEndpointBanned(ep));
    EXPECT_TRUE(bm.getBannedList().empty());
}

TEST(BanManagerTest, BanAndUnbanIp) {
    BanManager bm;
    std::string ip = "10.0.0.1";

    EXPECT_FALSE(bm.isIpBanned(ip));

    bm.banIp(ip, "admin", "abuse");
    EXPECT_TRUE(bm.isIpBanned(ip));

    auto list = bm.getBannedList();
    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list[0].ip, ip);

    bm.unbanIp(ip);
    EXPECT_FALSE(bm.isIpBanned(ip));
    EXPECT_TRUE(bm.getBannedList().empty());
}

TEST(BanManagerTest, ClearAllBans) {
    BanManager bm;
    bm.banIp("1.2.3.4", "a", "r");
    bm.banEndpoint({"5.6.7.8", 1111}, "b", "r");

    EXPECT_FALSE(bm.getBannedList().empty());
    bm.clearAllBans();
    EXPECT_TRUE(bm.getBannedList().empty());
}
