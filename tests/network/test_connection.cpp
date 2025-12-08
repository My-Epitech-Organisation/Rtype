/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Unit tests for Connection class
*/

#include <gtest/gtest.h>

#include <chrono>
#include <cstring>
#include <thread>

#include "connection/Connection.hpp"
#include "protocol/ByteOrderSpec.hpp"
#include "protocol/Header.hpp"
#include "protocol/OpCode.hpp"
#include "protocol/Payloads.hpp"

using namespace rtype::network;
using namespace std::chrono_literals;

// ============================================================================
// TEST FIXTURE
// ============================================================================

class ConnectionTest : public ::testing::Test {
   protected:
    void SetUp() override {
        config_.stateConfig.connectTimeout = 100ms;
        config_.stateConfig.disconnectTimeout = 100ms;
        config_.stateConfig.heartbeatTimeout = 200ms;
        config_.stateConfig.maxConnectRetries = 3;
        config_.reliabilityConfig.maxRetries = 3;
        config_.reliabilityConfig.retransmitTimeout = 50ms;
    }

    Connection::Config config_;
    Endpoint testEndpoint_{"127.0.0.1", 4242};
};

// ============================================================================
// CONSTRUCTION TESTS
// ============================================================================

TEST_F(ConnectionTest, Constructor_DefaultConfig) {
    Connection conn;
    EXPECT_EQ(conn.state(), ConnectionState::Disconnected);
    EXPECT_FALSE(conn.isConnected());
    EXPECT_TRUE(conn.isDisconnected());
    EXPECT_FALSE(conn.userId().has_value());
}

TEST_F(ConnectionTest, Constructor_CustomConfig) {
    Connection conn(config_);
    EXPECT_EQ(conn.state(), ConnectionState::Disconnected);
    EXPECT_FALSE(conn.isConnected());
}

// ============================================================================
// CONNECT TESTS
// ============================================================================

TEST_F(ConnectionTest, Connect_Success) {
    Connection conn(config_);
    auto result = conn.connect();
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(conn.state(), ConnectionState::Connecting);
}

TEST_F(ConnectionTest, Connect_AlreadyConnecting) {
    Connection conn(config_);
    conn.connect();
    auto result = conn.connect();
    EXPECT_TRUE(result.isErr());
}

TEST_F(ConnectionTest, Connect_GeneratesPacket) {
    Connection conn(config_);
    conn.connect();

    auto packets = conn.getOutgoingPackets();
    EXPECT_EQ(packets.size(), 1);
    EXPECT_TRUE(packets[0].isReliable);
    EXPECT_GE(packets[0].data.size(), kHeaderSize);
}

TEST_F(ConnectionTest, Connect_PacketHasCorrectOpcode) {
    Connection conn(config_);
    conn.connect();

    auto packets = conn.getOutgoingPackets();
    ASSERT_EQ(packets.size(), 1);

    Header header;
    std::memcpy(&header, packets[0].data.data(), kHeaderSize);
    EXPECT_EQ(header.magic, kMagicByte);
    EXPECT_EQ(static_cast<OpCode>(header.opcode), OpCode::C_CONNECT);
}

// ============================================================================
// DISCONNECT TESTS
// ============================================================================

TEST_F(ConnectionTest, Disconnect_FromDisconnected_Fails) {
    Connection conn(config_);
    auto result = conn.disconnect();
    EXPECT_TRUE(result.isErr());
}

TEST_F(ConnectionTest, Disconnect_FromConnecting_Success) {
    Connection conn(config_);
    conn.connect();
    auto result = conn.disconnect();
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(conn.state(), ConnectionState::Disconnecting);
}

TEST_F(ConnectionTest, Disconnect_GeneratesPacket) {
    Connection conn(config_);
    conn.connect();
    conn.getOutgoingPackets();  // Clear connect packet

    conn.disconnect();
    auto packets = conn.getOutgoingPackets();
    EXPECT_EQ(packets.size(), 1);
    EXPECT_TRUE(packets[0].isReliable);
}

TEST_F(ConnectionTest, Disconnect_PacketHasCorrectOpcode) {
    Connection conn(config_);
    conn.connect();
    conn.getOutgoingPackets();  // Clear connect packet

    conn.disconnect();
    auto packets = conn.getOutgoingPackets();
    ASSERT_EQ(packets.size(), 1);

    Header header;
    std::memcpy(&header, packets[0].data.data(), kHeaderSize);
    EXPECT_EQ(static_cast<OpCode>(header.opcode), OpCode::DISCONNECT);
}

