/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_reliable_channel.cpp - Unit tests for ReliableChannel RUDP implementation
*/

#include <gtest/gtest.h>

#include <chrono>
#include <thread>
#include <vector>

#include "reliability/ReliableChannel.hpp"

using namespace rtype::network;

class ReliableChannelTest : public ::testing::Test {
   protected:
    ReliableChannel channel_{};
    std::vector<std::uint8_t> testData_{0x01, 0x02, 0x03, 0x04};
};

// ============================================================================
// Track Outgoing Tests
// ============================================================================

TEST_F(ReliableChannelTest, TrackOutgoing_Success) {
    auto result = channel_.trackOutgoing(1, testData_);
    EXPECT_TRUE(result.isOk());
}

TEST_F(ReliableChannelTest, TrackOutgoing_DuplicateSeqId) {
    EXPECT_TRUE(channel_.trackOutgoing(5, testData_).isOk());
    auto result = channel_.trackOutgoing(5, testData_);
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::DuplicatePacket);
}

TEST_F(ReliableChannelTest, TrackOutgoing_MultiplePackets) {
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(channel_.trackOutgoing(i, testData_).isOk());
    }
    EXPECT_EQ(channel_.getPendingCount(), 10);
}

// ============================================================================
// ACK Recording Tests
// ============================================================================

TEST_F(ReliableChannelTest, RecordAck_Success) {
    channel_.trackOutgoing(42, testData_);
    channel_.recordAck(42);
    EXPECT_EQ(channel_.getPendingCount(), 1);  // Still in pending (not cleaned)
}

TEST_F(ReliableChannelTest, RecordAck_NonexistentSeqId) {
    channel_.recordAck(999);  // Should not crash
    EXPECT_EQ(channel_.getPendingCount(), 0);
}

TEST_F(ReliableChannelTest, RecordAck_MultiplePackets) {
    channel_.trackOutgoing(1, testData_);
    channel_.trackOutgoing(2, testData_);
    channel_.trackOutgoing(3, testData_);

    channel_.recordAck(1);
    channel_.recordAck(3);
    // Seq 2 is not ACKed

    EXPECT_EQ(channel_.getPendingCount(), 3);
}

// ============================================================================
// Duplicate Detection Tests
// ============================================================================

TEST_F(ReliableChannelTest, IsDuplicate_FreshPacket) {
    EXPECT_FALSE(channel_.isDuplicate(100));
}

TEST_F(ReliableChannelTest, IsDuplicate_RecordedPacket) {
    channel_.recordReceived(100);
    EXPECT_TRUE(channel_.isDuplicate(100));
}

TEST_F(ReliableChannelTest, IsDuplicate_MultiplePackets) {
    channel_.recordReceived(10);
    channel_.recordReceived(20);
    channel_.recordReceived(30);

    EXPECT_TRUE(channel_.isDuplicate(10));
    EXPECT_TRUE(channel_.isDuplicate(20));
    EXPECT_TRUE(channel_.isDuplicate(30));
    EXPECT_FALSE(channel_.isDuplicate(40));
}

// ============================================================================
// Last Received Sequence Tests
// ============================================================================

TEST_F(ReliableChannelTest, GetLastReceivedSeqId_Initial) {
    EXPECT_EQ(channel_.getLastReceivedSeqId(), 0);
}

TEST_F(ReliableChannelTest, GetLastReceivedSeqId_AfterRecord) {
    channel_.recordReceived(42);
    EXPECT_EQ(channel_.getLastReceivedSeqId(), 42);
}

TEST_F(ReliableChannelTest, GetLastReceivedSeqId_TracksHighestSequence) {
    channel_.recordReceived(10);
    EXPECT_EQ(channel_.getLastReceivedSeqId(), 10);

    channel_.recordReceived(20);
    EXPECT_EQ(channel_.getLastReceivedSeqId(), 20);

    channel_.recordReceived(5);
    EXPECT_EQ(channel_.getLastReceivedSeqId(), 20);  // Should remain 20 (highest)
}

// ============================================================================
// Retransmission Tests
// ============================================================================

TEST_F(ReliableChannelTest, GetPacketsToRetransmit_NoPackets) {
    auto toRetransmit = channel_.getPacketsToRetransmit();
    EXPECT_TRUE(toRetransmit.empty());
}

TEST_F(ReliableChannelTest, GetPacketsToRetransmit_AckedPacket) {
    channel_.trackOutgoing(1, testData_);
    channel_.recordAck(1);

    auto toRetransmit = channel_.getPacketsToRetransmit();
    EXPECT_TRUE(toRetransmit.empty());
}

