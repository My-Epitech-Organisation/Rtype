/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Test packet validation security features
*/

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include "Protocol.hpp"
#include "Serializer.hpp"

using namespace rtype::network;

/**
 * @brief Test fixture for packet validation
 */
class PacketValidationTest : public ::testing::Test {
   protected:
    void SetUp() override {
        // Create a valid header for testing
        validHeader = Header::create(OpCode::C_INPUT, 42, 100, 8);
    }

    Header validHeader;
};

/**
 * @brief Test magic byte validation
 */
TEST_F(PacketValidationTest, MagicByteValidation) {
    // Valid magic byte
    EXPECT_TRUE(Validator::validateMagic(kMagicByte).isOk());

    // Invalid magic bytes
    EXPECT_TRUE(Validator::validateMagic(0x00).isErr());
    EXPECT_TRUE(Validator::validateMagic(0xFF).isErr());
    EXPECT_TRUE(Validator::validateMagic(0xA0).isErr());
    EXPECT_TRUE(Validator::validateMagic(0xA2).isErr());
}

/**
 * @brief Test packet size validation
 */
TEST_F(PacketValidationTest, PacketSizeValidation) {
    // Too small (less than header size)
    EXPECT_EQ(Validator::validatePacketSize(0).error(),
              NetworkError::PacketTooSmall);
    EXPECT_EQ(Validator::validatePacketSize(15).error(),
              NetworkError::PacketTooSmall);

    // Valid sizes
    EXPECT_TRUE(Validator::validatePacketSize(kHeaderSize).isOk());
    EXPECT_TRUE(Validator::validatePacketSize(kHeaderSize + 100).isOk());
    EXPECT_TRUE(Validator::validatePacketSize(kMaxPacketSize).isOk());

    // Too large
    EXPECT_EQ(Validator::validatePacketSize(kMaxPacketSize + 1).error(),
              NetworkError::PacketTooLarge);
    EXPECT_EQ(Validator::validatePacketSize(10000).error(),
              NetworkError::PacketTooLarge);
}

/**
 * @brief Test payload size validation against maximum
 */
TEST_F(PacketValidationTest, PayloadMaxSizeValidation) {
    // Valid payload sizes
    EXPECT_TRUE(Validator::validatePayloadMaxSize(0).isOk());
    EXPECT_TRUE(Validator::validatePayloadMaxSize(100).isOk());
    EXPECT_TRUE(Validator::validatePayloadMaxSize(kMaxPayloadSize).isOk());

    // Too large (buffer overflow risk)
    EXPECT_EQ(Validator::validatePayloadMaxSize(kMaxPayloadSize + 1).error(),
              NetworkError::PacketTooLarge);
    EXPECT_EQ(Validator::validatePayloadMaxSize(65535).error(),
              NetworkError::PacketTooLarge);
}

/**
 * @brief Test header validation
 */
TEST_F(PacketValidationTest, HeaderValidation) {
    // Valid header
    auto header = validHeader;
    EXPECT_TRUE(Validator::validateHeader(header).isOk());

    // Invalid magic
    header = validHeader;
    header.magic = 0x00;
    EXPECT_EQ(Validator::validateHeader(header).error(),
              NetworkError::InvalidMagic);

    // Invalid opcode
    header = validHeader;
    header.opcode = 0xFF;
    EXPECT_EQ(Validator::validateHeader(header).error(),
              NetworkError::UnknownOpcode);

    // Invalid reserved bytes
    header = validHeader;
    header.reserved[0] = 0x01;
    EXPECT_EQ(Validator::validateHeader(header).error(),
              NetworkError::MalformedPacket);
}

/**
 * @brief Test UserID validation for client packets
 */
TEST_F(PacketValidationTest, ClientUserIdValidation) {
    // Valid client UserIDs
    EXPECT_TRUE(Validator::validateClientUserId(1, OpCode::C_INPUT).isOk());
    EXPECT_TRUE(Validator::validateClientUserId(kMaxClientUserId, OpCode::C_INPUT).isOk());

    // Unassigned during C_CONNECT is valid
    EXPECT_TRUE(Validator::validateClientUserId(kUnassignedUserId, OpCode::C_CONNECT).isOk());

    // Server UserID not allowed for client
    EXPECT_EQ(Validator::validateClientUserId(kServerUserId, OpCode::C_INPUT).error(),
              NetworkError::InvalidUserId);

    // Unassigned not allowed after connection
    EXPECT_EQ(Validator::validateClientUserId(kUnassignedUserId, OpCode::C_INPUT).error(),
              NetworkError::InvalidUserId);
}

/**
 * @brief Test UserID validation for server packets
 */
