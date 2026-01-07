/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Extra branch coverage tests for ServerApp
*/

#include <gtest/gtest.h>
#include <atomic>
#include <memory>

#include "server/serverApp/ServerApp.hpp"

using namespace rtype::server;

class ServerAppExtraBranchTest : public ::testing::Test {
protected:
    void SetUp() override {
        shutdownFlag_ = std::make_shared<std::atomic<bool>>(false);
    }

    void TearDown() override {
        shutdownFlag_->store(true);
    }

    std::shared_ptr<std::atomic<bool>> shutdownFlag_;
};

TEST_F(ServerAppExtraBranchTest, ConstructorThenImmediateCheck) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);
    
    EXPECT_TRUE(server.isRunning());
    EXPECT_EQ(server.getConnectedClientCount(), 0u);
    EXPECT_TRUE(server.getConnectedClientIds().empty());
}

TEST_F(ServerAppExtraBranchTest, ConstructorWithVariousTickRates) {
    std::vector<uint32_t> rates = {1, 15, 30, 60, 90, 120, 144};
    
    for (auto rate : rates) {
        auto flag = std::make_shared<std::atomic<bool>>(false);
        ServerApp server(8080, 4, rate, flag, 30, false);
        EXPECT_TRUE(server.isRunning());
    }
}

TEST_F(ServerAppExtraBranchTest, MultipleGettersInSequence) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);
    
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(server.isRunning());
        EXPECT_EQ(server.getConnectedClientCount(), 0u);
        auto ids = server.getConnectedClientIds();
        EXPECT_TRUE(ids.empty());
        auto info = server.getClientInfo(i);
        EXPECT_FALSE(info.has_value());
    }
}

TEST_F(ServerAppExtraBranchTest, StopThenCheckMultiple) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);
    
    EXPECT_TRUE(server.isRunning());
    server.stop();
    
    for (int i = 0; i < 5; ++i) {
        EXPECT_FALSE(server.isRunning());
    }
}

TEST_F(ServerAppExtraBranchTest, VariousPortNumbers) {
    std::vector<uint16_t> ports = {1025, 2000, 5000, 8000, 9000, 12000};
    
    for (auto port : ports) {
        auto flag = std::make_shared<std::atomic<bool>>(false);
        ServerApp server(port, 4, 60, flag, 30, false);
        EXPECT_TRUE(server.isRunning());
    }
}

TEST_F(ServerAppExtraBranchTest, VerboseModeVariations) {
    auto flag1 = std::make_shared<std::atomic<bool>>(false);
    ServerApp server1(8080, 4, 60, flag1, 30, true);
    EXPECT_TRUE(server1.isRunning());
    
    auto flag2 = std::make_shared<std::atomic<bool>>(false);
    ServerApp server2(8081, 4, 60, flag2, 30, false);
    EXPECT_TRUE(server2.isRunning());
}

TEST_F(ServerAppExtraBranchTest, DifferentPlayerCounts) {
    std::vector<size_t> counts = {1, 2, 3, 4, 6, 8, 10, 16};
    
    for (auto count : counts) {
        auto flag = std::make_shared<std::atomic<bool>>(false);
        ServerApp server(8080, count, 60, flag, 30, false);
        EXPECT_TRUE(server.isRunning());
    }
}

TEST_F(ServerAppExtraBranchTest, TimeoutVariations) {
    std::vector<uint32_t> timeouts = {1, 5, 15, 30, 45, 60, 120};
    
    for (auto timeout : timeouts) {
        auto flag = std::make_shared<std::atomic<bool>>(false);
        ServerApp server(8080, 4, 60, flag, timeout, false);
        EXPECT_TRUE(server.isRunning());
    }
}

TEST_F(ServerAppExtraBranchTest, GetClientInfoRangeOfIds) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);
    
    for (uint32_t id = 0; id < 20; ++id) {
        auto info = server.getClientInfo(id);
        EXPECT_FALSE(info.has_value());
    }
}

TEST_F(ServerAppExtraBranchTest, ConstructorAndCheckVerbose) {
    auto flag = std::make_shared<std::atomic<bool>>(false);
    ServerApp server(8080, 4, 60, flag, 30, true);
    
    EXPECT_TRUE(server.isRunning());
    EXPECT_EQ(server.getConnectedClientCount(), 0u);
}

TEST_F(ServerAppExtraBranchTest, MultipleConstructorsWithDifferentParams) {
    auto flag1 = std::make_shared<std::atomic<bool>>(false);
    ServerApp s1(8080, 2, 30, flag1, 15, false);
    
    auto flag2 = std::make_shared<std::atomic<bool>>(false);
    ServerApp s2(9000, 8, 120, flag2, 60, true);
    
    auto flag3 = std::make_shared<std::atomic<bool>>(false);
    ServerApp s3(5000, 4, 60, flag3, 30, false);
    
    EXPECT_TRUE(s1.isRunning());
    EXPECT_TRUE(s2.isRunning());
    EXPECT_TRUE(s3.isRunning());
}
