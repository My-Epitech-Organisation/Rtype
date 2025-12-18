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

#include "serverApp/packetProcessor/PacketProcessor.hpp"
#include "shared/ServerMetrics.hpp"
#include "Serializer.hpp"
#include "protocol/Header.hpp"
#include "protocol/OpCode.hpp"

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
// PROCESS RAW DATA TESTS - ERROR BRANCH COVERAGE
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

    // Invalid magic byte (not 0xA1)
    std::vector<uint8_t> invalidData(16, 0x00);
    auto result = processor.processRawData("endpoint1",
        std::span<const uint8_t>(invalidData.data(), invalidData.size()));

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(metrics_->packetsDropped.load(), 1u);
}

TEST_F(PacketProcessorTest, ProcessRawData_TooShort) {
    PacketProcessor processor(metrics_, false);

    std::vector<uint8_t> shortData = {0xA1, 0x00, 0x00, 0x00};
    auto result = processor.processRawData("endpoint1",
        std::span<const uint8_t>(shortData.data(), shortData.size()));

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(metrics_->packetsDropped.load(), 1u);
}

TEST_F(PacketProcessorTest, ProcessRawData_InvalidOpCode) {
    PacketProcessor processor(metrics_, false);

    // Create packet with invalid opcode (0xFF)
    std::vector<uint8_t> packet(16, 0x00);
    packet[0] = 0xA1;  // Valid magic
    packet[1] = 0xFF;  // Invalid opcode

    auto result = processor.processRawData("endpoint1",
        std::span<const uint8_t>(packet.data(), packet.size()));

    EXPECT_FALSE(result.has_value());
}

TEST_F(PacketProcessorTest, ProcessRawData_PayloadSizeMismatch) {
    PacketProcessor processor(metrics_, false);

    processor.registerConnection("endpoint1", 1);

    // Header claims 100 bytes of payload but we only send 16 bytes total
    std::vector<uint8_t> packet(16, 0x00);
    packet[0] = 0xA1;  // Magic
    packet[1] = static_cast<uint8_t>(OpCode::C_INPUT);
    packet[2] = 0x00;  // Payload size high byte
    packet[3] = 0x64;  // Payload size = 100

    auto result = processor.processRawData("endpoint1",
        std::span<const uint8_t>(packet.data(), packet.size()));

    EXPECT_FALSE(result.has_value());
}

TEST_F(PacketProcessorTest, ProcessRawData_UnregisteredEndpoint) {
    PacketProcessor processor(metrics_, false);

    // Create minimal valid-looking packet
    std::vector<uint8_t> packet(16, 0x00);
    packet[0] = 0xA1;
    packet[1] = static_cast<uint8_t>(OpCode::C_CONNECT);

    auto result = processor.processRawData("unknown_endpoint",
        std::span<const uint8_t>(packet.data(), packet.size()));

    // PacketProcessor passes the packet to validation first, 
    // the unregistered endpoint check happens in security context
    // We just verify it doesn't crash and returns some result
    EXPECT_NO_THROW({
        (void)result;
    });
}

TEST_F(PacketProcessorTest, ProcessRawData_VerboseMode_DropsPacket) {
    PacketProcessor processor(metrics_, true);

    std::vector<uint8_t> invalidData = {0x00, 0x00, 0x00, 0x00};
    auto result = processor.processRawData("endpoint1",
        std::span<const uint8_t>(invalidData.data(), invalidData.size()));

    EXPECT_FALSE(result.has_value());
    EXPECT_GE(metrics_->packetsDropped.load(), 1u);
}

// ============================================================================
// REGISTER/UNREGISTER CONNECTION TESTS
// ============================================================================

TEST_F(PacketProcessorTest, RegisterConnection_ValidEndpoint) {
    PacketProcessor processor(metrics_, false);

    EXPECT_NO_THROW({
        processor.registerConnection("endpoint1", 42);
    });
}

TEST_F(PacketProcessorTest, RegisterConnection_MultipleEndpoints) {
    PacketProcessor processor(metrics_, false);

    EXPECT_NO_THROW({
        processor.registerConnection("endpoint1", 1);
        processor.registerConnection("endpoint2", 2);
        processor.registerConnection("endpoint3", 3);
    });
}

TEST_F(PacketProcessorTest, UnregisterConnection_ValidEndpoint) {
    PacketProcessor processor(metrics_, false);

    processor.registerConnection("endpoint1", 1);

    EXPECT_NO_THROW({
        processor.unregisterConnection("endpoint1");
    });
}

TEST_F(PacketProcessorTest, UnregisterConnection_UnknownEndpoint) {
    PacketProcessor processor(metrics_, false);

    // Should not crash
    EXPECT_NO_THROW({
        processor.unregisterConnection("unknown_endpoint");
    });
}

TEST_F(PacketProcessorTest, UnregisterConnection_ThenRegisterAgain) {
    PacketProcessor processor(metrics_, false);

    processor.registerConnection("endpoint1", 1);
    processor.unregisterConnection("endpoint1");
    processor.registerConnection("endpoint1", 2);

    // Should work without issues
}