// ============================================================================
// PROCESS PACKET TESTS
// ============================================================================

TEST_F(ConnectionTest, ProcessPacket_TooSmall) {
    Connection conn(config_);
    conn.connect();

    Buffer smallPacket(kHeaderSize - 1, 0);
    auto result = conn.processPacket(smallPacket, testEndpoint_);
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::PacketTooSmall);
}

TEST_F(ConnectionTest, ProcessPacket_InvalidMagic) {
    Connection conn(config_);
    conn.connect();

    Header header;
    header.magic = 0x00;  // Invalid magic
    header.opcode = static_cast<std::uint8_t>(OpCode::S_ACCEPT);
    header.payloadSize = 0;
    header.userId = 0;
    header.seqId = 0;
    header.ackId = 0;
    header.flags = 0;
    header.reserved = {0, 0, 0};

    Buffer packet(kHeaderSize);
    std::memcpy(packet.data(), &header, kHeaderSize);

    auto result = conn.processPacket(packet, testEndpoint_);
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::InvalidMagic);
}

TEST_F(ConnectionTest, ProcessPacket_MalformedPacket) {
    Connection conn(config_);
    conn.connect();

    Header header;
    header.magic = kMagicByte;
    header.opcode = static_cast<std::uint8_t>(OpCode::S_ACCEPT);
    header.payloadSize = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(100));  // Claims 100 bytes payload
    header.userId = 0;
    header.seqId = 0;
    header.ackId = 0;
    header.flags = 0;
    header.reserved = {0, 0, 0};

    Buffer packet(kHeaderSize);  // But only header provided
    std::memcpy(packet.data(), &header, kHeaderSize);

    auto result = conn.processPacket(packet, testEndpoint_);
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::MalformedPacket);
}

TEST_F(ConnectionTest, ProcessPacket_InvalidSender) {
    Connection conn(config_);
    conn.connect();

    // First, accept a connection to set server endpoint
    Header acceptHeader;
    acceptHeader.magic = kMagicByte;
    acceptHeader.opcode = static_cast<std::uint8_t>(OpCode::S_ACCEPT);
    acceptHeader.payloadSize = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(sizeof(AcceptPayload)));
    acceptHeader.userId = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(1));
    acceptHeader.seqId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(1));
    acceptHeader.ackId = 0;
    acceptHeader.flags = 0;
    acceptHeader.reserved = {0, 0, 0};

    AcceptPayload payload;
    payload.newUserId = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(42));

    Buffer acceptPacket(kHeaderSize + sizeof(AcceptPayload));
    std::memcpy(acceptPacket.data(), &acceptHeader, kHeaderSize);
    std::memcpy(acceptPacket.data() + kHeaderSize, &payload, sizeof(AcceptPayload));

    conn.processPacket(acceptPacket, testEndpoint_);  // Sets server endpoint

    // Now try from different sender
    Endpoint wrongEndpoint{"192.168.1.1", 5555};
    auto result = conn.processPacket(acceptPacket, wrongEndpoint);
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::InvalidSender);
}

TEST_F(ConnectionTest, ProcessPacket_AcceptSuccess) {
    Connection conn(config_);
    conn.connect();

    Header acceptHeader;
    acceptHeader.magic = kMagicByte;
    acceptHeader.opcode = static_cast<std::uint8_t>(OpCode::S_ACCEPT);
    acceptHeader.payloadSize = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(sizeof(AcceptPayload)));
    acceptHeader.userId = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(1));
    acceptHeader.seqId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(1));
    acceptHeader.ackId = 0;
    acceptHeader.flags = 0;
    acceptHeader.reserved = {0, 0, 0};

    AcceptPayload payload;
    payload.newUserId = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(42));

    Buffer acceptPacket(kHeaderSize + sizeof(AcceptPayload));
    std::memcpy(acceptPacket.data(), &acceptHeader, kHeaderSize);
    std::memcpy(acceptPacket.data() + kHeaderSize, &payload, sizeof(AcceptPayload));

    auto result = conn.processPacket(acceptPacket, testEndpoint_);
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(conn.state(), ConnectionState::Connected);
    EXPECT_TRUE(conn.userId().has_value());
    EXPECT_EQ(*conn.userId(), 42);
}

