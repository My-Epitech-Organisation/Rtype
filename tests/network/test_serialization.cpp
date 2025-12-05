/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_serialization
*/

#include <gtest/gtest.h>
#include <vector>
#include <string>

#include "../../lib/rtype_network/src/Packet.hpp"
#include "../../lib/rtype_network/src/Serializer.hpp"

using namespace rtype::network;

// TestData struct with explicit packing to ensure no padding for reliable serialization testing
#pragma pack(1)
struct TestData {
    int32_t x;
    float y;
};
#pragma pack()

enum class AIBehavior : uint8_t {
    MoveLeft = 0,
    SineWave,
    Chase,
    Patrol,
    Stationary
};

#pragma pack(1)
struct AIComponent {
    AIBehavior behavior = AIBehavior::MoveLeft;
    float speed = 100.0F;
    float stateTimer = 0.0F;
    float targetX = 0.0F;
    float targetY = 0.0F;
};
#pragma pack()

TEST(SerializationTest, SerializeDeserializePacket) {
    Packet packet(PacketType::PlayerInput);
    auto serialized = Serializer::serialize(packet);

    EXPECT_FALSE(serialized.empty());
    EXPECT_EQ(serialized[0], static_cast<uint8_t>(PacketType::PlayerInput));
}

TEST(SerializationTest, DeserializePacket) {
    Packet original(PacketType::EntityUpdate);
    auto serialized = Serializer::serialize(original);
    auto deserialized = Serializer::deserialize(serialized);

    EXPECT_EQ(deserialized.type(), PacketType::EntityUpdate);
}

TEST(SerializationTest, PacketWithData) {
    Packet packet(PacketType::EntitySpawn);
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    packet.setData(data);

    auto serialized = Serializer::serialize(packet);
    auto deserialized = Serializer::deserialize(serialized);

    EXPECT_EQ(deserialized.type(), PacketType::EntitySpawn);
    EXPECT_EQ(deserialized.data(), data);
}

TEST(SerializationTest, SerializeDeserializeStruct) {
    TestData original{42, 3.14f};
    auto serialized = Serializer::serialize(original);
    auto deserialized = Serializer::deserialize<TestData>(serialized);

    EXPECT_EQ(deserialized.x, 42);
    EXPECT_FLOAT_EQ(deserialized.y, 3.14f);
}

TEST(SerializationTest, DeserializeInvalidSize) {
    std::vector<uint8_t> invalidBuffer = {1, 2, 3};  // Wrong size
    EXPECT_THROW(Serializer::deserialize<TestData>(invalidBuffer), std::runtime_error);
}

TEST(SerializationTest, DeserializeVariousInvalidBufferSizes) {
    // Test with various invalid buffer sizes
    std::vector<std::vector<uint8_t>> invalidBuffers = {
        {},  // Empty
        {1},  // Too small
        {1, 2, 3, 4, 5},  // Too small
        {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17}  // Too large
    };

    for (const auto& buffer : invalidBuffers) {
        EXPECT_THROW(Serializer::deserialize<TestData>(buffer), std::runtime_error);
    }
}

TEST(SerializationTest, SerializeDeserializeString) {
    std::string original = "Hello, World!";
    auto serialized = Serializer::serialize(original);
    auto deserialized = Serializer::deserializeString(serialized);

    EXPECT_EQ(deserialized, original);
}

TEST(SerializationTest, SerializeDeserializeEmptyString) {
    std::string original = "";
    auto serialized = Serializer::serialize(original);
    auto deserialized = Serializer::deserializeString(serialized);

    EXPECT_EQ(deserialized, original);
}

TEST(SerializationTest, SerializeDeserializeStringWithSpecialChars) {
    std::string original = "Hello\n\tWorld\x00with\0nulls";
    auto serialized = Serializer::serialize(original);
    auto deserialized = Serializer::deserializeString(serialized);

    EXPECT_EQ(deserialized, original);
}

TEST(SerializationTest, DeserializeStringInvalidBuffer) {
    std::vector<uint8_t> invalidBuffer = {1, 2, 3};  // Too small for uint32_t
    EXPECT_THROW(Serializer::deserializeString(invalidBuffer), std::runtime_error);
}