TEST_F(PacketValidationTest, ServerUserIdValidation) {
    // Only server UserID is valid
    EXPECT_TRUE(Validator::validateServerUserId(kServerUserId).isOk());

    // Client UserIDs not allowed for server
    EXPECT_EQ(Validator::validateServerUserId(1).error(),
              NetworkError::InvalidUserId);
    EXPECT_EQ(Validator::validateServerUserId(kUnassignedUserId).error(),
              NetworkError::InvalidUserId);
}

/**
 * @brief Test bounds checking before deserialization
 */
TEST_F(PacketValidationTest, BufferBoundsValidation) {
    std::vector<std::uint8_t> buffer(100, 0);

    // Valid bounds
    EXPECT_TRUE(Validator::validateBufferBounds(buffer, 0, 50).isOk());
    EXPECT_TRUE(Validator::validateBufferBounds(buffer, 50, 50).isOk());
    EXPECT_TRUE(Validator::validateBufferBounds(buffer, 0, 100).isOk());

    // Invalid bounds (out of range)
    EXPECT_EQ(Validator::validateBufferBounds(buffer, 0, 101).error(),
              NetworkError::MalformedPacket);
    EXPECT_EQ(Validator::validateBufferBounds(buffer, 50, 51).error(),
              NetworkError::MalformedPacket);
    EXPECT_EQ(Validator::validateBufferBounds(buffer, 101, 1).error(),
              NetworkError::MalformedPacket);
}

/**
 * @brief Test safe deserialization with bounds checking
 */
TEST_F(PacketValidationTest, SafeDeserialize) {
    std::vector<std::uint8_t> buffer(16, 0);
    buffer[0] = kMagicByte;

    // Valid deserialization
    auto result = Validator::safeDeserialize<std::uint8_t>(buffer, 0);
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.value(), kMagicByte);

    // Out of bounds
    auto result2 = Validator::safeDeserialize<Header>(buffer, 1);
    EXPECT_TRUE(result2.isErr());
    EXPECT_EQ(result2.error(), NetworkError::MalformedPacket);
}

/**
 * @brief Test complete packet validation pipeline
 */
TEST_F(PacketValidationTest, CompletePacketValidation) {
    // Create a valid packet
    auto header = Header::create(OpCode::S_ENTITY_MOVE, kServerUserId, 1, static_cast<std::uint16_t>(sizeof(EntityMovePayload)));
    std::vector<std::uint8_t> payload(sizeof(EntityMovePayload), 0x42);
    
    auto headerBytes = ByteOrderSpec::serializeToNetwork(header);
    headerBytes.insert(headerBytes.end(), payload.begin(), payload.end());

    // Valid packet from server
    EXPECT_TRUE(Validator::validatePacket(headerBytes, true).isOk());

    // Invalid: client sending with server UserID
    EXPECT_TRUE(Validator::validatePacket(headerBytes, false).isErr());
}

/**
 * @brief Test malformed packet handling (zero crashes)
 */
TEST_F(PacketValidationTest, MalformedPacketsNoCrash) {
    // Empty packet
    std::vector<std::uint8_t> empty;
    EXPECT_TRUE(Validator::validatePacket(empty, false).isErr());

    // Too small
    std::vector<std::uint8_t> tooSmall(8, 0xFF);
    EXPECT_TRUE(Validator::validatePacket(tooSmall, false).isErr());

    // Wrong magic
    std::vector<std::uint8_t> wrongMagic(kHeaderSize, 0);
    EXPECT_TRUE(Validator::validatePacket(wrongMagic, false).isErr());

    // Payload size mismatch
    auto header = validHeader;
    header.payloadSize = 100;  // Claim 100 bytes
    auto bytes = ByteOrderSpec::serializeToNetwork(header);
    // Only add 10 bytes of payload
    bytes.insert(bytes.end(), 10, 0x00);
    EXPECT_TRUE(Validator::validatePacket(bytes, false).isErr());
}

/**
 * @brief Test fixture for SecurityContext
 */
class SecurityContextTest : public ::testing::Test {
   protected:
    SecurityContext context;
    const std::string testConnection = "127.0.0.1:12345";
};

/**
 * @brief Test sequence ID validation (first packet)
 */
TEST_F(SecurityContextTest, FirstPacketAccepted) {
    auto result = context.validateSequenceId(testConnection, 100);
    EXPECT_TRUE(result.isOk());
}

/**
 * @brief Test duplicate packet detection
 */
TEST_F(SecurityContextTest, DuplicatePacketRejected) {
    context.validateSequenceId(testConnection, 100);
    
    // Try to send same sequence again
    auto result = context.validateSequenceId(testConnection, 100);
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::DuplicatePacket);
}

