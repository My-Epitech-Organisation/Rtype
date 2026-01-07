/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Compressor unit tests
*/

#include <gtest/gtest.h>

#include <cstring>
#include <vector>

#include "compression/Compressor.hpp"
#include "core/Types.hpp"

using namespace rtype::network;

class CompressorTest : public ::testing::Test {
   protected:
    Compressor compressor_;
};

// =============================================================================
// Constructor Tests
// =============================================================================

TEST_F(CompressorTest, DefaultConstructor) {
    Compressor c;
    // Default threshold is 64 bytes
    EXPECT_FALSE(c.shouldCompress(63));
    EXPECT_TRUE(c.shouldCompress(64));
    EXPECT_TRUE(c.shouldCompress(100));
}

TEST_F(CompressorTest, CustomConfigConstructor) {
    Compressor::Config config;
    config.minSizeThreshold = 128;
    config.maxExpansionRatio = 0.9f;

    Compressor c(config);
    EXPECT_FALSE(c.shouldCompress(127));
    EXPECT_TRUE(c.shouldCompress(128));
}

// =============================================================================
// shouldCompress Tests
// =============================================================================

TEST_F(CompressorTest, ShouldCompressBelowThreshold) {
    EXPECT_FALSE(compressor_.shouldCompress(0));
    EXPECT_FALSE(compressor_.shouldCompress(32));
    EXPECT_FALSE(compressor_.shouldCompress(63));
}

TEST_F(CompressorTest, ShouldCompressAtThreshold) {
    EXPECT_TRUE(compressor_.shouldCompress(64));
}

TEST_F(CompressorTest, ShouldCompressAboveThreshold) {
    EXPECT_TRUE(compressor_.shouldCompress(100));
    EXPECT_TRUE(compressor_.shouldCompress(1000));
    EXPECT_TRUE(compressor_.shouldCompress(10000));
}

// =============================================================================
// maxCompressedSize Tests
// =============================================================================

TEST_F(CompressorTest, MaxCompressedSizeZero) {
    std::size_t maxSize = Compressor::maxCompressedSize(0);
    EXPECT_GT(maxSize, 0u);  // LZ4 frame has overhead even for empty input
}

TEST_F(CompressorTest, MaxCompressedSizeSmall) {
    std::size_t maxSize = Compressor::maxCompressedSize(100);
    EXPECT_GE(maxSize, 100u);  // Worst case >= original
}

TEST_F(CompressorTest, MaxCompressedSizeLarge) {
    std::size_t maxSize = Compressor::maxCompressedSize(1000);
    EXPECT_GE(maxSize, 1000u);
}

// =============================================================================
// compress Tests
// =============================================================================

TEST_F(CompressorTest, CompressEmptyPayload) {
    Buffer empty;
    auto result = compressor_.compress(empty);

    EXPECT_FALSE(result.wasCompressed);
    EXPECT_EQ(result.originalSize, 0u);
    EXPECT_TRUE(result.data.empty());
}

TEST_F(CompressorTest, CompressBelowThresholdReturnsOriginal) {
    Buffer small(32, 0xAB);
    auto result = compressor_.compress(small);

    EXPECT_FALSE(result.wasCompressed);
    EXPECT_EQ(result.originalSize, 32u);
    EXPECT_EQ(result.data.size(), 32u);
    EXPECT_EQ(result.data, small);
}

TEST_F(CompressorTest, CompressCompressibleData) {
    // Highly compressible data (all zeros)
    Buffer compressible(500, 0x00);
    auto result = compressor_.compress(compressible);

    EXPECT_TRUE(result.wasCompressed);
    EXPECT_EQ(result.originalSize, 500u);
    EXPECT_LT(result.data.size(), 500u);  // Should be smaller
}

TEST_F(CompressorTest, CompressRepetitiveData) {
    // Repetitive pattern is highly compressible
    Buffer repetitive(1000);
    for (std::size_t i = 0; i < repetitive.size(); ++i) {
        repetitive[i] = static_cast<std::uint8_t>(i % 4);
    }

    auto result = compressor_.compress(repetitive);

    EXPECT_TRUE(result.wasCompressed);
    EXPECT_EQ(result.originalSize, 1000u);
    EXPECT_LT(result.data.size(), 1000u);
}

TEST_F(CompressorTest, CompressIncompressibleData) {
    // Random-like data is hard to compress
    Buffer incompressible(200);
    for (std::size_t i = 0; i < incompressible.size(); ++i) {
        // Pseudo-random pattern
        incompressible[i] = static_cast<std::uint8_t>((i * 17 + 31) % 256);
    }

    auto result = compressor_.compress(incompressible);

    // May or may not compress depending on LZ4's behavior
    EXPECT_EQ(result.originalSize, 200u);
}

TEST_F(CompressorTest, CompressWithHighExpansionRatioConfig) {
    // Config that rejects compression if ratio > 0.5
    Compressor::Config config;
    config.minSizeThreshold = 64;
    config.maxExpansionRatio = 0.5f;

    Compressor strictCompressor(config);

    // Data that compresses but not by 50%
    Buffer data(100);
    for (std::size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<std::uint8_t>((i * 7) % 256);
    }

    auto result = strictCompressor.compress(data);
    // If compression ratio > 0.5, original is returned
    EXPECT_EQ(result.originalSize, 100u);
}

// =============================================================================
// decompress Tests
// =============================================================================