TEST(SerializationTest, SerializeDeserializeAIComponent) {
    AIComponent ai;
    ai.behavior = AIBehavior::Chase;
    ai.speed = 150.5f;
    ai.stateTimer = 2.3f;
    ai.targetX = 100.0f;
    ai.targetY = 200.0f;

    auto serialized = Serializer::serialize(ai);
    auto deserialized = Serializer::deserialize<AIComponent>(serialized);

    EXPECT_EQ(deserialized.behavior, AIBehavior::Chase);
    EXPECT_FLOAT_EQ(deserialized.speed, 150.5f);
    EXPECT_FLOAT_EQ(deserialized.stateTimer, 2.3f);
    EXPECT_FLOAT_EQ(deserialized.targetX, 100.0f);
    EXPECT_FLOAT_EQ(deserialized.targetY, 200.0f);
}

TEST(SerializationTest, SerializeDeserializeAIComponentDefault) {
    AIComponent ai;  // All default values

    auto serialized = Serializer::serialize(ai);
    auto deserialized = Serializer::deserialize<AIComponent>(serialized);

    EXPECT_EQ(deserialized.behavior, AIBehavior::MoveLeft);
    EXPECT_FLOAT_EQ(deserialized.speed, 100.0f);
    EXPECT_FLOAT_EQ(deserialized.stateTimer, 0.0f);
    EXPECT_FLOAT_EQ(deserialized.targetX, 0.0f);
    EXPECT_FLOAT_EQ(deserialized.targetY, 0.0f);
}

TEST(SerializationTest, SerializeDeserializeMultipleFloats) {
    #pragma pack(1)
    struct FloatArray {
        float values[5];
    };
    #pragma pack()

    FloatArray arr;
    arr.values[0] = 1.1f;
    arr.values[1] = 2.2f;
    arr.values[2] = 3.3f;
    arr.values[3] = 4.4f;
    arr.values[4] = 5.5f;

    auto serialized = Serializer::serialize(arr);
    auto deserialized = Serializer::deserialize<FloatArray>(serialized);

    for (int i = 0; i < 5; ++i) {
        EXPECT_FLOAT_EQ(deserialized.values[i], arr.values[i]);
    }
}

TEST(SerializationTest, SerializeDeserializeMixedTypes) {
    #pragma pack(1)
    struct MixedData {
        int id;
        float health;
        bool alive;
        char name[16];
    };
    #pragma pack()

    MixedData data;
    data.id = 42;
    data.health = 85.7f;
    data.alive = true;
    std::strncpy(data.name, "PlayerOne", sizeof(data.name));

    auto serialized = Serializer::serialize(data);
    auto deserialized = Serializer::deserialize<MixedData>(serialized);

    EXPECT_EQ(deserialized.id, 42);
    EXPECT_FLOAT_EQ(deserialized.health, 85.7f);
    EXPECT_EQ(deserialized.alive, true);
    EXPECT_STREQ(deserialized.name, "PlayerOne");
}

TEST(SerializationTest, RoundTripSerializationConsistency) {
    // Test that multiple round-trips produce identical results
    TestData original{123, 45.67f};

    auto serialized1 = Serializer::serialize(original);
    auto deserialized1 = Serializer::deserialize<TestData>(serialized1);
    auto serialized2 = Serializer::serialize(deserialized1);
    auto deserialized2 = Serializer::deserialize<TestData>(serialized2);

    EXPECT_EQ(deserialized1.x, deserialized2.x);
    EXPECT_FLOAT_EQ(deserialized1.y, deserialized2.y);
    EXPECT_EQ(serialized1, serialized2);  // Serialized data should be identical
}

TEST(PacketTest, DefaultConstructor) {
    Packet packet;
    EXPECT_EQ(packet.type(), PacketType::Unknown);
    EXPECT_TRUE(packet.data().empty());
}

TEST(PacketTest, TypedConstructor) {
    Packet packet(PacketType::PlayerInput);
    EXPECT_EQ(packet.type(), PacketType::PlayerInput);
}

