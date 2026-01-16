/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Additional branch tests for PacketProcessor
*/

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <vector>

#include "serverApp/packetProcessor/PacketProcessor.hpp"
#include "shared/ServerMetrics.hpp"
#include "protocol/Header.hpp"
#include "protocol/OpCode.hpp"
#include "protocol/ByteOrderSpec.hpp"

using namespace rtype::server;
using namespace rtype::network;

class PacketProcessorBranchesTest : public ::testing::Test {
  protected:
    void SetUp() override { metrics_ = std::make_shared<ServerMetrics>(); }
    void TearDown() override { metrics_.reset(); }

    std::shared_ptr<ServerMetrics> metrics_;
};

TEST_F(PacketProcessorBranchesTest, DuplicateSequenceIsRejected) {
    PacketProcessor processor(metrics_, false);

    // Register the endpoint so packets are accepted
    processor.registerConnection("ep_dup", 1u);

    // Build valid header (PING has no payload)
    Header header = Header::create(OpCode::PING, 1u, static_cast<std::uint16_t>(100u), 0u);
    auto bytes = ByteOrderSpec::serializeToNetwork(header);

    // First packet should be accepted
    auto r1 = processor.processRawData("ep_dup", std::span<const uint8_t>(bytes.data(), bytes.size()));
    ASSERT_TRUE(r1.has_value());

    // Second packet with same seq should be rejected as duplicate
    auto r2 = processor.processRawData("ep_dup", std::span<const uint8_t>(bytes.data(), bytes.size()));
    EXPECT_FALSE(r2.has_value());
    EXPECT_GT(metrics_->packetsDropped.load(), 0u);
}

TEST_F(PacketProcessorBranchesTest, UserIdSpoofingIsRejected) {
    PacketProcessor processor(metrics_, false);

    // Claim a user ID without registering the endpoint
    Header header = Header::create(OpCode::C_INPUT, 42u, static_cast<std::uint16_t>(1u), 0u);
    auto bytes = ByteOrderSpec::serializeToNetwork(header);

    auto res = processor.processRawData("ep_spoof", std::span<const uint8_t>(bytes.data(), bytes.size()));
    EXPECT_FALSE(res.has_value());
    EXPECT_GE(metrics_->packetsDropped.load(), 1u);
}

TEST_F(PacketProcessorBranchesTest, VerboseModeAcceptsValidPacket) {
    PacketProcessor processor(metrics_, true);

    // Register mapping then send a valid PING packet (no payload)
    processor.registerConnection("ep_ok", 7u);

    Header header = Header::create(OpCode::PING, 7u, static_cast<std::uint16_t>(1u), 0u);
    auto bytes = ByteOrderSpec::serializeToNetwork(header);

    auto res = processor.processRawData("ep_ok", std::span<const uint8_t>(bytes.data(), bytes.size()));
    EXPECT_TRUE(res.has_value());
}

TEST_F(PacketProcessorBranchesTest, PayloadIsAttachedToPacket) {
    PacketProcessor processor(metrics_, false);

    // Register mapping then send a packet with payload
    processor.registerConnection("ep_payload", 9u);

    // C_INPUT payload is 2 bytes (input mask uint16_t)
    Header header = Header::create(OpCode::C_INPUT, 9u, static_cast<std::uint16_t>(1u), 2u);
    auto headerBytes = ByteOrderSpec::serializeToNetwork(header);

    std::vector<uint8_t> bytes(headerBytes.begin(), headerBytes.end());
    // Append 2 bytes of payload (little-endian uint16_t)
    bytes.push_back(0x0F);
    bytes.push_back(0x00);

    auto res = processor.processRawData("ep_payload", std::span<const uint8_t>(bytes.data(), bytes.size()));
    ASSERT_TRUE(res.has_value());
    auto pkt = res.value();
    EXPECT_EQ(pkt.data().size(), 2u);
    EXPECT_EQ(pkt.data()[0], 0x0F);
}

TEST_F(PacketProcessorBranchesTest, UnregisterConnectionRemovesMapping) {
    PacketProcessor processor(metrics_, false);

    // Register then unregister
    processor.registerConnection("ep_tmp", 5u);
    processor.unregisterConnection("ep_tmp");

    Header header = Header::create(OpCode::C_INPUT, 5u, static_cast<std::uint16_t>(1u), 0u);
    auto bytes = ByteOrderSpec::serializeToNetwork(header);

    auto res = processor.processRawData("ep_tmp", std::span<const uint8_t>(bytes.data(), bytes.size()));
    EXPECT_FALSE(res.has_value());
    EXPECT_GT(metrics_->packetsDropped.load(), 0u);
}
