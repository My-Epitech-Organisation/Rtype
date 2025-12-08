/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** BinarySerializer Branch Coverage Tests
*/

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "Config/SaveManager/Serialization/BinarySerializer.hpp"

namespace rtype::game::config {

class BinarySerializerTest : public ::testing::Test {
   protected:
    std::shared_ptr<std::vector<uint8_t>> buffer =
        std::make_shared<std::vector<uint8_t>>();
    std::shared_ptr<size_t> offset = std::make_shared<size_t>(0);

    std::shared_ptr<const std::vector<uint8_t>> getConstBuffer() const {
        return std::make_shared<const std::vector<uint8_t>>(*buffer);
    }

    void resetOffset() { *offset = 0; }

    void clearBuffer() {
        buffer->clear();
        resetOffset();
    }
};

// =============================================================================
// Write Tests
// =============================================================================

TEST_F(BinarySerializerTest, WriteUint8) {
    BinarySerializer::writeUint8(buffer, 0x00);
    BinarySerializer::writeUint8(buffer, 0xFF);
    BinarySerializer::writeUint8(buffer, 0x7F);

    ASSERT_EQ(buffer->size(), 3);
    EXPECT_EQ((*buffer)[0], 0x00);
    EXPECT_EQ((*buffer)[1], 0xFF);
    EXPECT_EQ((*buffer)[2], 0x7F);
}

TEST_F(BinarySerializerTest, WriteUint16) {
    BinarySerializer::writeUint16(buffer, 0x0000);
    BinarySerializer::writeUint16(buffer, 0xFFFF);
    BinarySerializer::writeUint16(buffer, 0x1234);

    ASSERT_EQ(buffer->size(), 6);
    // Little endian: 0x1234 -> 0x34, 0x12
    EXPECT_EQ((*buffer)[4], 0x34);
    EXPECT_EQ((*buffer)[5], 0x12);
}

TEST_F(BinarySerializerTest, WriteUint32) {
    BinarySerializer::writeUint32(buffer, 0x00000000);
    BinarySerializer::writeUint32(buffer, 0xFFFFFFFF);
    BinarySerializer::writeUint32(buffer, 0x12345678);

    ASSERT_EQ(buffer->size(), 12);
    // Little endian: 0x12345678 -> 0x78, 0x56, 0x34, 0x12
    EXPECT_EQ((*buffer)[8], 0x78);
    EXPECT_EQ((*buffer)[9], 0x56);
    EXPECT_EQ((*buffer)[10], 0x34);
    EXPECT_EQ((*buffer)[11], 0x12);
}

TEST_F(BinarySerializerTest, WriteUint64) {
    BinarySerializer::writeUint64(buffer, 0x0000000000000000ULL);
    BinarySerializer::writeUint64(buffer, 0xFFFFFFFFFFFFFFFFULL);
    BinarySerializer::writeUint64(buffer, 0x123456789ABCDEF0ULL);

    ASSERT_EQ(buffer->size(), 24);
}

TEST_F(BinarySerializerTest, WriteInt32Positive) {
    BinarySerializer::writeInt32(buffer, 12345);

    ASSERT_EQ(buffer->size(), 4);
}

TEST_F(BinarySerializerTest, WriteInt32Negative) {
    BinarySerializer::writeInt32(buffer, -12345);

    ASSERT_EQ(buffer->size(), 4);
}

TEST_F(BinarySerializerTest, WriteInt32Zero) {
    BinarySerializer::writeInt32(buffer, 0);

    ASSERT_EQ(buffer->size(), 4);
    EXPECT_EQ((*buffer)[0], 0);
    EXPECT_EQ((*buffer)[1], 0);
    EXPECT_EQ((*buffer)[2], 0);
    EXPECT_EQ((*buffer)[3], 0);
}

TEST_F(BinarySerializerTest, WriteInt32MinMax) {
    BinarySerializer::writeInt32(buffer, std::numeric_limits<int32_t>::min());
    BinarySerializer::writeInt32(buffer, std::numeric_limits<int32_t>::max());

    ASSERT_EQ(buffer->size(), 8);
}

TEST_F(BinarySerializerTest, WriteFloatPositive) {
    BinarySerializer::writeFloat(buffer, 3.14159f);

    ASSERT_EQ(buffer->size(), 4);
}

TEST_F(BinarySerializerTest, WriteFloatNegative) {
    BinarySerializer::writeFloat(buffer, -3.14159f);

    ASSERT_EQ(buffer->size(), 4);
}

TEST_F(BinarySerializerTest, WriteFloatZero) {
    BinarySerializer::writeFloat(buffer, 0.0f);

    ASSERT_EQ(buffer->size(), 4);
}

TEST_F(BinarySerializerTest, WriteFloatSpecialValues) {
    BinarySerializer::writeFloat(buffer, std::numeric_limits<float>::min());
    BinarySerializer::writeFloat(buffer, std::numeric_limits<float>::max());
    BinarySerializer::writeFloat(buffer, std::numeric_limits<float>::epsilon());

    ASSERT_EQ(buffer->size(), 12);
}

TEST_F(BinarySerializerTest, WriteStringEmpty) {
    BinarySerializer::writeString(buffer, "");

    ASSERT_EQ(buffer->size(), 4);  // Just the length (0)
}

TEST_F(BinarySerializerTest, WriteStringNormal) {
    std::string testStr = "Hello, World!";
    BinarySerializer::writeString(buffer, testStr);

    ASSERT_EQ(buffer->size(), 4 + testStr.size());
}

TEST_F(BinarySerializerTest, WriteStringLong) {
    std::string testStr(1000, 'x');
    BinarySerializer::writeString(buffer, testStr);

    ASSERT_EQ(buffer->size(), 4 + 1000);
}

TEST_F(BinarySerializerTest, WriteStringWithSpecialChars) {
    std::string testStr = "Test\0with\nnull\tand\rspecial";
    BinarySerializer::writeString(buffer, testStr);

    EXPECT_GE(buffer->size(), 4);  // At least the length
}

// =============================================================================
// Read Tests - Success cases
// =============================================================================

TEST_F(BinarySerializerTest, ReadUint8Success) {
    *buffer = {0x00, 0xFF, 0x7F};
    auto constBuffer = getConstBuffer();

    EXPECT_EQ(BinarySerializer::readUint8(constBuffer, offset), 0x00);
    EXPECT_EQ(BinarySerializer::readUint8(constBuffer, offset), 0xFF);
    EXPECT_EQ(BinarySerializer::readUint8(constBuffer, offset), 0x7F);
    EXPECT_EQ(*offset, 3);
}

TEST_F(BinarySerializerTest, ReadUint16Success) {
    *buffer = {0x34, 0x12, 0xFF, 0xFF};
    auto constBuffer = getConstBuffer();

    EXPECT_EQ(BinarySerializer::readUint16(constBuffer, offset), 0x1234);
    EXPECT_EQ(BinarySerializer::readUint16(constBuffer, offset), 0xFFFF);
    EXPECT_EQ(*offset, 4);
}

TEST_F(BinarySerializerTest, ReadUint32Success) {
    *buffer = {0x78, 0x56, 0x34, 0x12, 0xFF, 0xFF, 0xFF, 0xFF};
    auto constBuffer = getConstBuffer();

    EXPECT_EQ(BinarySerializer::readUint32(constBuffer, offset), 0x12345678);
    EXPECT_EQ(BinarySerializer::readUint32(constBuffer, offset), 0xFFFFFFFF);
    EXPECT_EQ(*offset, 8);
}

TEST_F(BinarySerializerTest, ReadUint64Success) {
    *buffer = {0xF0, 0xDE, 0xBC, 0x9A, 0x78, 0x56, 0x34, 0x12};
    auto constBuffer = getConstBuffer();

    EXPECT_EQ(BinarySerializer::readUint64(constBuffer, offset),
              0x123456789ABCDEF0ULL);
    EXPECT_EQ(*offset, 8);
}

TEST_F(BinarySerializerTest, ReadInt32Success) {
    // Write then read
    BinarySerializer::writeInt32(buffer, 12345);
    BinarySerializer::writeInt32(buffer, -12345);
    auto constBuffer = getConstBuffer();

    int32_t val1 = BinarySerializer::readInt32(constBuffer, offset);
    int32_t val2 = BinarySerializer::readInt32(constBuffer, offset);

    EXPECT_EQ(val1, 12345);
    EXPECT_EQ(val2, -12345);
}

TEST_F(BinarySerializerTest, ReadFloatSuccess) {
    BinarySerializer::writeFloat(buffer, 3.14159f);
    BinarySerializer::writeFloat(buffer, -2.71828f);
    auto constBuffer = getConstBuffer();

    float val1 = BinarySerializer::readFloat(constBuffer, offset);
    float val2 = BinarySerializer::readFloat(constBuffer, offset);

    EXPECT_FLOAT_EQ(val1, 3.14159f);
    EXPECT_FLOAT_EQ(val2, -2.71828f);
}

TEST_F(BinarySerializerTest, ReadStringSuccess) {
    std::string original = "Test string!";
    BinarySerializer::writeString(buffer, original);
    auto constBuffer = getConstBuffer();

    std::string result = BinarySerializer::readString(constBuffer, offset);

    EXPECT_EQ(result, original);
}

TEST_F(BinarySerializerTest, ReadStringEmpty) {
    BinarySerializer::writeString(buffer, "");
    auto constBuffer = getConstBuffer();

    std::string result = BinarySerializer::readString(constBuffer, offset);

    EXPECT_EQ(result, "");
}

// =============================================================================
// Read Tests - Buffer overflow (exception branches)
// =============================================================================

TEST_F(BinarySerializerTest, ReadUint8BufferOverflow) {
    *buffer = {};  // Empty buffer
    auto constBuffer = getConstBuffer();

    EXPECT_THROW(BinarySerializer::readUint8(constBuffer, offset),
                 std::out_of_range);
}

TEST_F(BinarySerializerTest, ReadUint8BufferOverflowAtEnd) {
    *buffer = {0x01};
    *offset = 1;  // Already at end
    auto constBuffer = getConstBuffer();

    EXPECT_THROW(BinarySerializer::readUint8(constBuffer, offset),
                 std::out_of_range);
}

TEST_F(BinarySerializerTest, ReadUint16BufferOverflow) {
    *buffer = {0x01};  // Only 1 byte, need 2
    auto constBuffer = getConstBuffer();

    EXPECT_THROW(BinarySerializer::readUint16(constBuffer, offset),
                 std::out_of_range);
}

TEST_F(BinarySerializerTest, ReadUint16BufferOverflowPartial) {
    *buffer = {0x01, 0x02, 0x03};
    *offset = 2;  // Only 1 byte left, need 2
    auto constBuffer = getConstBuffer();

    EXPECT_THROW(BinarySerializer::readUint16(constBuffer, offset),
                 std::out_of_range);
}

TEST_F(BinarySerializerTest, ReadUint32BufferOverflow) {
    *buffer = {0x01, 0x02, 0x03};  // Only 3 bytes, need 4
    auto constBuffer = getConstBuffer();

    EXPECT_THROW(BinarySerializer::readUint32(constBuffer, offset),
                 std::out_of_range);
}

TEST_F(BinarySerializerTest, ReadUint32BufferOverflowEmpty) {
    *buffer = {};
    auto constBuffer = getConstBuffer();

    EXPECT_THROW(BinarySerializer::readUint32(constBuffer, offset),
                 std::out_of_range);
}

TEST_F(BinarySerializerTest, ReadUint64BufferOverflow) {
    *buffer = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};  // Only 7 bytes, need 8
    auto constBuffer = getConstBuffer();