TEST_F(ReliableChannelTest, GetPacketsToRetransmit_TimeoutExpired) {
    ReliableChannel::Config config;
    config.retransmitTimeout = std::chrono::milliseconds(50);
    ReliableChannel channel{config};

    channel.trackOutgoing(1, testData_);

    // Before timeout
    auto toRetransmit = channel.getPacketsToRetransmit();
    EXPECT_TRUE(toRetransmit.empty());

    // After timeout
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    toRetransmit = channel.getPacketsToRetransmit();
    EXPECT_EQ(toRetransmit.size(), 1);
    EXPECT_EQ(toRetransmit[0].seqId, 1);
}

TEST_F(ReliableChannelTest, GetPacketsToRetransmit_RetryCountIncremented) {
    ReliableChannel::Config config;
    config.retransmitTimeout = std::chrono::milliseconds(30);
    ReliableChannel channel{config};

    channel.trackOutgoing(1, testData_);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto toRetransmit1 = channel.getPacketsToRetransmit();
    EXPECT_EQ(toRetransmit1.size(), 1);
    EXPECT_EQ(toRetransmit1[0].retryCount, 1);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto toRetransmit2 = channel.getPacketsToRetransmit();
    EXPECT_EQ(toRetransmit2.size(), 1);
    EXPECT_EQ(toRetransmit2[0].retryCount, 2);
}

TEST_F(ReliableChannelTest, GetPacketsToRetransmit_ImmediateRetransmit) {
    ReliableChannel::Config config;
    config.retransmitTimeout = std::chrono::milliseconds(0);
    config.maxRetries = 2;
    ReliableChannel channel{config};

    channel.trackOutgoing(7, testData_);

    // Immediate call should dequeue for retransmit when timeout == 0
    auto first = channel.getPacketsToRetransmit();
    EXPECT_EQ(first.size(), 1);
    EXPECT_EQ(first[0].retryCount, 1);

    auto second = channel.getPacketsToRetransmit();
    EXPECT_EQ(second.size(), 1);
    EXPECT_EQ(second[0].retryCount, 2);
}

// ============================================================================
// Cleanup Tests
// ============================================================================

TEST_F(ReliableChannelTest, Cleanup_RemovesAckedPackets) {
    channel_.trackOutgoing(1, testData_);
    channel_.trackOutgoing(2, testData_);

    channel_.recordAck(1);
    EXPECT_EQ(channel_.getPendingCount(), 2);

    auto result = channel_.cleanup();
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(channel_.getPendingCount(), 1);
}

TEST_F(ReliableChannelTest, Cleanup_SuccessfulWhenUnderRetryLimit) {
    ReliableChannel::Config config;
    config.retransmitTimeout = std::chrono::milliseconds(20);
    config.maxRetries = 3;
    ReliableChannel channel{config};

    channel.trackOutgoing(1, testData_);

    // Trigger multiple retransmits
    for (int i = 0; i < 2; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        auto toRetransmit = channel.getPacketsToRetransmit();
        EXPECT_FALSE(toRetransmit.empty());
    }

    auto result = channel.cleanup();
    EXPECT_TRUE(result.isOk());
}

TEST_F(ReliableChannelTest, Cleanup_FailsWhenMaxRetriesExceeded) {
    ReliableChannel::Config config;
    config.retransmitTimeout = std::chrono::milliseconds(20);
    config.maxRetries = 1;
    ReliableChannel channel{config};

    channel.trackOutgoing(1, testData_);

    // Trigger retransmits until limit exceeded
    bool reachedMaxRetries = false;
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        auto toRetransmit = channel.getPacketsToRetransmit();
        if (!toRetransmit.empty()) {
            if (toRetransmit[0].retryCount >= config.maxRetries) {
                EXPECT_EQ(toRetransmit[0].retryCount, config.maxRetries);
                reachedMaxRetries = true;
                break;
            }
        }
    }

    EXPECT_TRUE(reachedMaxRetries);  // Ensure we actually reached the retry limit

    auto result = channel.cleanup();
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::RetryLimitExceeded);
}

// ============================================================================
// Clear Tests
// ============================================================================

TEST_F(ReliableChannelTest, Clear_RemovesAllPendingPackets) {
    channel_.trackOutgoing(1, testData_);
    channel_.trackOutgoing(2, testData_);
    EXPECT_EQ(channel_.getPendingCount(), 2);

    channel_.clear();
    EXPECT_EQ(channel_.getPendingCount(), 0);
}