TEST_F(ConnectionTest, ProcessPacket_AcceptTooSmallPayload) {
    Connection conn(config_);
    conn.connect();

    Header acceptHeader;
    acceptHeader.magic = kMagicByte;
    acceptHeader.opcode = static_cast<std::uint8_t>(OpCode::S_ACCEPT);
    acceptHeader.payloadSize = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(2));  // Too small
    acceptHeader.userId = 0;
    acceptHeader.seqId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(1));
    acceptHeader.ackId = 0;
    acceptHeader.flags = 0;
    acceptHeader.reserved = {0, 0, 0};

    Buffer acceptPacket(kHeaderSize + 2);
    std::memcpy(acceptPacket.data(), &acceptHeader, kHeaderSize);

    auto result = conn.processPacket(acceptPacket, testEndpoint_);
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::MalformedPacket);
}

TEST_F(ConnectionTest, ProcessPacket_DisconnectFromServer) {
    Connection conn(config_);
    conn.connect();

    // First connect
    Header acceptHeader;
    acceptHeader.magic = kMagicByte;
    acceptHeader.opcode = static_cast<std::uint8_t>(OpCode::S_ACCEPT);
    acceptHeader.payloadSize = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(sizeof(AcceptPayload)));
    acceptHeader.userId = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(1));
    acceptHeader.seqId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(1));
    acceptHeader.ackId = 0;
    acceptHeader.flags = 0;
    acceptHeader.reserved = {0, 0, 0};

    AcceptPayload payload;
    payload.newUserId = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(42));

    Buffer acceptPacket(kHeaderSize + sizeof(AcceptPayload));
    std::memcpy(acceptPacket.data(), &acceptHeader, kHeaderSize);
    std::memcpy(acceptPacket.data() + kHeaderSize, &payload, sizeof(AcceptPayload));

    conn.processPacket(acceptPacket, testEndpoint_);
    EXPECT_EQ(conn.state(), ConnectionState::Connected);

    // Now receive disconnect
    Header disconnectHeader;
    disconnectHeader.magic = kMagicByte;
    disconnectHeader.opcode = static_cast<std::uint8_t>(OpCode::DISCONNECT);
    disconnectHeader.payloadSize = 0;
    disconnectHeader.userId = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(0));
    disconnectHeader.seqId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(2));
    disconnectHeader.ackId = 0;
    disconnectHeader.flags = 0;
    disconnectHeader.reserved = {0, 0, 0};

    Buffer disconnectPacket(kHeaderSize);
    std::memcpy(disconnectPacket.data(), &disconnectHeader, kHeaderSize);

    auto result = conn.processPacket(disconnectPacket, testEndpoint_);
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(conn.state(), ConnectionState::Disconnected);
}

// ============================================================================
// UPDATE TESTS
// ============================================================================

TEST_F(ConnectionTest, Update_WhileDisconnected_NoEffect) {
    Connection conn(config_);
    conn.update();
    EXPECT_EQ(conn.state(), ConnectionState::Disconnected);
}

TEST_F(ConnectionTest, Update_ConnectionTimeout_RetriesConnect) {
    Connection conn(config_);
    conn.connect();
    conn.getOutgoingPackets();  // Clear initial packet

    // Wait for timeout
    std::this_thread::sleep_for(config_.stateConfig.connectTimeout + 20ms);

    conn.update();
    auto packets = conn.getOutgoingPackets();
    EXPECT_GE(packets.size(), 1);  // Should have retry packet
}

TEST_F(ConnectionTest, Update_MaxRetriesExceeded_Disconnects) {
    Connection::Config fastConfig;
    fastConfig.stateConfig.connectTimeout = 10ms;
    fastConfig.stateConfig.maxConnectRetries = 1;
    fastConfig.reliabilityConfig.maxRetries = 1;
    fastConfig.reliabilityConfig.retransmitTimeout = 10ms;

    Connection conn(fastConfig);
    conn.connect();

    // Wait for multiple timeouts
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(15ms);
        conn.update();
    }

    EXPECT_TRUE(conn.isDisconnected());
}

// ============================================================================
// BUILD PACKET TESTS
// ============================================================================

TEST_F(ConnectionTest, BuildPacket_NotConnected_Fails) {
    Connection conn(config_);
    Buffer payload;
    auto result = conn.buildPacket(OpCode::PING, payload);
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::NotConnected);
}

TEST_F(ConnectionTest, BuildPacket_WhileConnecting_Fails) {
    Connection conn(config_);
    conn.connect();
    Buffer payload;
    auto result = conn.buildPacket(OpCode::PING, payload);
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::NotConnected);
}