/**
 * @brief Test in-order packet acceptance
 */
TEST_F(SecurityContextTest, InOrderPacketsAccepted) {
    EXPECT_TRUE(context.validateSequenceId(testConnection, 100).isOk());
    EXPECT_TRUE(context.validateSequenceId(testConnection, 101).isOk());
    EXPECT_TRUE(context.validateSequenceId(testConnection, 102).isOk());
}

/**
 * @brief Test out-of-order within window
 */
TEST_F(SecurityContextTest, OutOfOrderWithinWindowAccepted) {
    context.validateSequenceId(testConnection, 100);
    context.validateSequenceId(testConnection, 102);
    
    // Packet 101 arrives late but within window
    EXPECT_TRUE(context.validateSequenceId(testConnection, 101).isOk());
}

/**
 * @brief Test stale packet rejection (outside window)
 */
TEST_F(SecurityContextTest, StalePacketRejected) {
    context.validateSequenceId(testConnection, 2000);
    
    // Packet far in the past (outside anti-replay window)
    auto result = context.validateSequenceId(testConnection, 500);
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::InvalidSequence);
}

/**
 * @brief Test sequence ID wraparound at 65535
 */
TEST_F(SecurityContextTest, SequenceIdWraparound) {
    context.validateSequenceId(testConnection, 65534);
    context.validateSequenceId(testConnection, 65535);
    
    // Wraparound to 0
    EXPECT_TRUE(context.validateSequenceId(testConnection, 0).isOk());
    EXPECT_TRUE(context.validateSequenceId(testConnection, 1).isOk());
}

/**
 * @brief Test UserID mapping registration
 */
TEST_F(SecurityContextTest, UserIdMapping) {
    context.registerConnection(testConnection, 42);
    
    // Valid UserID
    EXPECT_TRUE(context.validateUserIdMapping(testConnection, 42).isOk());
    
    // Spoofed UserID
    auto result = context.validateUserIdMapping(testConnection, 99);
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::InvalidUserId);
}

/**
 * @brief Test connection cleanup
 */
TEST_F(SecurityContextTest, ConnectionCleanup) {
    context.registerConnection(testConnection, 42);
    EXPECT_EQ(context.getConnectionCount(), 1);
    
    context.removeConnection(testConnection);
    EXPECT_EQ(context.getConnectionCount(), 0);
}

/**
 * @brief Fuzz test with random data
 */
TEST(FuzzTest, RandomDataNoCrash) {
    std::vector<std::uint8_t> randomData;
    
    // Test various sizes of random data
    for (size_t size = 0; size < 200; ++size) {
        randomData.resize(size);
        for (auto& byte : randomData) {
            byte = static_cast<std::uint8_t>(rand() % 256);
        }
        
        // Should not crash, just return error
        auto result = Validator::validatePacket(randomData, false);
        // We don't care about the result, just that it doesn't crash
        (void)result;
    }
}

/**
 * @brief Fuzz test with malicious payloads
 */
TEST(FuzzTest, MaliciousPayloadsNoCrash) {
    // All 0xFF bytes
    std::vector<std::uint8_t> allFF(100, 0xFF);
    EXPECT_TRUE(Validator::validatePacket(allFF, false).isErr());
    
    // All 0x00 bytes
    std::vector<std::uint8_t> allZero(100, 0x00);
    EXPECT_TRUE(Validator::validatePacket(allZero, false).isErr());
    
    // Alternating pattern
    std::vector<std::uint8_t> alternating(100);
    for (size_t i = 0; i < alternating.size(); ++i) {
        alternating[i] = (i % 2) ? 0xFF : 0x00;
    }
    EXPECT_TRUE(Validator::validatePacket(alternating, false).isErr());
}

/**
 * @brief Test buffer overflow protection
 */
TEST(SecurityTest, BufferOverflowProtection) {
    // Create packet claiming huge payload
    Header header{};
    header.magic = kMagicByte;
    header.opcode = static_cast<std::uint8_t>(OpCode::C_INPUT);
    header.payloadSize = 65535;  // Claim massive payload
    header.userId = 42;
    header.seqId = 1;
    header.ackId = 0;
    header.flags = 0;
    header.reserved = {0, 0, 0};
    
    auto bytes = ByteOrderSpec::serializeToNetwork(header);
    // But only include small payload
    bytes.insert(bytes.end(), 10, 0x00);
    
    // Should be rejected (payload too large + size mismatch)
    EXPECT_TRUE(Validator::validatePacket(bytes, false).isErr());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