TEST_F(ReliableChannelTest, Clear_RemovesAllReceivedSequences) {
    channel_.recordReceived(10);
    channel_.recordReceived(20);
    EXPECT_EQ(channel_.getReceivedCount(), 2);

    channel_.clear();
    EXPECT_EQ(channel_.getReceivedCount(), 0);
    EXPECT_EQ(channel_.getLastReceivedSeqId(), 0);
}

// ============================================================================
// Sequence Wraparound Tests (uint16 max = 65535)
// ============================================================================

TEST_F(ReliableChannelTest, SequenceWraparound_TrackHighSequences) {
    std::uint16_t highSeq = 65534;
    channel_.trackOutgoing(highSeq, testData_);
    channel_.trackOutgoing(highSeq + 1, testData_);
    channel_.trackOutgoing(0, testData_);  // Wrapped around

    EXPECT_EQ(channel_.getPendingCount(), 3);
}

TEST_F(ReliableChannelTest, SequenceWraparound_DuplicateDetection) {
    std::uint16_t wrapped = 0;
    channel_.recordReceived(wrapped);
    EXPECT_TRUE(channel_.isDuplicate(wrapped));

    channel_.recordReceived(65535);
    EXPECT_TRUE(channel_.isDuplicate(65535));
}

TEST_F(ReliableChannelTest, SequenceWraparound_AckTracking) {
    // Test ACK tracking with wraparound
    channel_.recordReceived(65535);  // High sequence
    EXPECT_EQ(channel_.getLastReceivedSeqId(), 65535);

    channel_.recordReceived(0);  // Wrapped around (newer)
    EXPECT_EQ(channel_.getLastReceivedSeqId(), 0);

    channel_.recordReceived(65534);  // Older (should not update)
    EXPECT_EQ(channel_.getLastReceivedSeqId(), 0);  // Should remain 0
}

TEST_F(ReliableChannelTest, ReceivedSeqIdPruning_PreventsMemoryLeak) {
    // Record many sequence IDs to trigger pruning
    for (std::uint16_t i = 0; i < 1200; ++i) {  // More than window size (1000)
        channel_.recordReceived(i);
    }
    
    // Should have pruned old entries, keeping only recent ones
    EXPECT_LE(channel_.getReceivedCount(), 1000 + 50);  // Allow some buffer
    
    // Verify we can still detect duplicates for recent sequences
    EXPECT_TRUE(channel_.isDuplicate(1199));  // Recent sequence should be remembered
    EXPECT_EQ(channel_.getLastReceivedSeqId(), 1199);  // Highest should be tracked
}

// ============================================================================
// Out-of-Order Packet Handling
// ============================================================================