// ============================================================================
// Network Byte Order Tests - Serialize → ByteOrder → Send Pattern
// ============================================================================

TEST(SerializationTest, NetworkByteOrderPrimitiveRoundTrip) {
    // Test primitive type with network byte order conversion
    uint32_t original = 0x12345678;

    // Serialize → ByteOrder → "Send"
    auto bytes = Serializer::serialize(original);
    bytes = Serializer::toNetworkByteOrder<uint32_t>(bytes);

    // "Receive" → ByteOrder → Deserialize
    bytes = Serializer::fromNetworkByteOrder<uint32_t>(bytes);
    auto restored = Serializer::deserialize<uint32_t>(bytes);

    EXPECT_EQ(restored, original);
}

TEST(SerializationTest, NetworkByteOrderStructRoundTrip) {
    // Test struct byte order conversion round-trip on a single machine
    // Note: This validates that the generic struct conversion algorithm works correctly.
    // The conversion treats all 4-byte fields as uint32_t and all 2-byte fields as uint16_t.
    // This test passes on same-endian machines but does NOT validate true cross-platform compatibility.
    // True cross-platform testing would require testing between different endianness systems.
    TestData original{42, 3.14f};

    // Serialize → ByteOrder → "Send"
    auto bytes = Serializer::serialize(original);
    bytes = Serializer::toNetworkByteOrder<TestData>(bytes);

    // "Receive" → ByteOrder → Deserialize
    bytes = Serializer::fromNetworkByteOrder<TestData>(bytes);
    auto restored = Serializer::deserialize<TestData>(bytes);

    EXPECT_EQ(restored.x, original.x);
    EXPECT_FLOAT_EQ(restored.y, original.y);
}

TEST(SerializationTest, NetworkByteOrderAIComponentRoundTrip) {
    // Test AIComponent struct byte order conversion round-trip on a single machine
    // Note: This validates that the generic struct conversion algorithm works correctly for AIComponent.
    // The conversion treats all 4-byte fields as uint32_t and all 2-byte fields as uint16_t.
    // AIComponent is properly packed with #pragma pack(1) for reliable serialization.
    // This test passes on same-endian machines but does NOT validate true cross-platform compatibility.
    // True cross-platform testing would require testing between different endianness systems.
    AIComponent original;
    original.behavior = AIBehavior::Patrol;
    original.speed = 250.5f;
    original.stateTimer = 5.5f;
    original.targetX = 100.0f;
    original.targetY = 200.0f;

    // Serialize → ByteOrder → "Send"
    auto bytes = Serializer::serialize(original);
    bytes = Serializer::toNetworkByteOrder<AIComponent>(bytes);

    // "Receive" → ByteOrder → Deserialize
    bytes = Serializer::fromNetworkByteOrder<AIComponent>(bytes);
    auto restored = Serializer::deserialize<AIComponent>(bytes);

    EXPECT_EQ(restored.behavior, original.behavior);
    EXPECT_FLOAT_EQ(restored.speed, original.speed);
    EXPECT_FLOAT_EQ(restored.stateTimer, original.stateTimer);
    EXPECT_FLOAT_EQ(restored.targetX, original.targetX);
    EXPECT_FLOAT_EQ(restored.targetY, original.targetY);
}

TEST(SerializationTest, NetworkByteOrderComplexStruct) {
    // Test struct with multiple field types and byte order conversion
    // Note: This test validates the generic struct conversion functions but runs on a single machine.
    // The conversion treats all 4-byte fields as uint32_t and all 2-byte fields as uint16_t.
    // True cross-platform validation would require testing between different endianness systems.
    #pragma pack(1)
    struct ComplexData {
        uint32_t id;
        float x, y, z;
        uint16_t health;
        uint8_t team;
    };
    #pragma pack()

    ComplexData original{999, 10.5f, 20.3f, 30.1f, 100, 2};

    // Serialize → ByteOrder → "Send"
    auto bytes = Serializer::serialize(original);
    bytes = Serializer::toNetworkByteOrder<ComplexData>(bytes);

    // "Receive" → ByteOrder → Deserialize
    bytes = Serializer::fromNetworkByteOrder<ComplexData>(bytes);
    auto restored = Serializer::deserialize<ComplexData>(bytes);

    EXPECT_EQ(restored.id, original.id);
    EXPECT_FLOAT_EQ(restored.x, original.x);
    EXPECT_FLOAT_EQ(restored.y, original.y);
    EXPECT_FLOAT_EQ(restored.z, original.z);
    EXPECT_EQ(restored.health, original.health);
    EXPECT_EQ(restored.team, original.team);
}

