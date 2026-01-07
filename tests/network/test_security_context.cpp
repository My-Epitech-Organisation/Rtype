/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** SecurityContext unit tests
*/

#include <gtest/gtest.h>

#include <thread>
#include <chrono>

#include "protocol/SecurityContext.hpp"

using namespace rtype::network;

class SecurityContextTest : public ::testing::Test {
   protected:
    SecurityContext context_;
    const std::string testKey_ = "192.168.1.100:4242";
};

// =============================================================================
// Basic Connection Tests
// =============================================================================

TEST_F(SecurityContextTest, InitialConnectionCount) {
    EXPECT_EQ(context_.getConnectionCount(), 0);
}

TEST_F(SecurityContextTest, RegisterConnection) {
    context_.registerConnection(testKey_, 12345);
    EXPECT_EQ(context_.getConnectionCount(), 1);
}

TEST_F(SecurityContextTest, RegisterMultipleConnections) {
    context_.registerConnection("client1", 1);
    context_.registerConnection("client2", 2);
    context_.registerConnection("client3", 3);
    EXPECT_EQ(context_.getConnectionCount(), 3);
}

TEST_F(SecurityContextTest, RemoveConnection) {
    context_.registerConnection(testKey_, 123);
    EXPECT_EQ(context_.getConnectionCount(), 1);

    context_.removeConnection(testKey_);
    EXPECT_EQ(context_.getConnectionCount(), 0);
}

TEST_F(SecurityContextTest, RemoveNonexistentConnection) {
    context_.removeConnection("nonexistent");
    EXPECT_EQ(context_.getConnectionCount(), 0);
}

TEST_F(SecurityContextTest, ClearAllConnections) {
    context_.registerConnection("c1", 1);
    context_.registerConnection("c2", 2);
    context_.registerConnection("c3", 3);

    context_.clear();
    EXPECT_EQ(context_.getConnectionCount(), 0);
}

TEST_F(SecurityContextTest, GetConnectionInfo) {
    context_.registerConnection(testKey_, 42);

    const auto& info = context_.getConnectionInfo(testKey_);
    EXPECT_EQ(info.userId, 42);
}

TEST_F(SecurityContextTest, GetConnectionInfoThrowsIfNotFound) {
    EXPECT_THROW(context_.getConnectionInfo("nonexistent"), std::out_of_range);
}

// =============================================================================
// Sequence ID Validation Tests
// =============================================================================

TEST_F(SecurityContextTest, ValidateFirstSequenceId) {
    auto result = context_.validateSequenceId(testKey_, 100);
    EXPECT_TRUE(result.isOk());
}

TEST_F(SecurityContextTest, ValidateSequentialSequenceIds) {
    EXPECT_TRUE(context_.validateSequenceId(testKey_, 1).isOk());
    EXPECT_TRUE(context_.validateSequenceId(testKey_, 2).isOk());
    EXPECT_TRUE(context_.validateSequenceId(testKey_, 3).isOk());
}

TEST_F(SecurityContextTest, ValidateDuplicateSequenceId) {
    EXPECT_TRUE(context_.validateSequenceId(testKey_, 100).isOk());
    auto result = context_.validateSequenceId(testKey_, 100);
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::DuplicatePacket);
}

TEST_F(SecurityContextTest, ValidateOutOfOrderSequenceIds) {
    EXPECT_TRUE(context_.validateSequenceId(testKey_, 5).isOk());
    EXPECT_TRUE(context_.validateSequenceId(testKey_, 3).isOk());
    EXPECT_TRUE(context_.validateSequenceId(testKey_, 7).isOk());
    EXPECT_TRUE(context_.validateSequenceId(testKey_, 4).isOk());
}

TEST_F(SecurityContextTest, ValidateSequenceIdWraparound) {
    EXPECT_TRUE(context_.validateSequenceId(testKey_, 65534).isOk());
    EXPECT_TRUE(context_.validateSequenceId(testKey_, 65535).isOk());
    EXPECT_TRUE(context_.validateSequenceId(testKey_, 0).isOk());
    EXPECT_TRUE(context_.validateSequenceId(testKey_, 1).isOk());
}

TEST_F(SecurityContextTest, ValidateStaleSequenceId) {
    EXPECT_TRUE(context_.validateSequenceId(testKey_, 5000).isOk());

    // Very old sequence (should be rejected)
    auto result = context_.validateSequenceId(testKey_, 1);
    // Depending on window size this may or may not be rejected
    // For window size 1000, 5000 - 1 = 4999 > 1000, so it should fail
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::InvalidSequence);
}

TEST_F(SecurityContextTest, ValidateSequenceIdWindowEdge) {
    EXPECT_TRUE(context_.validateSequenceId(testKey_, 1000).isOk());
    // Just within window
    EXPECT_TRUE(context_.validateSequenceId(testKey_, 1).isOk());
}

TEST_F(SecurityContextTest, ValidateSequenceIdHighDistance) {
    EXPECT_TRUE(context_.validateSequenceId(testKey_, 100).isOk());
    // Moderate jump forward (within wraparound threshold)
    EXPECT_TRUE(context_.validateSequenceId(testKey_, 1000).isOk());
}