    EXPECT_THROW(BinarySerializer::readUint64(constBuffer, offset),
                 std::out_of_range);
}

TEST_F(BinarySerializerTest, ReadUint64BufferOverflowEmpty) {
    *buffer = {};
    auto constBuffer = getConstBuffer();

    EXPECT_THROW(BinarySerializer::readUint64(constBuffer, offset),
                 std::out_of_range);
}

TEST_F(BinarySerializerTest, ReadStringBufferOverflowInLength) {
    *buffer = {0x01, 0x02};  // Only 2 bytes, need 4 for length
    auto constBuffer = getConstBuffer();

    EXPECT_THROW(BinarySerializer::readString(constBuffer, offset),
                 std::out_of_range);
}

TEST_F(BinarySerializerTest, ReadStringBufferOverflowInContent) {
    // Length says 100 bytes, but buffer is shorter
    *buffer = {100, 0, 0, 0, 'H', 'e', 'l', 'l', 'o'};  // Length=100, only 5 chars
    auto constBuffer = getConstBuffer();

    EXPECT_THROW(BinarySerializer::readString(constBuffer, offset),
                 std::out_of_range);
}

// =============================================================================
// Round-trip Tests
// =============================================================================

TEST_F(BinarySerializerTest, RoundTripUint8) {
    uint8_t values[] = {0, 1, 127, 128, 255};
    for (auto val : values) {
        clearBuffer();
        resetOffset();
        BinarySerializer::writeUint8(buffer, val);
        auto constBuffer = getConstBuffer();
        EXPECT_EQ(BinarySerializer::readUint8(constBuffer, offset), val);
    }
}

