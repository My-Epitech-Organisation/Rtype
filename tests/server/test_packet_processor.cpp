/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_packet_processor - Unit tests for PacketProcessor
*/

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <vector>

#include "../../src/server/serverApp/packetProcessor/PacketProcessor.hpp"
#include "../../src/server/shared/ServerMetrics.hpp"
#include "../../lib/rtype_network/src/Serializer.hpp"
#include "../../lib/rtype_network/src/protocol/Header.hpp"
#include "../../lib/rtype_network/src/protocol/OpCode.hpp"

using namespace rtype::server;
using namespace rtype::network;

// ============================================================================
// TEST FIXTURE
// ============================================================================

class PacketProcessorTest : public ::testing::Test {
   protected:
    void SetUp() override {
        metrics_ = std::make_shared<ServerMetrics>();
    }

    void TearDown() override {
        metrics_.reset();
    }

    std::shared_ptr<ServerMetrics> metrics_;

    // Helper to create a valid RTGP packet
    std::vector<uint8_t> createValidPacket(OpCode opcode, uint32_t userId,
                                           uint32_t seqId,
                                           const std::vector<uint8_t>& payload = {}) {
        std::vector<uint8_t> packet;

        // RTGP Magic bytes
        packet.push_back('R');
        packet.push_back('T');
        packet.push_back('G');
        packet.push_back('P');

        // Version (1)
        packet.push_back(1);

        // OpCode
        packet.push_back(static_cast<uint8_t>(opcode));

        // Sequence ID (big-endian)
        packet.push_back((seqId >> 24) & 0xFF);
        packet.push_back((seqId >> 16) & 0xFF);
        packet.push_back((seqId >> 8) & 0xFF);
        packet.push_back(seqId & 0xFF);

        // User ID (big-endian)
        packet.push_back((userId >> 24) & 0xFF);
        packet.push_back((userId >> 16) & 0xFF);
        packet.push_back((userId >> 8) & 0xFF);
        packet.push_back(userId & 0xFF);

        // Payload size (big-endian)
        uint16_t payloadSize = static_cast<uint16_t>(payload.size());
        packet.push_back((payloadSize >> 8) & 0xFF);
        packet.push_back(payloadSize & 0xFF);

        // Payload
        packet.insert(packet.end(), payload.begin(), payload.end());

        return packet;
    }
};

// ============================================================================
// CONSTRUCTOR TESTS
// ============================================================================

TEST_F(PacketProcessorTest, Constructor_ValidParameters) {
    EXPECT_NO_THROW({
        PacketProcessor processor(metrics_, false);
    });
}

TEST_F(PacketProcessorTest, Constructor_VerboseMode) {
    EXPECT_NO_THROW({
        PacketProcessor processor(metrics_, true);
    });
}

// ============================================================================
// PROCESS RAW DATA TESTS - VALIDATION BRANCH COVERAGE
// ============================================================================

TEST_F(PacketProcessorTest, ProcessRawData_EmptyData) {
    PacketProcessor processor(metrics_, false);

    std::vector<uint8_t> emptyData;
    auto result = processor.processRawData("endpoint1",
        std::span<const uint8_t>(emptyData.data(), emptyData.size()));

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(metrics_->packetsDropped.load(), 1u);
}