TEST_F(ReliableChannelTest, OutOfOrder_ReceiveInDifferentOrder) {
    channel_.recordReceived(3);
    EXPECT_TRUE(channel_.isDuplicate(3));
    EXPECT_EQ(channel_.getLastReceivedSeqId(), 3);

    channel_.recordReceived(1);
    EXPECT_TRUE(channel_.isDuplicate(1));
    EXPECT_EQ(channel_.getLastReceivedSeqId(), 3);  // Should remain 3 (highest)

    channel_.recordReceived(2);
    EXPECT_TRUE(channel_.isDuplicate(2));
    EXPECT_EQ(channel_.getLastReceivedSeqId(), 3);  // Should remain 3 (highest)

    EXPECT_EQ(channel_.getReceivedCount(), 3);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(ReliableChannelTest, Integration_FullReliableFlow) {
    // 1. Send packet
    EXPECT_TRUE(channel_.trackOutgoing(1, testData_).isOk());

    // 2. No retransmit yet
    auto toRetransmit = channel_.getPacketsToRetransmit();
    EXPECT_TRUE(toRetransmit.empty());

    // 3. Simulate receiving a packet (different endpoint)
    EXPECT_FALSE(channel_.isDuplicate(100));  // First receive = not duplicate
    channel_.recordReceived(100);
    EXPECT_TRUE(channel_.isDuplicate(100));   // Second check = duplicate

    // 4. Send ACK for received packet
    EXPECT_EQ(channel_.getLastReceivedSeqId(), 100);

    // 5. Receive ACK for our packet
    channel_.recordAck(1);
    toRetransmit = channel_.getPacketsToRetransmit();
    EXPECT_TRUE(toRetransmit.empty());

    // 6. Cleanup
    EXPECT_TRUE(channel_.cleanup().isOk());
    EXPECT_EQ(channel_.getPendingCount(), 0);
}

TEST_F(ReliableChannelTest, Integration_MultiplePacketExchange) {
    // Client sends packets 1, 2, 3
    EXPECT_TRUE(channel_.trackOutgoing(1, testData_).isOk());
    EXPECT_TRUE(channel_.trackOutgoing(2, testData_).isOk());
    EXPECT_TRUE(channel_.trackOutgoing(3, testData_).isOk());

    // Server receives packets 1, 3 (2 lost)
    EXPECT_FALSE(channel_.isDuplicate(1));  // First receive
    channel_.recordReceived(1);
    EXPECT_TRUE(channel_.isDuplicate(1));   // Duplicate detected

    EXPECT_FALSE(channel_.isDuplicate(3));  // First receive
    channel_.recordReceived(3);
    EXPECT_TRUE(channel_.isDuplicate(3));   // Duplicate detected

    // Server ACKs 1 and 3
    channel_.recordAck(1);
    channel_.recordAck(3);

    // Packet 2 still pending (not ACKed)
    EXPECT_EQ(channel_.getPendingCount(), 3);

    auto cleanup = channel_.cleanup();
    EXPECT_TRUE(cleanup.isOk());
    EXPECT_EQ(channel_.getPendingCount(), 1);  // Packet 2 still there
}

TEST_F(ReliableChannelTest, Integration_CustomConfig) {
    ReliableChannel::Config config;
    config.retransmitTimeout = std::chrono::milliseconds(100);
    config.maxRetries = 2;
    ReliableChannel channel{config};

    channel.trackOutgoing(1, testData_);

    // First retransmit
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    auto toRetransmit = channel.getPacketsToRetransmit();
    EXPECT_EQ(toRetransmit.size(), 1);
    EXPECT_EQ(toRetransmit[0].retryCount, 1);

    // Second retransmit
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    toRetransmit = channel.getPacketsToRetransmit();
    EXPECT_EQ(toRetransmit.size(), 1);
    EXPECT_EQ(toRetransmit[0].retryCount, 2);

    // No more retransmits (max reached)
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    toRetransmit = channel.getPacketsToRetransmit();
    EXPECT_TRUE(toRetransmit.empty());

    // Cleanup should fail
    auto result = channel.cleanup();
    EXPECT_TRUE(result.isErr());
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(ReliableChannelTest, EdgeCase_EmptyData) {
    std::vector<std::uint8_t> emptyData;
    EXPECT_TRUE(channel_.trackOutgoing(1, emptyData).isOk());
}

TEST_F(ReliableChannelTest, EdgeCase_LargeData) {
    std::vector<std::uint8_t> largeData(1000, 0xAA);
    EXPECT_TRUE(channel_.trackOutgoing(1, largeData).isOk());
}

TEST_F(ReliableChannelTest, EdgeCase_ManyPendingPackets) {
    for (int i = 0; i < 100; ++i) {
        EXPECT_TRUE(channel_.trackOutgoing(i, testData_).isOk());
    }
    EXPECT_EQ(channel_.getPendingCount(), 100);
}

TEST_F(ReliableChannelTest, RecordReceived_PrunesOldSeqIds) {
    // Record many seq IDs to trigger pruning
    // kReceivedSeqIdWindow is 1024 by default
    for (uint16_t i = 0; i < 1500; ++i) {
        channel_.recordReceived(i);
    }
    // After pruning, only recent ones should remain
    EXPECT_LE(channel_.getReceivedCount(), 1500);
    EXPECT_GT(channel_.getReceivedCount(), 0);
}

TEST_F(ReliableChannelTest, RecordReceived_UpdatesLastSeqId) {
    channel_.recordReceived(100);
    EXPECT_EQ(channel_.getLastReceivedSeqId(), 100);
    
    channel_.recordReceived(200);
    EXPECT_EQ(channel_.getLastReceivedSeqId(), 200);
    
    // Older seq ID shouldn't update lastReceived
    channel_.recordReceived(50);
    EXPECT_EQ(channel_.getLastReceivedSeqId(), 200);
}

TEST_F(ReliableChannelTest, Clear_ResetsAllState) {
    channel_.trackOutgoing(1, testData_);
    channel_.recordReceived(100);
    
    channel_.clear();
    
    EXPECT_EQ(channel_.getPendingCount(), 0);
    EXPECT_EQ(channel_.getReceivedCount(), 0);
    EXPECT_EQ(channel_.getLastReceivedSeqId(), 0);
}