TEST_F(SecurityContextTest, ValidateSequenceIdNegativeWraparound) {
    // Start near max
    EXPECT_TRUE(context_.validateSequenceId(testKey_, 65000).isOk());
    // Jump back (wrapping around)
    EXPECT_TRUE(context_.validateSequenceId(testKey_, 100).isOk());
}

// =============================================================================
// User ID Validation Tests
// =============================================================================

TEST_F(SecurityContextTest, ValidateUnassignedUserIdNewConnection) {
    auto result = context_.validateUserIdMapping(testKey_, kUnassignedUserId);
    EXPECT_TRUE(result.isOk());
}

TEST_F(SecurityContextTest, ValidateRegisteredUserId) {
    context_.registerConnection(testKey_, 12345);
    auto result = context_.validateUserIdMapping(testKey_, 12345);
    EXPECT_TRUE(result.isOk());
}

TEST_F(SecurityContextTest, ValidateMismatchedUserId) {
    context_.registerConnection(testKey_, 12345);
    auto result = context_.validateUserIdMapping(testKey_, 99999);
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::InvalidUserId);
}

TEST_F(SecurityContextTest, ValidateUnknownConnectionWithUserId) {
    auto result = context_.validateUserIdMapping("unknown", 12345);
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::InvalidUserId);
}

TEST_F(SecurityContextTest, ValidateUnassignedWithUnassignedRegistered) {
    context_.registerConnection(testKey_, kUnassignedUserId);
    auto result = context_.validateUserIdMapping(testKey_, kUnassignedUserId);
    EXPECT_TRUE(result.isOk());
}

// =============================================================================
// Cleanup Tests
// =============================================================================

TEST_F(SecurityContextTest, CleanupNoStaleConnections) {
    context_.registerConnection("c1", 1);
    context_.registerConnection("c2", 2);

    auto removed = context_.cleanupStaleConnections(3600);
    EXPECT_EQ(removed, 0);
    EXPECT_EQ(context_.getConnectionCount(), 2);
}

TEST_F(SecurityContextTest, CleanupStaleConnections) {
    context_.registerConnection("old", 1);

    // Sleep just a tiny bit then use very short timeout
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto removed = context_.cleanupStaleConnections(0);
    EXPECT_EQ(removed, 1);
    EXPECT_EQ(context_.getConnectionCount(), 0);
}

TEST_F(SecurityContextTest, CleanupMixedConnections) {
    context_.registerConnection("active1", 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    context_.registerConnection("active2", 2);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Should not remove anything with long timeout
    auto removed = context_.cleanupStaleConnections(3600);
    EXPECT_EQ(removed, 0);
}

// =============================================================================
// ConnectionInfo Tests
// =============================================================================

TEST_F(SecurityContextTest, ConnectionInfoInitialValues) {
    SecurityContext::ConnectionInfo info;
    EXPECT_EQ(info.userId, kUnassignedUserId);
    EXPECT_EQ(info.lastValidSeqId, 0);
    EXPECT_FALSE(info.initialized);
    EXPECT_TRUE(info.receivedSeqs.empty());
}

// =============================================================================
// Integration Tests
// =============================================================================

TEST_F(SecurityContextTest, FullConnectionLifecycle) {
    // New connection
    EXPECT_TRUE(context_.validateUserIdMapping(testKey_, kUnassignedUserId).isOk());

    // First packet
    EXPECT_TRUE(context_.validateSequenceId(testKey_, 1).isOk());

    // Register after accept
    context_.registerConnection(testKey_, 42);

    // Validate registered user
    EXPECT_TRUE(context_.validateUserIdMapping(testKey_, 42).isOk());

    // More packets
    EXPECT_TRUE(context_.validateSequenceId(testKey_, 2).isOk());
    EXPECT_TRUE(context_.validateSequenceId(testKey_, 3).isOk());

    // Duplicate rejected
    EXPECT_TRUE(context_.validateSequenceId(testKey_, 2).isErr());

    // Disconnect
    context_.removeConnection(testKey_);
    EXPECT_EQ(context_.getConnectionCount(), 0);
}

TEST_F(SecurityContextTest, MultipleConnectionsIsolated) {
    // Two separate connections
    context_.registerConnection("client1", 100);
    context_.registerConnection("client2", 200);

    // Sequence IDs are independent
    EXPECT_TRUE(context_.validateSequenceId("client1", 1).isOk());
    EXPECT_TRUE(context_.validateSequenceId("client2", 1).isOk());

    // User ID validation is per-connection
    EXPECT_TRUE(context_.validateUserIdMapping("client1", 100).isOk());
    EXPECT_TRUE(context_.validateUserIdMapping("client2", 200).isOk());

    // Wrong user ID fails
    EXPECT_TRUE(context_.validateUserIdMapping("client1", 200).isErr());
}

// =============================================================================
// Window Size Tests
// =============================================================================

TEST_F(SecurityContextTest, ManySequenceIdsPruned) {
    // Fill up more than window size
    for (uint16_t i = 0; i < 1200; ++i) {
        EXPECT_TRUE(context_.validateSequenceId(testKey_, i).isOk());
    }

    // Old sequences should be pruned
    const auto& info = context_.getConnectionInfo(testKey_);
    EXPECT_LE(info.receivedSeqs.size(), kAntiReplayWindowSize + 50);
}