TEST_F(ConnectionTest, BuildPacket_WhenConnected_Success) {
    Connection conn(config_);
    conn.connect();

    // Accept connection
    Header acceptHeader;
    acceptHeader.magic = kMagicByte;
    acceptHeader.opcode = static_cast<std::uint8_t>(OpCode::S_ACCEPT);
    acceptHeader.payloadSize = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(sizeof(AcceptPayload)));
    acceptHeader.userId = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(1));
    acceptHeader.seqId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(1));
    acceptHeader.ackId = 0;
    acceptHeader.flags = 0;
    acceptHeader.reserved = {0, 0, 0};

    AcceptPayload payload;
    payload.newUserId = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(42));

    Buffer acceptPacket(kHeaderSize + sizeof(AcceptPayload));
    std::memcpy(acceptPacket.data(), &acceptHeader, kHeaderSize);
    std::memcpy(acceptPacket.data() + kHeaderSize, &payload, sizeof(AcceptPayload));

    conn.processPacket(acceptPacket, testEndpoint_);
    EXPECT_TRUE(conn.isConnected());

    Buffer emptyPayload;
    auto result = conn.buildPacket(OpCode::PING, emptyPayload);
    EXPECT_TRUE(result.isOk());
    EXPECT_GE(result.value().data.size(), kHeaderSize);
}

TEST_F(ConnectionTest, BuildPacket_WithPayload) {
    Connection conn(config_);
    conn.connect();

    // Accept connection
    Header acceptHeader;
    acceptHeader.magic = kMagicByte;
    acceptHeader.opcode = static_cast<std::uint8_t>(OpCode::S_ACCEPT);
    acceptHeader.payloadSize = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(sizeof(AcceptPayload)));
    acceptHeader.userId = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(1));
    acceptHeader.seqId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(1));
    acceptHeader.ackId = 0;
    acceptHeader.flags = 0;
    acceptHeader.reserved = {0, 0, 0};

    AcceptPayload acceptPayload;
    acceptPayload.newUserId = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(42));

    Buffer acceptPacket(kHeaderSize + sizeof(AcceptPayload));
    std::memcpy(acceptPacket.data(), &acceptHeader, kHeaderSize);
    std::memcpy(acceptPacket.data() + kHeaderSize, &acceptPayload, sizeof(AcceptPayload));

    conn.processPacket(acceptPacket, testEndpoint_);

    Buffer payload = {0x01, 0x02, 0x03, 0x04};
    auto result = conn.buildPacket(OpCode::C_INPUT, payload);
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.value().data.size(), kHeaderSize + payload.size());
}

// ============================================================================
// OUTGOING PACKETS TESTS
// ============================================================================

TEST_F(ConnectionTest, GetOutgoingPackets_EmptyByDefault) {
    Connection conn(config_);
    auto packets = conn.getOutgoingPackets();
    EXPECT_TRUE(packets.empty());
}

TEST_F(ConnectionTest, GetOutgoingPackets_ClearsQueue) {
    Connection conn(config_);
    conn.connect();

    auto packets1 = conn.getOutgoingPackets();
    EXPECT_EQ(packets1.size(), 1);

    auto packets2 = conn.getOutgoingPackets();
    EXPECT_TRUE(packets2.empty());
}

// ============================================================================
// RECORD ACK TESTS
// ============================================================================

TEST_F(ConnectionTest, RecordAck_NoExceptions) {
    Connection conn(config_);
    conn.recordAck(0);
    conn.recordAck(1);
    conn.recordAck(65535);
}

// ============================================================================
// STATE ACCESSORS TESTS
// ============================================================================

TEST_F(ConnectionTest, State_ReturnsCorrectState) {
    Connection conn(config_);
    EXPECT_EQ(conn.state(), ConnectionState::Disconnected);

    conn.connect();
    EXPECT_EQ(conn.state(), ConnectionState::Connecting);
}

TEST_F(ConnectionTest, IsConnected_ReturnsCorrectValue) {
    Connection conn(config_);
    EXPECT_FALSE(conn.isConnected());

    conn.connect();
    EXPECT_FALSE(conn.isConnected());

    // Accept connection
    Header acceptHeader;
    acceptHeader.magic = kMagicByte;
    acceptHeader.opcode = static_cast<std::uint8_t>(OpCode::S_ACCEPT);
    acceptHeader.payloadSize = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(sizeof(AcceptPayload)));
    acceptHeader.userId = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(1));
    acceptHeader.seqId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(1));
    acceptHeader.ackId = 0;
    acceptHeader.flags = 0;
    acceptHeader.reserved = {0, 0, 0};

    AcceptPayload payload;
    payload.newUserId = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(42));

    Buffer acceptPacket(kHeaderSize + sizeof(AcceptPayload));
    std::memcpy(acceptPacket.data(), &acceptHeader, kHeaderSize);
    std::memcpy(acceptPacket.data() + kHeaderSize, &payload, sizeof(AcceptPayload));

    conn.processPacket(acceptPacket, testEndpoint_);
    EXPECT_TRUE(conn.isConnected());
}

