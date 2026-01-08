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

TEST(BanManagerTest, IpBanBlocksAllEndpointsFromThatIp) {
    BanManager bm;
    std::string ip = "192.168.1.100";

    // Ban the IP
    bm.banIp(ip, "player1", "abuse");

    // Any endpoint from that IP should be considered banned
    Endpoint ep1{ip, 12345};
    Endpoint ep2{ip, 54321};
    Endpoint ep3{ip, 9999};

    EXPECT_TRUE(bm.isEndpointBanned(ep1));
    EXPECT_TRUE(bm.isEndpointBanned(ep2));
    EXPECT_TRUE(bm.isEndpointBanned(ep3));

    // But a different IP should not be banned
    Endpoint differentIp{"192.168.1.101", 12345};
    EXPECT_FALSE(bm.isEndpointBanned(differentIp));
}

TEST(BanManagerTest, EndpointBanDoesNotBlockOtherPortsFromSameIp) {
    BanManager bm;
    Endpoint ep{"10.0.0.50", 8080};

    bm.banEndpoint(ep, "player", "reason");

    // Same IP, same port - banned
    EXPECT_TRUE(bm.isEndpointBanned(ep));

    // Same IP, different port - NOT banned (endpoint ban is specific)
    Endpoint differentPort{"10.0.0.50", 9090};
    EXPECT_FALSE(bm.isEndpointBanned(differentPort));
}

TEST(BanManagerTest, MultipleBansAndUnbans) {
    BanManager bm;

    // Ban multiple IPs
    bm.banIp("1.1.1.1", "p1", "r1");
    bm.banIp("2.2.2.2", "p2", "r2");
    bm.banIp("3.3.3.3", "p3", "r3");

    // Ban multiple endpoints
    bm.banEndpoint({"4.4.4.4", 1000}, "p4", "r4");
    bm.banEndpoint({"5.5.5.5", 2000}, "p5", "r5");

    EXPECT_EQ(bm.getBannedList().size(), 5);

    // Unban one IP
    bm.unbanIp("2.2.2.2");
    EXPECT_EQ(bm.getBannedList().size(), 4);
    EXPECT_FALSE(bm.isIpBanned("2.2.2.2"));
    EXPECT_TRUE(bm.isIpBanned("1.1.1.1"));
    EXPECT_TRUE(bm.isIpBanned("3.3.3.3"));

    // Unban one endpoint
    bm.unbanEndpoint({"4.4.4.4", 1000});
    EXPECT_EQ(bm.getBannedList().size(), 3);
}

TEST(BanManagerTest, BanDetailsPreserved) {
    BanManager bm;
    std::string ip = "172.16.0.1";
    std::string playerName = "TestPlayer";
    std::string reason = "Testing ban details";

    bm.banIp(ip, playerName, reason);

    auto list = bm.getBannedList();
    ASSERT_EQ(list.size(), 1);
    EXPECT_EQ(list[0].ip, ip);
    EXPECT_EQ(list[0].playerName, playerName);
    EXPECT_EQ(list[0].reason, reason);
    EXPECT_EQ(list[0].port, 0);  // IP bans have port 0
}

TEST(BanManagerTest, EndpointBanDetailsPreserved) {
    BanManager bm;
    Endpoint ep{"192.168.0.1", 7777};
    std::string playerName = "EndpointPlayer";
    std::string reason = "Endpoint ban reason";

    bm.banEndpoint(ep, playerName, reason);

    auto list = bm.getBannedList();
    ASSERT_EQ(list.size(), 1);
    EXPECT_EQ(list[0].ip, ep.address);
    EXPECT_EQ(list[0].port, ep.port);
    EXPECT_EQ(list[0].playerName, playerName);
    EXPECT_EQ(list[0].reason, reason);
}

TEST(BanManagerTest, UnbanNonExistentDoesNotCrash) {
    BanManager bm;

    // Should not crash when unbanning something that doesn't exist
    bm.unbanIp("99.99.99.99");
    bm.unbanEndpoint({"88.88.88.88", 1234});

    EXPECT_TRUE(bm.getBannedList().empty());
}

TEST(BanManagerTest, DoubleBanSameIp) {
    BanManager bm;
    std::string ip = "10.10.10.10";

    bm.banIp(ip, "player1", "reason1");
    bm.banIp(ip, "player2", "reason2");  // Same IP, different details

    // Should only have one entry (set semantics)
    EXPECT_TRUE(bm.isIpBanned(ip));

    // Check details are updated
    auto list = bm.getBannedList();
    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list[0].playerName, "player2");  // Last ban wins
}

TEST(BanManagerTest, ClearAllBansMultipleTimes) {
    BanManager bm;

    bm.banIp("1.1.1.1", "p", "r");
    bm.clearAllBans();
    EXPECT_TRUE(bm.getBannedList().empty());

    // Clear again when already empty
    bm.clearAllBans();
    EXPECT_TRUE(bm.getBannedList().empty());

    // Add more and clear again
    bm.banEndpoint({"2.2.2.2", 100}, "p", "r");
    bm.banEndpoint({"3.3.3.3", 200}, "p", "r");
    bm.clearAllBans();
    EXPECT_TRUE(bm.getBannedList().empty());
}