TEST_F(PacketProcessorTest, ProcessRawData_InvalidMagic) {
    PacketProcessor processor(metrics_, false);

    std::vector<uint8_t> invalidData = {'X', 'Y', 'Z', 'W', 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    auto result = processor.processRawData("endpoint1",
        std::span<const uint8_t>(invalidData.data(), invalidData.size()));

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(metrics_->packetsDropped.load(), 1u);
}

TEST_F(PacketProcessorTest, ProcessRawData_TooShort) {
    PacketProcessor processor(metrics_, false);

    std::vector<uint8_t> shortData = {'R', 'T', 'G', 'P'};
    auto result = processor.processRawData("endpoint1",
        std::span<const uint8_t>(shortData.data(), shortData.size()));

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(metrics_->packetsDropped.load(), 1u);
}

TEST_F(PacketProcessorTest, ProcessRawData_ValidPacket) {
    PacketProcessor processor(metrics_, true);

    // Register the connection first
    processor.registerConnection("endpoint1", 1);

    auto packet = createValidPacket(OpCode::C_INPUT, 1, 1);
    auto result = processor.processRawData("endpoint1",
        std::span<const uint8_t>(packet.data(), packet.size()));

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(metrics_->packetsDropped.load(), 0u);
}

TEST_F(PacketProcessorTest, ProcessRawData_SequenceIdReplay) {
    PacketProcessor processor(metrics_, false);

    processor.registerConnection("endpoint1", 1);

    // Send first packet
    auto packet1 = createValidPacket(OpCode::C_INPUT, 1, 1);
    auto result1 = processor.processRawData("endpoint1",
        std::span<const uint8_t>(packet1.data(), packet1.size()));
    EXPECT_TRUE(result1.has_value());

    // Send same sequence again (should be rejected)
    auto result2 = processor.processRawData("endpoint1",
        std::span<const uint8_t>(packet1.data(), packet1.size()));
    EXPECT_FALSE(result2.has_value());
    EXPECT_GE(metrics_->packetsDropped.load(), 1u);
}

TEST_F(PacketProcessorTest, ProcessRawData_UserIdSpoofing) {
    PacketProcessor processor(metrics_, false);

    // Register endpoint with userId 1
    processor.registerConnection("endpoint1", 1);

    // Send packet claiming to be userId 2
    auto packet = createValidPacket(OpCode::C_INPUT, 2, 1);
    auto result = processor.processRawData("endpoint1",
        std::span<const uint8_t>(packet.data(), packet.size()));

    EXPECT_FALSE(result.has_value());
    EXPECT_GE(metrics_->packetsDropped.load(), 1u);
}

TEST_F(PacketProcessorTest, ProcessRawData_UnregisteredEndpoint) {
    PacketProcessor processor(metrics_, false);

    // Don't register the endpoint
    auto packet = createValidPacket(OpCode::C_INPUT, 1, 1);
    auto result = processor.processRawData("unknown_endpoint",
        std::span<const uint8_t>(packet.data(), packet.size()));

    // Should fail due to unregistered endpoint
    EXPECT_FALSE(result.has_value());
}

TEST_F(PacketProcessorTest, ProcessRawData_WithPayload) {
    PacketProcessor processor(metrics_, true);

    processor.registerConnection("endpoint1", 1);

    std::vector<uint8_t> payload = {0x01, 0x02, 0x03, 0x04};
    auto packet = createValidPacket(OpCode::C_INPUT, 1, 1, payload);
    auto result = processor.processRawData("endpoint1",
        std::span<const uint8_t>(packet.data(), packet.size()));

    EXPECT_TRUE(result.has_value());
    if (result.has_value()) {
        EXPECT_EQ(result->data().size(), payload.size());
    }
}

TEST_F(PacketProcessorTest, ProcessRawData_MultipleValidPackets) {
    PacketProcessor processor(metrics_, false);

    processor.registerConnection("endpoint1", 1);

    for (uint32_t i = 1; i <= 5; ++i) {
        auto packet = createValidPacket(OpCode::C_INPUT, 1, i);
        auto result = processor.processRawData("endpoint1",
            std::span<const uint8_t>(packet.data(), packet.size()));
        EXPECT_TRUE(result.has_value());
    }

    EXPECT_EQ(metrics_->packetsDropped.load(), 0u);
}

// ============================================================================
// REGISTER/UNREGISTER CONNECTION TESTS
// ============================================================================

TEST_F(PacketProcessorTest, RegisterConnection_ValidEndpoint) {
    PacketProcessor processor(metrics_, false);

    EXPECT_NO_THROW({
        processor.registerConnection("endpoint1", 42);
    });

    // Verify by sending a packet
    auto packet = createValidPacket(OpCode::C_INPUT, 42, 1);
    auto result = processor.processRawData("endpoint1",
        std::span<const uint8_t>(packet.data(), packet.size()));
    EXPECT_TRUE(result.has_value());
}

TEST_F(PacketProcessorTest, RegisterConnection_MultipleEndpoints) {
    PacketProcessor processor(metrics_, false);

    processor.registerConnection("endpoint1", 1);
    processor.registerConnection("endpoint2", 2);
    processor.registerConnection("endpoint3", 3);

    auto packet1 = createValidPacket(OpCode::C_INPUT, 1, 1);
    auto packet2 = createValidPacket(OpCode::C_INPUT, 2, 1);
    auto packet3 = createValidPacket(OpCode::C_INPUT, 3, 1);

    EXPECT_TRUE(processor.processRawData("endpoint1",
        std::span<const uint8_t>(packet1.data(), packet1.size())).has_value());
    EXPECT_TRUE(processor.processRawData("endpoint2",
        std::span<const uint8_t>(packet2.data(), packet2.size())).has_value());
    EXPECT_TRUE(processor.processRawData("endpoint3",
        std::span<const uint8_t>(packet3.data(), packet3.size())).has_value());
}

TEST_F(PacketProcessorTest, UnregisterConnection_ValidEndpoint) {
    PacketProcessor processor(metrics_, false);

    processor.registerConnection("endpoint1", 1);

    EXPECT_NO_THROW({
        processor.unregisterConnection("endpoint1");
    });

    // After unregister, packets should fail validation
    auto packet = createValidPacket(OpCode::C_INPUT, 1, 1);
    auto result = processor.processRawData("endpoint1",
        std::span<const uint8_t>(packet.data(), packet.size()));
    EXPECT_FALSE(result.has_value());
}

TEST_F(PacketProcessorTest, UnregisterConnection_UnknownEndpoint) {
    PacketProcessor processor(metrics_, false);

    // Should not crash
    EXPECT_NO_THROW({
        processor.unregisterConnection("unknown_endpoint");
    });
}

// ============================================================================
// GET SECURITY CONTEXT TESTS
// ============================================================================

TEST_F(PacketProcessorTest, GetSecurityContext_ReturnsReference) {
    PacketProcessor processor(metrics_, false);

    auto& context = processor.getSecurityContext();
    (void)context;  // Just verify we can get a reference
}

// ============================================================================
// EXCEPTION HANDLING TESTS
// ============================================================================

TEST_F(PacketProcessorTest, ProcessRawData_InvalidPayloadSize) {
    PacketProcessor processor(metrics_, false);

    processor.registerConnection("endpoint1", 1);

    // Create packet with claimed payload size larger than actual
    std::vector<uint8_t> packet;
    packet.push_back('R');
    packet.push_back('T');
    packet.push_back('G');
    packet.push_back('P');
    packet.push_back(1);  // Version
    packet.push_back(static_cast<uint8_t>(OpCode::C_INPUT));
    packet.push_back(0); packet.push_back(0); packet.push_back(0); packet.push_back(1);  // SeqID
    packet.push_back(0); packet.push_back(0); packet.push_back(0); packet.push_back(1);  // UserID
    packet.push_back(0); packet.push_back(100);  // PayloadSize = 100, but no actual payload

    auto result = processor.processRawData("endpoint1",
        std::span<const uint8_t>(packet.data(), packet.size()));

    EXPECT_FALSE(result.has_value());
}

// ============================================================================
// VERBOSE MODE TESTS
// ============================================================================

TEST_F(PacketProcessorTest, VerboseMode_LogsAcceptedPacket) {
    PacketProcessor processor(metrics_, true);

    processor.registerConnection("endpoint1", 1);

    auto packet = createValidPacket(OpCode::C_INPUT, 1, 1);
    auto result = processor.processRawData("endpoint1",
        std::span<const uint8_t>(packet.data(), packet.size()));

    EXPECT_TRUE(result.has_value());
}

TEST_F(PacketProcessorTest, VerboseMode_LogsDroppedPacket) {
    PacketProcessor processor(metrics_, true);

    std::vector<uint8_t> invalidData = {'X', 'Y', 'Z', 'W'};
    auto result = processor.processRawData("endpoint1",
        std::span<const uint8_t>(invalidData.data(), invalidData.size()));

    EXPECT_FALSE(result.has_value());
}