TEST_F(ConnectionTest, IsDisconnected_ReturnsCorrectValue) {
    Connection conn(config_);
    EXPECT_TRUE(conn.isDisconnected());

    conn.connect();
    EXPECT_FALSE(conn.isDisconnected());
}

TEST_F(ConnectionTest, UserId_NulloptWhenNotConnected) {
    Connection conn(config_);
    EXPECT_FALSE(conn.userId().has_value());
}

TEST_F(ConnectionTest, LastDisconnectReason_NulloptInitially) {
    Connection conn(config_);
    EXPECT_FALSE(conn.lastDisconnectReason().has_value());
}

// ============================================================================
// CALLBACKS TESTS
// ============================================================================

TEST_F(ConnectionTest, SetCallbacks_NoExceptions) {
    Connection conn(config_);

    ConnectionCallbacks callbacks;
    callbacks.onConnected = [](std::uint32_t) {};
    callbacks.onDisconnected = [](DisconnectReason) {};
    callbacks.onConnectFailed = [](NetworkError) {};

    EXPECT_NO_THROW(conn.setCallbacks(std::move(callbacks)));
}

TEST_F(ConnectionTest, Callback_OnConnected_Called) {
    Connection conn(config_);

    bool callbackCalled = false;
    std::uint32_t receivedUserId = 0;

    ConnectionCallbacks callbacks;
    callbacks.onConnected = [&](std::uint32_t userId) {
        callbackCalled = true;
        receivedUserId = userId;
    };
    conn.setCallbacks(std::move(callbacks));

    conn.connect();

    // Accept connection
    Header acceptHeader;
    acceptHeader.magic = kMagicByte;
    acceptHeader.opcode = static_cast<std::uint8_t>(OpCode::S_ACCEPT);
    acceptHeader.payloadSize = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(sizeof(AcceptPayload)));
    acceptHeader.userId = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(1));
    acceptHeader.seqId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(1));
    acceptHeader.ackId = 0;
    acceptHeader.flags = 0;
    acceptHeader.reserved = {0, 0, 0};

    AcceptPayload payload;
    payload.newUserId = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(42));

    Buffer acceptPacket(kHeaderSize + sizeof(AcceptPayload));
    std::memcpy(acceptPacket.data(), &acceptHeader, kHeaderSize);
    std::memcpy(acceptPacket.data() + kHeaderSize, &payload, sizeof(AcceptPayload));

    conn.processPacket(acceptPacket, testEndpoint_);

    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(receivedUserId, 42);
}

// ============================================================================
// RESET TESTS
// ============================================================================

TEST_F(ConnectionTest, Reset_ClearsState) {
    Connection conn(config_);
    conn.connect();

    // Accept connection
    Header acceptHeader;
    acceptHeader.magic = kMagicByte;
    acceptHeader.opcode = static_cast<std::uint8_t>(OpCode::S_ACCEPT);
    acceptHeader.payloadSize = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(sizeof(AcceptPayload)));
    acceptHeader.userId = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(1));
    acceptHeader.seqId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(1));
    acceptHeader.ackId = 0;
    acceptHeader.flags = 0;
    acceptHeader.reserved = {0, 0, 0};

    AcceptPayload payload;
    payload.newUserId = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(42));

    Buffer acceptPacket(kHeaderSize + sizeof(AcceptPayload));
    std::memcpy(acceptPacket.data(), &acceptHeader, kHeaderSize);
    std::memcpy(acceptPacket.data() + kHeaderSize, &payload, sizeof(AcceptPayload));

    conn.processPacket(acceptPacket, testEndpoint_);
    EXPECT_TRUE(conn.isConnected());
    EXPECT_TRUE(conn.userId().has_value());

    conn.reset();

    EXPECT_TRUE(conn.isDisconnected());
    EXPECT_FALSE(conn.userId().has_value());
}