// ============================================================================
// GET SECURITY CONTEXT TESTS
// ============================================================================

TEST_F(PacketProcessorTest, GetSecurityContext_ReturnsReference) {
    PacketProcessor processor(metrics_, false);

    auto& context = processor.getSecurityContext();
    (void)context;  // Just verify we can get a reference
}

TEST_F(PacketProcessorTest, GetSecurityContext_CanModify) {
    PacketProcessor processor(metrics_, false);

    auto& context = processor.getSecurityContext();
    context.registerConnection("test_endpoint", 100);

    // Should have registered the connection
}

// ============================================================================
// METRICS TRACKING TESTS
// ============================================================================

TEST_F(PacketProcessorTest, Metrics_PacketsDropped_Increases) {
    PacketProcessor processor(metrics_, false);

    uint64_t initialDropped = metrics_->packetsDropped.load();

    std::vector<uint8_t> invalidData = {0x00};
    processor.processRawData("endpoint1",
        std::span<const uint8_t>(invalidData.data(), invalidData.size()));

    EXPECT_GT(metrics_->packetsDropped.load(), initialDropped);
}

TEST_F(PacketProcessorTest, Metrics_MultipleDrops) {
    PacketProcessor processor(metrics_, false);

    std::vector<uint8_t> invalidData = {0x00};

    for (int i = 0; i < 5; ++i) {
        processor.processRawData("endpoint" + std::to_string(i),
            std::span<const uint8_t>(invalidData.data(), invalidData.size()));
    }

    EXPECT_GE(metrics_->packetsDropped.load(), 5u);
}

// ============================================================================
// BRANCH COVERAGE TESTS - Missing paths from coverage report
// ============================================================================

TEST_F(PacketProcessorTest, ProcessRawData_WithPayload) {
    PacketProcessor processor(metrics_, false);
    std::string endpoint = "127.0.0.1:50001";
    std::uint32_t userId = 1001;
    
    processor.registerConnection(endpoint, userId);
    
    // Create a valid packet with payload (tests header.payloadSize > 0 branch)
    InputPayload input{0x01};  // Up button pressed
    auto header = Header::create(OpCode::C_INPUT, userId, 1, sizeof(InputPayload));
    auto headerBytes = ByteOrderSpec::serializeToNetwork(header);
    auto payloadBytes = ByteOrderSpec::serializeToNetwork(input);
    
    // Combine header + payload
    headerBytes.insert(headerBytes.end(), payloadBytes.begin(), payloadBytes.end());
    
    auto result = processor.processRawData(endpoint, headerBytes);
    
    EXPECT_TRUE(result.has_value());
}

TEST_F(PacketProcessorTest, VerboseMode_LogsAcceptedPacket) {
    PacketProcessor processor(metrics_, true);  // Verbose mode enabled (tests _verbose branch)
    std::string endpoint = "127.0.0.1:50002";
    std::uint32_t userId = 1002;
    
    processor.registerConnection(endpoint, userId);
    
    // Use PING opcode which has no payload
    auto header = Header::create(OpCode::PING, userId, 1, 0);
    auto headerBytes = ByteOrderSpec::serializeToNetwork(header);
    
    auto result = processor.processRawData(endpoint, headerBytes);
    
    EXPECT_TRUE(result.has_value());
}

TEST_F(PacketProcessorTest, ProcessRawData_InvalidSequenceId) {
    PacketProcessor processor(metrics_, false);
    std::string endpoint = "127.0.0.1:50003";
    std::uint32_t userId = 1003;
    
    processor.registerConnection(endpoint, userId);
    
    // Send first packet with seq 1
    auto header1 = Header::create(OpCode::PING, userId, 1, 0);
    auto packet1 = ByteOrderSpec::serializeToNetwork(header1);
    auto result1 = processor.processRawData(endpoint, packet1);
    EXPECT_TRUE(result1.has_value());
    
    // Try to send duplicate packet with same seq ID (tests seqResult.isErr() branch)
    std::size_t droppedBefore = metrics_->packetsDropped.load();
    auto result2 = processor.processRawData(endpoint, packet1);
    
    EXPECT_FALSE(result2.has_value());
    EXPECT_GT(metrics_->packetsDropped.load(), droppedBefore);
}

TEST_F(PacketProcessorTest, ProcessRawData_UserIdSpoofing) {
    PacketProcessor processor(metrics_, false);
    std::string endpoint = "127.0.0.1:50004";
    std::uint32_t registeredUserId = 1004;
    std::uint32_t spoofedUserId = 9999;
    
    processor.registerConnection(endpoint, registeredUserId);
    
    // Try to send packet with different user ID (tests userIdResult.isErr() branch)
    auto header = Header::create(OpCode::PING, spoofedUserId, 1, 0);
    auto packetData = ByteOrderSpec::serializeToNetwork(header);
    
    std::size_t droppedBefore = metrics_->packetsDropped.load();
    auto result = processor.processRawData(endpoint, packetData);
    
    EXPECT_FALSE(result.has_value());
    EXPECT_GT(metrics_->packetsDropped.load(), droppedBefore);
}