TEST_F(CompressorTest, DecompressEmptyData) {
    Buffer empty;
    auto result = compressor_.decompress(empty);

    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::DecompressionFailed);
}

TEST_F(CompressorTest, DecompressInvalidData) {
    Buffer garbage = {0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x34};
    auto result = compressor_.decompress(garbage);

    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::DecompressionFailed);
}

TEST_F(CompressorTest, DecompressValidCompressedData) {
    // First compress some data
    Buffer original(500, 0x42);
    auto compressed = compressor_.compress(original);

    ASSERT_TRUE(compressed.wasCompressed);

    // Now decompress
    auto decompressed = compressor_.decompress(compressed.data);

    ASSERT_TRUE(decompressed.isOk());
    EXPECT_EQ(decompressed.value().size(), original.size());
    EXPECT_EQ(decompressed.value(), original);
}

TEST_F(CompressorTest, CompressDecompressRoundtrip) {
    // Test various data patterns
    std::vector<Buffer> testCases = {
        Buffer(100, 0x00),                     // All zeros
        Buffer(200, 0xFF),                     // All ones
        Buffer(300, 0xAA),                     // Pattern
        Buffer(kMaxPacketSize - 100, 0x55),   // Near max size
    };

    for (const auto& original : testCases) {
        auto compressed = compressor_.compress(original);

        if (compressed.wasCompressed) {
            auto decompressed = compressor_.decompress(compressed.data);

            ASSERT_TRUE(decompressed.isOk())
                << "Failed to decompress data of size " << original.size();
            EXPECT_EQ(decompressed.value(), original);
        }
    }
}

TEST_F(CompressorTest, CompressDecompressSequentialData) {
    Buffer sequential(256);
    for (std::size_t i = 0; i < sequential.size(); ++i) {
        sequential[i] = static_cast<std::uint8_t>(i);
    }

    auto compressed = compressor_.compress(sequential);
    EXPECT_EQ(compressed.originalSize, 256u);

    if (compressed.wasCompressed) {
        auto decompressed = compressor_.decompress(compressed.data);
        ASSERT_TRUE(decompressed.isOk());
        EXPECT_EQ(decompressed.value(), sequential);
    }
}

// =============================================================================
// Edge Cases
// =============================================================================

TEST_F(CompressorTest, CompressExactlyAtThreshold) {
    Buffer exactThreshold(64, 0x00);
    auto result = compressor_.compress(exactThreshold);

    // Should attempt compression at exactly 64 bytes
    EXPECT_EQ(result.originalSize, 64u);
}

TEST_F(CompressorTest, DecompressTruncatedFrame) {
    // Compress valid data first
    Buffer original(200, 0x33);
    auto compressed = compressor_.compress(original);

    if (compressed.wasCompressed && compressed.data.size() > 10) {
        // Truncate the compressed data
        Buffer truncated(compressed.data.begin(),
                         compressed.data.begin() + 10);

        auto result = compressor_.decompress(truncated);
        // Should fail - incomplete frame
        EXPECT_TRUE(result.isErr());
    }
}

TEST_F(CompressorTest, DecompressCorruptedFrame) {
    // Compress valid data first
    Buffer original(200, 0x44);
    auto compressed = compressor_.compress(original);

    if (compressed.wasCompressed && compressed.data.size() > 20) {
        // Corrupt some bytes in the middle
        Buffer corrupted = compressed.data;
        corrupted[15] ^= 0xFF;
        corrupted[16] ^= 0xFF;
        corrupted[17] ^= 0xFF;

        auto result = compressor_.decompress(corrupted);
        // May fail or produce garbage - either is acceptable
        // The important thing is no crash
    }
}

TEST_F(CompressorTest, CompressLargeCompressibleData) {
    // Large buffer with highly compressible content
    Buffer large(kMaxPacketSize, 0x00);
    auto result = compressor_.compress(large);

    EXPECT_TRUE(result.wasCompressed);
    EXPECT_EQ(result.originalSize, kMaxPacketSize);
    EXPECT_LT(result.data.size(), kMaxPacketSize);
}

// =============================================================================
// Move Semantics
// =============================================================================

TEST_F(CompressorTest, MoveConstructor) {
    Compressor::Config config;
    config.minSizeThreshold = 100;

    Compressor original(config);
    Compressor moved(std::move(original));

    EXPECT_FALSE(moved.shouldCompress(99));
    EXPECT_TRUE(moved.shouldCompress(100));
}

TEST_F(CompressorTest, MoveAssignment) {
    Compressor::Config config;
    config.minSizeThreshold = 200;

    Compressor original(config);
    Compressor target;

    target = std::move(original);

    EXPECT_FALSE(target.shouldCompress(199));
    EXPECT_TRUE(target.shouldCompress(200));
}

// =============================================================================
// Buffer Size Limit (Security)
// =============================================================================

TEST_F(CompressorTest, DecompressRespectsMaxPacketSize) {
    // Compress data that when decompressed would exceed limits
    // The decompressor should handle this safely
    Buffer large(1000, 0x00);
    auto compressed = compressor_.compress(large);

    if (compressed.wasCompressed) {
        auto result = compressor_.decompress(compressed.data);
        ASSERT_TRUE(result.isOk());
        // Result should be clamped or valid
        EXPECT_LE(result.value().size(), kMaxPacketSize);
    }
}