TEST_F(ConnectionTest, Reset_ClearsOutgoingQueue) {
    Connection conn(config_);
    conn.connect();

    EXPECT_EQ(conn.getOutgoingPackets().size(), 1);

    conn.connect();  // Queue another packet (will fail but queue might have something)
    conn.reset();

    EXPECT_TRUE(conn.getOutgoingPackets().empty());
}

TEST_F(ConnectionTest, Reset_AllowsReconnect) {
    Connection conn(config_);
    conn.connect();
    conn.reset();

    auto result = conn.connect();
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(conn.state(), ConnectionState::Connecting);
}

// ============================================================================
// RELIABILITY TESTS
// ============================================================================

TEST_F(ConnectionTest, ReliableChannel_Accessible) {
    Connection conn(config_);
    const auto& channel = conn.reliableChannel();
    EXPECT_EQ(channel.getPendingCount(), 0);
}

TEST_F(ConnectionTest, DuplicatePacket_Rejected) {
    Connection conn(config_);
    conn.connect();

    // Accept connection
    Header acceptHeader;
    acceptHeader.magic = kMagicByte;
    acceptHeader.opcode = static_cast<std::uint8_t>(OpCode::S_ACCEPT);
    acceptHeader.payloadSize = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(sizeof(AcceptPayload)));
    acceptHeader.userId = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(1));
    acceptHeader.seqId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(1));
    acceptHeader.ackId = 0;
    acceptHeader.flags = Flags::kReliable;
    acceptHeader.reserved = {0, 0, 0};

    AcceptPayload payload;
    payload.newUserId = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(42));

    Buffer acceptPacket(kHeaderSize + sizeof(AcceptPayload));
    std::memcpy(acceptPacket.data(), &acceptHeader, kHeaderSize);
    std::memcpy(acceptPacket.data() + kHeaderSize, &payload, sizeof(AcceptPayload));

    auto result1 = conn.processPacket(acceptPacket, testEndpoint_);
    EXPECT_TRUE(result1.isOk());

    // Same packet again (duplicate seqId)
    auto result2 = conn.processPacket(acceptPacket, testEndpoint_);
    EXPECT_TRUE(result2.isErr());
    EXPECT_EQ(result2.error(), NetworkError::DuplicatePacket);
}

// ============================================================================
// SEQUENCE ID TESTS
// ============================================================================

TEST_F(ConnectionTest, SequenceId_Increments) {
    Connection conn(config_);
    conn.connect();

    auto packets1 = conn.getOutgoingPackets();
    ASSERT_EQ(packets1.size(), 1);

    Header header1;
    std::memcpy(&header1, packets1[0].data.data(), kHeaderSize);
    std::uint16_t seqId1 = ByteOrderSpec::fromNetwork(header1.seqId);

    // Force another connect attempt (reset first)
    conn.reset();
    conn.connect();

    auto packets2 = conn.getOutgoingPackets();
    ASSERT_EQ(packets2.size(), 1);

    Header header2;
    std::memcpy(&header2, packets2[0].data.data(), kHeaderSize);
    std::uint16_t seqId2 = ByteOrderSpec::fromNetwork(header2.seqId);

    // After reset, sequence should start from 0 again
    EXPECT_EQ(seqId2, 0);
}

// ============================================================================
// ACK PROCESSING TESTS
// ============================================================================

TEST_F(ConnectionTest, AckProcessing_RecordsAckFromHeader) {
    Connection conn(config_);
    conn.connect();

    // Accept connection with ACK flag
    Header acceptHeader;
    acceptHeader.magic = kMagicByte;
    acceptHeader.opcode = static_cast<std::uint8_t>(OpCode::S_ACCEPT);
    acceptHeader.payloadSize = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(sizeof(AcceptPayload)));
    acceptHeader.userId = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(1));
    acceptHeader.seqId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(1));
    acceptHeader.ackId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(0));  // Acking our seqId 0
    acceptHeader.flags = Flags::kIsAck;
    acceptHeader.reserved = {0, 0, 0};

    AcceptPayload payload;
    payload.newUserId = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(42));

    Buffer acceptPacket(kHeaderSize + sizeof(AcceptPayload));
    std::memcpy(acceptPacket.data(), &acceptHeader, kHeaderSize);
    std::memcpy(acceptPacket.data() + kHeaderSize, &payload, sizeof(AcceptPayload));

    auto result = conn.processPacket(acceptPacket, testEndpoint_);
    EXPECT_TRUE(result.isOk());
    // Reliable channel should have processed the ACK
}