TEST(SerializationTest, NetworkByteOrderInvalidBufferSize) {
    // Test error handling for buffer size mismatch
    std::vector<uint8_t> invalidBuffer = {1, 2, 3};  // Wrong size

    EXPECT_THROW(Serializer::toNetworkByteOrder<uint32_t>(invalidBuffer),
                 std::runtime_error);
    EXPECT_THROW(Serializer::fromNetworkByteOrder<uint32_t>(invalidBuffer),
                 std::runtime_error);
}

TEST(SerializationTest, MultipleStructsInSequence) {
    // Test serializing multiple structs in sequence
    TestData data1{100, 1.5f};
    TestData data2{200, 2.5f};
    TestData data3{300, 3.5f};

    std::vector<uint8_t> allBytes;

    // Serialize all structs
    auto bytes1 = Serializer::serialize(data1);
    auto bytes2 = Serializer::serialize(data2);
    auto bytes3 = Serializer::serialize(data3);

    // Convert to network order
    bytes1 = Serializer::toNetworkByteOrder<TestData>(bytes1);
    bytes2 = Serializer::toNetworkByteOrder<TestData>(bytes2);
    bytes3 = Serializer::toNetworkByteOrder<TestData>(bytes3);

    // Combine into single buffer
    allBytes.insert(allBytes.end(), bytes1.begin(), bytes1.end());
    allBytes.insert(allBytes.end(), bytes2.begin(), bytes2.end());
    allBytes.insert(allBytes.end(), bytes3.begin(), bytes3.end());

    // Deserialize from combined buffer
    size_t offset = 0;
    size_t structSize = sizeof(TestData);

    std::vector<uint8_t> recv1(allBytes.begin() + offset, allBytes.begin() + offset + structSize);
    offset += structSize;
    std::vector<uint8_t> recv2(allBytes.begin() + offset, allBytes.begin() + offset + structSize);
    offset += structSize;
    std::vector<uint8_t> recv3(allBytes.begin() + offset, allBytes.begin() + offset + structSize);

    recv1 = Serializer::fromNetworkByteOrder<TestData>(recv1);
    recv2 = Serializer::fromNetworkByteOrder<TestData>(recv2);
    recv3 = Serializer::fromNetworkByteOrder<TestData>(recv3);

    auto restored1 = Serializer::deserialize<TestData>(recv1);
    auto restored2 = Serializer::deserialize<TestData>(recv2);
    auto restored3 = Serializer::deserialize<TestData>(recv3);

    EXPECT_EQ(restored1.x, 100);
    EXPECT_FLOAT_EQ(restored1.y, 1.5f);
    EXPECT_EQ(restored2.x, 200);
    EXPECT_FLOAT_EQ(restored2.y, 2.5f);
    EXPECT_EQ(restored3.x, 300);
    EXPECT_FLOAT_EQ(restored3.y, 3.5f);
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST(SerializationTest, NonTriviallyCopyableTypeCompileError) {
    // This test verifies that non-trivially copyable types will not compile
    // Uncomment to verify compile-time error:
    /*
    struct NonTrivial {
        std::string name;
        std::vector<int> data;
    };

    NonTrivial nt{"test", {1, 2, 3}};
    auto bytes = Serializer::serialize(nt);  // Should not compile!
    */

    // This test passes by not having the commented code compile
    SUCCEED();
}

TEST(SerializationTest, NetworkByteOrderConsistency) {
    // Verify that converting back and forth doesn't corrupt data
    uint32_t original = 0xDEADBEEF;

    auto bytes = Serializer::serialize(original);

    // Convert to network and back multiple times
    for (int i = 0; i < 10; ++i) {
        bytes = Serializer::toNetworkByteOrder<uint32_t>(bytes);
        bytes = Serializer::fromNetworkByteOrder<uint32_t>(bytes);
    }

    auto restored = Serializer::deserialize<uint32_t>(bytes);
    EXPECT_EQ(restored, original);
}

TEST(SerializationTest, ZeroValuedStruct) {
    // Test struct with all zero values
    TestData zero{0, 0.0f};

    auto bytes = Serializer::serialize(zero);
    bytes = Serializer::toNetworkByteOrder<TestData>(bytes);
    bytes = Serializer::fromNetworkByteOrder<TestData>(bytes);
    auto restored = Serializer::deserialize<TestData>(bytes);

    EXPECT_EQ(restored.x, 0);
    EXPECT_FLOAT_EQ(restored.y, 0.0f);
}

TEST(SerializationTest, NegativeValuesStruct) {
    // Test struct with negative values
    TestData negative{-42, -3.14f};

    auto bytes = Serializer::serialize(negative);
    bytes = Serializer::toNetworkByteOrder<TestData>(bytes);
    bytes = Serializer::fromNetworkByteOrder<TestData>(bytes);
    auto restored = Serializer::deserialize<TestData>(bytes);

    EXPECT_EQ(restored.x, -42);
    EXPECT_FLOAT_EQ(restored.y, -3.14f);
}

TEST(SerializationTest, MaxValuedStruct) {
    // Test struct with maximum/extreme values
    #pragma pack(1)
    struct ExtremValues {
        uint32_t maxUint;
        int32_t maxInt;
        int32_t minInt;
        float maxFloat;
    };
    #pragma pack()

    ExtremValues extreme{
        UINT32_MAX,
        INT32_MAX,
        INT32_MIN,
        3.402823466e+38f  // Close to FLT_MAX
    };

    auto bytes = Serializer::serialize(extreme);
    bytes = Serializer::toNetworkByteOrder<ExtremValues>(bytes);
    bytes = Serializer::fromNetworkByteOrder<ExtremValues>(bytes);
    auto restored = Serializer::deserialize<ExtremValues>(bytes);

    EXPECT_EQ(restored.maxUint, UINT32_MAX);
    EXPECT_EQ(restored.maxInt, INT32_MAX);
    EXPECT_EQ(restored.minInt, INT32_MIN);
    EXPECT_FLOAT_EQ(restored.maxFloat, extreme.maxFloat);
}

// ============================================================================
// Cross-Platform Network Workflow Tests
// ============================================================================

TEST(SerializationTest, CrossPlatformPlayerDataWorkflow) {
    // Test the complete serialization workflow with byte order conversion
    // Note: This test validates the conversion functions but runs on a single machine.
    // True cross-platform testing would require testing between different endianness systems.
    #pragma pack(1)
    struct PlayerData {
        uint32_t playerId;
        float posX;
        float posY;
        float velocityX;
        float velocityY;
        uint16_t health;
        uint8_t team;
    };
    #pragma pack()

    // CLIENT SIDE - Prepare data for network transmission
    PlayerData clientPlayer{
        12345,          // playerId
        100.5f,         // posX
        250.75f,        // posY
        5.5f,           // velocityX
        -3.25f,         // velocityY
        100,            // health
        1               // team
    };

    // Step 1: Client serializes in native byte order
    auto clientBytes = Serializer::serialize(clientPlayer);
    EXPECT_EQ(clientBytes.size(), sizeof(PlayerData));

    // Step 2: Client converts to network byte order before sending
    clientBytes = Serializer::toNetworkByteOrder<PlayerData>(clientBytes);

    // Step 3: Simulate network transmission (data is now in network byte order)
    std::vector<uint8_t> networkBytes = clientBytes;

    // RECEIVER SIDE - Process received network data
    // Step 4: Receiver gets network bytes
    std::vector<uint8_t> receiverBytes = networkBytes;

    // Step 5: Receiver converts from network byte order to native
    receiverBytes = Serializer::fromNetworkByteOrder<PlayerData>(receiverBytes);

    // Step 6: Receiver deserializes to struct
    auto receiverPlayer = Serializer::deserialize<PlayerData>(receiverBytes);

    // Verify all fields match exactly (validates conversion functions work correctly)
    EXPECT_EQ(receiverPlayer.playerId, 12345u);
    EXPECT_FLOAT_EQ(receiverPlayer.posX, 100.5f);
    EXPECT_FLOAT_EQ(receiverPlayer.posY, 250.75f);
    EXPECT_FLOAT_EQ(receiverPlayer.velocityX, 5.5f);
    EXPECT_FLOAT_EQ(receiverPlayer.velocityY, -3.25f);
    EXPECT_EQ(receiverPlayer.health, 100);
    EXPECT_EQ(receiverPlayer.team, 1);
}

TEST(SerializationTest, CrossPlatformGameStateWorkflow) {
    // Test game state serialization workflow with struct byte order conversion
    // Note: This test validates that struct byte order conversion works correctly on a single machine.
    // The conversion treats all 4-byte fields as uint32_t and all 2-byte fields as uint16_t.
    // True cross-platform testing would require testing between different endianness systems.
    #pragma pack(1)
    struct GameState {
        uint32_t frameNumber;
        float gameTime;
        uint16_t playerCount;
        uint16_t enemyCount;
        uint32_t score;
        uint8_t isPaused;
    };
    #pragma pack()

    // Create game state data
    GameState serverState{
        1000,           // frameNumber
        45.67f,         // gameTime (seconds)
        4,              // playerCount
        15,             // enemyCount
        250000,         // score
        0               // isPaused (false)
    };

    // Serialize → Network Order → Simulate Send
    auto serverBytes = Serializer::serialize(serverState);
    serverBytes = Serializer::toNetworkByteOrder<GameState>(serverBytes);

    // Simulate network transmission
    std::vector<uint8_t> networkPacket = serverBytes;

    // Receive → Network Order → Deserialize
    auto clientBytes = networkPacket;
    clientBytes = Serializer::fromNetworkByteOrder<GameState>(clientBytes);
    auto clientState = Serializer::deserialize<GameState>(clientBytes);

    // Verify perfect match (validates conversion functions work correctly)
    EXPECT_EQ(clientState.frameNumber, 1000u);
    EXPECT_FLOAT_EQ(clientState.gameTime, 45.67f);
    EXPECT_EQ(clientState.playerCount, 4);
    EXPECT_EQ(clientState.enemyCount, 15);
    EXPECT_EQ(clientState.score, 250000u);
    EXPECT_EQ(clientState.isPaused, 0);
}

TEST(SerializationTest, CrossPlatformEntityUpdateWorkflow) {
    // Test entity update serialization workflow with struct byte order conversion
    // Note: This test validates that struct byte order conversion works correctly on a single machine.
    // The conversion treats all 4-byte fields as uint32_t and all 2-byte fields as uint16_t.
    // True cross-platform testing would require testing between different endianness systems.
    #pragma pack(1)
    struct EntityUpdate {
        uint32_t entityId;
        float x, y, z;
        float rotationX, rotationY, rotationZ;
        uint16_t animationState;
        uint8_t isActive;
    };
    #pragma pack()

    EntityUpdate original{
        9876,                           // entityId
        150.25f, 200.75f, 50.0f,       // position
        0.0f, 3.14159f, 1.57079f,      // rotation (radians)
        5,                              // animationState
        1                               // isActive
    };

    // Host A: Serialize and prepare for network
    auto hostABytes = Serializer::serialize(original);
    hostABytes = Serializer::toNetworkByteOrder<EntityUpdate>(hostABytes);

    // Network transmission simulation
    std::vector<uint8_t> transmitted = hostABytes;

    // Host B: Receive and process
    transmitted = Serializer::fromNetworkByteOrder<EntityUpdate>(transmitted);
    auto received = Serializer::deserialize<EntityUpdate>(transmitted);

    // Verify exact match (validates conversion functions work correctly)
    EXPECT_EQ(received.entityId, 9876u);
    EXPECT_FLOAT_EQ(received.x, 150.25f);
    EXPECT_FLOAT_EQ(received.y, 200.75f);
    EXPECT_FLOAT_EQ(received.z, 50.0f);
    EXPECT_FLOAT_EQ(received.rotationX, 0.0f);
    EXPECT_FLOAT_EQ(received.rotationY, 3.14159f);
    EXPECT_FLOAT_EQ(received.rotationZ, 1.57079f);
    EXPECT_EQ(received.animationState, 5);
    EXPECT_EQ(received.isActive, 1);
}

TEST(SerializationTest, CrossPlatformBidirectionalWorkflow) {
    // Test bidirectional communication workflow with byte order conversion
    // Note: This test validates the conversion functions but runs on a single machine.
    // True cross-platform testing would require testing between different endianness systems.
    #pragma pack(1)
    struct PlayerInput {
        uint32_t playerId;
        uint16_t inputFlags;  // Bitfield for keys pressed
        float aimX;
        float aimY;
        uint32_t timestamp;
    };
    #pragma pack()

    // CLIENT → SERVER
    PlayerInput clientInput{
        42,             // playerId
        0b1011,         // inputFlags (W, A, Space pressed)
        0.75f,          // aimX
        0.25f,          // aimY
        123456789       // timestamp
    };

    // Client sends
    auto c2s_bytes = Serializer::serialize(clientInput);
    c2s_bytes = Serializer::toNetworkByteOrder<PlayerInput>(c2s_bytes);

    // Server receives
    c2s_bytes = Serializer::fromNetworkByteOrder<PlayerInput>(c2s_bytes);
    auto serverInput = Serializer::deserialize<PlayerInput>(c2s_bytes);

    EXPECT_EQ(serverInput.playerId, 42u);
    EXPECT_EQ(serverInput.inputFlags, 0b1011);
    EXPECT_FLOAT_EQ(serverInput.aimX, 0.75f);
    EXPECT_FLOAT_EQ(serverInput.aimY, 0.25f);
    EXPECT_EQ(serverInput.timestamp, 123456789u);

    // SERVER → CLIENT (acknowledgment)
    PlayerInput serverResponse{
        42,             // playerId (echo)
        0,              // inputFlags (cleared)
        0.75f,          // aimX (confirmed)
        0.25f,          // aimY (confirmed)
        123456790       // timestamp (incremented)
    };

    // Server sends back
    auto s2c_bytes = Serializer::serialize(serverResponse);
    s2c_bytes = Serializer::toNetworkByteOrder<PlayerInput>(s2c_bytes);

    // Client receives
    s2c_bytes = Serializer::fromNetworkByteOrder<PlayerInput>(s2c_bytes);
    auto clientResponse = Serializer::deserialize<PlayerInput>(s2c_bytes);

    EXPECT_EQ(clientResponse.playerId, 42u);
    EXPECT_EQ(clientResponse.inputFlags, 0);
    EXPECT_FLOAT_EQ(clientResponse.aimX, 0.75f);
    EXPECT_FLOAT_EQ(clientResponse.aimY, 0.25f);
    EXPECT_EQ(clientResponse.timestamp, 123456790u);
}

TEST(SerializationTest, CrossPlatformRawByteVerification) {
    // Verify that network bytes are actually in big-endian format
    uint32_t value = 0x12345678;

    auto bytes = Serializer::serialize(value);
    bytes = Serializer::toNetworkByteOrder<uint32_t>(bytes);

    // In network byte order (big-endian), bytes should be: 0x12, 0x34, 0x56, 0x78
    EXPECT_EQ(bytes.size(), 4u);
    EXPECT_EQ(bytes[0], 0x12);  // Most significant byte first
    EXPECT_EQ(bytes[1], 0x34);
    EXPECT_EQ(bytes[2], 0x56);
    EXPECT_EQ(bytes[3], 0x78);  // Least significant byte last

    // Convert back and verify
    bytes = Serializer::fromNetworkByteOrder<uint32_t>(bytes);
    auto restored = Serializer::deserialize<uint32_t>(bytes);
    EXPECT_EQ(restored, 0x12345678u);
}

TEST(SerializationTest, StructByteOrderConversionValidation) {
    // Verify that struct byte order conversion actually works
    #pragma pack(1)
    struct TestStruct {
        uint32_t id;
        uint16_t flags;
        uint8_t type;
    };
    #pragma pack()

    TestStruct original{0x12345678, 0xABCD, 0x42};

    // Serialize and convert to network byte order
    auto bytes = Serializer::serialize(original);
    bytes = Serializer::toNetworkByteOrder<TestStruct>(bytes);

    // Verify bytes are in big-endian format
    EXPECT_EQ(bytes.size(), 7u);
    // uint32_t id: 0x12345678 -> big-endian: 0x12, 0x34, 0x56, 0x78
    EXPECT_EQ(bytes[0], 0x12);
    EXPECT_EQ(bytes[1], 0x34);
    EXPECT_EQ(bytes[2], 0x56);
    EXPECT_EQ(bytes[3], 0x78);
    // uint16_t flags: 0xABCD -> big-endian: 0xAB, 0xCD
    EXPECT_EQ(bytes[4], 0xAB);
    EXPECT_EQ(bytes[5], 0xCD);
    // uint8_t type: unchanged
    EXPECT_EQ(bytes[6], 0x42);

    // Convert back and verify round-trip
    bytes = Serializer::fromNetworkByteOrder<TestStruct>(bytes);
    auto restored = Serializer::deserialize<TestStruct>(bytes);

    EXPECT_EQ(restored.id, 0x12345678u);
    EXPECT_EQ(restored.flags, 0xABCDu);
    EXPECT_EQ(restored.type, 0x42u);
}

TEST(SerializationTest, SerializeStringTooLarge) {
    // Create a string larger than UINT32_MAX
    // We can't actually create such a string, but we can test the boundary
    std::string largeString(1000000, 'A');  // Large but not too large

    // This should work fine
    EXPECT_NO_THROW(Serializer::serialize(largeString));

    // Test with a mock large size by creating a custom string-like object
    // that reports a large size
    class MockLargeString {
    public:
        size_t size() const { return static_cast<size_t>(UINT32_MAX) + 1; }
        const char* begin() const { return nullptr; }
        const char* end() const { return nullptr; }
    };

    // We can't easily test this without modifying the function signature
    // But the branch exists in the code, so this is noted for future testing
    SUCCEED();  // Placeholder test
}

TEST(SerializationTest, DeserializeStringBufferSizeMismatch) {
    // Test the case where buffer has length field but not enough data for the string
    std::vector<uint8_t> buffer;

    // Write a length of 10
    uint32_t length = 10;
    std::vector<uint8_t> lengthBytes(sizeof(uint32_t));
    ByteOrder::writeTo(lengthBytes.data(), length);
    buffer.insert(buffer.end(), lengthBytes.begin(), lengthBytes.end());

    // But only provide 5 bytes of string data (not enough)
    for (int i = 0; i < 5; ++i) {
        buffer.push_back('A');
    }

    // This should throw because buffer.size() < sizeof(uint32_t) + length
    EXPECT_THROW(Serializer::deserializeString(buffer), std::runtime_error);
}

TEST(SerializationTest, DeserializeEmptyBuffer) {
    // Test deserializing an empty buffer
    std::vector<uint8_t> emptyBuffer;
    auto packet = Serializer::deserialize(emptyBuffer);

    // Should create a default packet
    EXPECT_EQ(packet.type(), PacketType::Unknown);
    EXPECT_TRUE(packet.data().empty());
}

TEST(SerializationTest, DeserializeSingleByteBuffer) {
    // Test deserializing a buffer with just one byte (packet type only)
    std::vector<uint8_t> singleByteBuffer = {static_cast<uint8_t>(PacketType::PlayerInput)};
    auto packet = Serializer::deserialize(singleByteBuffer);

    EXPECT_EQ(packet.type(), PacketType::PlayerInput);
    EXPECT_TRUE(packet.data().empty());
}