TEST_F(BinarySerializerTest, RoundTripUint16) {
    uint16_t values[] = {0, 1, 255, 256, 32767, 32768, 65535};
    for (auto val : values) {
        clearBuffer();
        resetOffset();
        BinarySerializer::writeUint16(buffer, val);
        auto constBuffer = getConstBuffer();
        EXPECT_EQ(BinarySerializer::readUint16(constBuffer, offset), val);
    }
}

TEST_F(BinarySerializerTest, RoundTripUint32) {
    uint32_t values[] = {0, 1, 255, 65535, 0x7FFFFFFF, 0xFFFFFFFF};
    for (auto val : values) {
        clearBuffer();
        resetOffset();
        BinarySerializer::writeUint32(buffer, val);
        auto constBuffer = getConstBuffer();
        EXPECT_EQ(BinarySerializer::readUint32(constBuffer, offset), val);
    }
}

TEST_F(BinarySerializerTest, RoundTripUint64) {
    uint64_t values[] = {0,          1,          0xFFFFFFFFULL,
                         0x7FFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL};
    for (auto val : values) {
        clearBuffer();
        resetOffset();
        BinarySerializer::writeUint64(buffer, val);
        auto constBuffer = getConstBuffer();
        EXPECT_EQ(BinarySerializer::readUint64(constBuffer, offset), val);
    }
}

TEST_F(BinarySerializerTest, RoundTripInt32) {
    int32_t values[] = {0, 1, -1, 100, -100, std::numeric_limits<int32_t>::min(),
                        std::numeric_limits<int32_t>::max()};
    for (auto val : values) {
        clearBuffer();
        resetOffset();
        BinarySerializer::writeInt32(buffer, val);
        auto constBuffer = getConstBuffer();
        EXPECT_EQ(BinarySerializer::readInt32(constBuffer, offset), val);
    }
}

TEST_F(BinarySerializerTest, RoundTripFloat) {
    float values[] = {0.0f, 1.0f, -1.0f, 3.14159f, -2.71828f,
                      std::numeric_limits<float>::min(),
                      std::numeric_limits<float>::max()};
    for (auto val : values) {
        clearBuffer();
        resetOffset();
        BinarySerializer::writeFloat(buffer, val);
        auto constBuffer = getConstBuffer();
        EXPECT_FLOAT_EQ(BinarySerializer::readFloat(constBuffer, offset), val);
    }
}

TEST_F(BinarySerializerTest, RoundTripString) {
    std::vector<std::string> values = {"", "a", "Hello", "Test string with spaces",
                                        std::string(1000, 'x')};
    for (const auto& val : values) {
        clearBuffer();
        resetOffset();
        BinarySerializer::writeString(buffer, val);
        auto constBuffer = getConstBuffer();
        EXPECT_EQ(BinarySerializer::readString(constBuffer, offset), val);
    }
}

// =============================================================================
// Complex/Sequential Tests
// =============================================================================

TEST_F(BinarySerializerTest, MultipleValuesSequential) {
    // Write multiple values of different types
    BinarySerializer::writeUint8(buffer, 42);
    BinarySerializer::writeUint16(buffer, 1234);
    BinarySerializer::writeUint32(buffer, 567890);
    BinarySerializer::writeInt32(buffer, -9999);
    BinarySerializer::writeFloat(buffer, 1.5f);
    BinarySerializer::writeString(buffer, "test");
    auto constBuffer = getConstBuffer();

    // Read them back in order
    EXPECT_EQ(BinarySerializer::readUint8(constBuffer, offset), 42);
    EXPECT_EQ(BinarySerializer::readUint16(constBuffer, offset), 1234);
    EXPECT_EQ(BinarySerializer::readUint32(constBuffer, offset), 567890);
    EXPECT_EQ(BinarySerializer::readInt32(constBuffer, offset), -9999);
    EXPECT_FLOAT_EQ(BinarySerializer::readFloat(constBuffer, offset), 1.5f);
    EXPECT_EQ(BinarySerializer::readString(constBuffer, offset), "test");
}

TEST_F(BinarySerializerTest, OffsetIncrementCorrectly) {
    BinarySerializer::writeUint8(buffer, 1);
    BinarySerializer::writeUint16(buffer, 2);
    BinarySerializer::writeUint32(buffer, 3);
    BinarySerializer::writeUint64(buffer, 4);
    auto constBuffer = getConstBuffer();

    size_t expectedOffset = 0;

    BinarySerializer::readUint8(constBuffer, offset);
    expectedOffset += 1;
    EXPECT_EQ(*offset, expectedOffset);

    BinarySerializer::readUint16(constBuffer, offset);
    expectedOffset += 2;
    EXPECT_EQ(*offset, expectedOffset);

    BinarySerializer::readUint32(constBuffer, offset);
    expectedOffset += 4;
    EXPECT_EQ(*offset, expectedOffset);

    BinarySerializer::readUint64(constBuffer, offset);
    expectedOffset += 8;
    EXPECT_EQ(*offset, expectedOffset);
}

}  // namespace rtype::game::config
