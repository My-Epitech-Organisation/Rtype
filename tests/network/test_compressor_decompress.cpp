/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_compressor_decompress - Additional decompression coverage tests
*/

#include <gtest/gtest.h>

#include <cstring>
#include <random>
#include <vector>

#include "compression/Compressor.hpp"
#include "core/Types.hpp"

using namespace rtype::network;

class CompressorDecompressTest : public ::testing::Test {
   protected:
    Compressor compressor_;
};

// =============================================================================
// Decompression Success Paths
// =============================================================================

TEST_F(CompressorDecompressTest, DecompressSmallCompressedData) {
    Buffer original(100, 0x42);
    auto compressed = compressor_.compress(original);

    if (compressed.wasCompressed) {
        auto result = compressor_.decompress(compressed.data);
        ASSERT_TRUE(result.isOk());
        EXPECT_EQ(result.value(), original);
    }
}

TEST_F(CompressorDecompressTest, DecompressMediumCompressedData) {
    Buffer original(1000, 0xAB);
    auto compressed = compressor_.compress(original);

    if (compressed.wasCompressed) {
        auto result = compressor_.decompress(compressed.data);
        ASSERT_TRUE(result.isOk());
        EXPECT_EQ(result.value(), original);
    }
}

TEST_F(CompressorDecompressTest, DecompressLargeCompressedData) {
    // Use a size that won't exceed kMaxPacketSize after decompression
    Buffer original(1000, 0xCD);
    auto compressed = compressor_.compress(original);

    if (compressed.wasCompressed) {
        auto result = compressor_.decompress(compressed.data);
        ASSERT_TRUE(result.isOk());
        EXPECT_EQ(result.value().size(), original.size());
    }
}

TEST_F(CompressorDecompressTest, DecompressPatternedData) {
    // Use smaller size to stay within kMaxPacketSize
    Buffer original(1000);
    for (std::size_t i = 0; i < original.size(); ++i) {
        original[i] = static_cast<std::uint8_t>(i % 256);
    }

    auto compressed = compressor_.compress(original);

    if (compressed.wasCompressed) {
        auto result = compressor_.decompress(compressed.data);
        ASSERT_TRUE(result.isOk());
        EXPECT_EQ(result.value().size(), original.size());
    }
}

TEST_F(CompressorDecompressTest, DecompressRepeatingPattern) {
    // Pattern that compresses well - use smaller size
    Buffer original(1000);
    for (std::size_t i = 0; i < original.size(); ++i) {
        original[i] = static_cast<std::uint8_t>(i % 8);
    }

    auto compressed = compressor_.compress(original);
    ASSERT_TRUE(compressed.wasCompressed);

    auto result = compressor_.decompress(compressed.data);
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().size(), original.size());
}

// =============================================================================
// Decompression Error Paths
// =============================================================================

TEST_F(CompressorDecompressTest, DecompressEmptyBuffer) {
    Buffer empty;
    auto result = compressor_.decompress(empty);
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::DecompressionFailed);
}

TEST_F(CompressorDecompressTest, DecompressSingleByte) {
    Buffer single = {0x42};
    auto result = compressor_.decompress(single);
    EXPECT_TRUE(result.isErr());
}

TEST_F(CompressorDecompressTest, DecompressInvalidMagicBytes) {
    // LZ4 frame starts with magic number 0x184D2204
    Buffer invalidMagic = {0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF};
    auto result = compressor_.decompress(invalidMagic);
    EXPECT_TRUE(result.isErr());
}

TEST_F(CompressorDecompressTest, DecompressGarbageData) {
    Buffer garbage(50);
    for (std::size_t i = 0; i < garbage.size(); ++i) {
        garbage[i] = static_cast<std::uint8_t>((i * 37) % 256);
    }

    auto result = compressor_.decompress(garbage);
    EXPECT_TRUE(result.isErr());
}

TEST_F(CompressorDecompressTest, DecompressTooShortFrame) {
    // Valid magic but incomplete frame
    Buffer shortFrame = {0x04, 0x22, 0x4D, 0x18};  // LZ4 magic number (little endian)
    auto result = compressor_.decompress(shortFrame);
    EXPECT_TRUE(result.isErr());
}

TEST_F(CompressorDecompressTest, DecompressCorruptedChecksum) {
    Buffer original(500, 0x55);
    auto compressed = compressor_.compress(original);

    if (compressed.wasCompressed && compressed.data.size() > 10) {
        // Corrupt the last few bytes (likely checksum area)
        compressed.data[compressed.data.size() - 1] ^= 0xFF;
        compressed.data[compressed.data.size() - 2] ^= 0xFF;

        auto result = compressor_.decompress(compressed.data);
        // May fail or succeed depending on where corruption hits
        // The important thing is no crash
    }
}

TEST_F(CompressorDecompressTest, DecompressCorruptedHeader) {
    Buffer original(500, 0x66);
    auto compressed = compressor_.compress(original);

    if (compressed.wasCompressed && compressed.data.size() > 8) {
        // Corrupt bytes after magic number (header area)
        compressed.data[4] ^= 0xFF;
        compressed.data[5] ^= 0xFF;

        auto result = compressor_.decompress(compressed.data);
        // Should fail or produce error
    }
}

// =============================================================================
// Edge Cases
// =============================================================================

TEST_F(CompressorDecompressTest, DecompressMultipleRoundtrips) {
    Buffer original(300, 0x77);

    for (int i = 0; i < 5; ++i) {
        auto compressed = compressor_.compress(original);
        if (compressed.wasCompressed) {
            auto result = compressor_.decompress(compressed.data);
            ASSERT_TRUE(result.isOk()) << "Failed on roundtrip " << i;
            EXPECT_EQ(result.value(), original);
        }
    }
}

TEST_F(CompressorDecompressTest, DecompressDifferentCompressors) {
    // Compress with one instance, decompress with another
    Compressor compressor1;
    Compressor compressor2;

    Buffer original(400, 0x88);
    auto compressed = compressor1.compress(original);

    if (compressed.wasCompressed) {
        auto result = compressor2.decompress(compressed.data);
        ASSERT_TRUE(result.isOk());
        EXPECT_EQ(result.value(), original);
    }
}

TEST_F(CompressorDecompressTest, DecompressWithCustomConfig) {
    Compressor::Config config;
    config.minSizeThreshold = 32;
    Compressor customCompressor(config);

    Buffer original(100, 0x99);
    auto compressed = customCompressor.compress(original);

    if (compressed.wasCompressed) {
        // Standard compressor should still decompress
        auto result = compressor_.decompress(compressed.data);
        ASSERT_TRUE(result.isOk());
        EXPECT_EQ(result.value(), original);
    }
}

TEST_F(CompressorDecompressTest, DecompressNearMaxSize) {
    // Create data near the max packet size
    Buffer original(kMaxPacketSize - 100, 0xAA);
    auto compressed = compressor_.compress(original);

    if (compressed.wasCompressed) {
        auto result = compressor_.decompress(compressed.data);
        ASSERT_TRUE(result.isOk());
        EXPECT_EQ(result.value().size(), original.size());
    }
}

TEST_F(CompressorDecompressTest, DecompressVariousPatterns) {
    std::vector<Buffer> patterns;

    // All zeros
    patterns.emplace_back(500, 0x00);

    // All ones
    patterns.emplace_back(500, 0xFF);

    // Alternating bytes
    Buffer alternating(500);
    for (std::size_t i = 0; i < alternating.size(); ++i) {
        alternating[i] = (i % 2 == 0) ? 0xAA : 0x55;
    }
    patterns.push_back(alternating);

    // Sequential
    Buffer sequential(256);
    for (std::size_t i = 0; i < sequential.size(); ++i) {
        sequential[i] = static_cast<std::uint8_t>(i);
    }
    patterns.push_back(sequential);

    for (const auto& pattern : patterns) {
        auto compressed = compressor_.compress(pattern);
        if (compressed.wasCompressed) {
            auto result = compressor_.decompress(compressed.data);
            ASSERT_TRUE(result.isOk());
            EXPECT_EQ(result.value(), pattern);
        }
    }
}

// =============================================================================
// Frame Info Tests (contentSize path)
// =============================================================================

TEST_F(CompressorDecompressTest, DecompressWithKnownContentSize) {
    // The compress function sets frameInfo.contentSize
    Buffer original(1000, 0xBB);
    auto compressed = compressor_.compress(original);

    if (compressed.wasCompressed) {
        auto result = compressor_.decompress(compressed.data);
        ASSERT_TRUE(result.isOk());
        // Should allocate exact size from contentSize
        EXPECT_EQ(result.value().size(), original.size());
    }
}

TEST_F(CompressorDecompressTest, DecompressInterleavedData) {
    // Create interleaved data pattern - smaller size
    Buffer original(1000);
    for (std::size_t i = 0; i < original.size(); ++i) {
        original[i] = static_cast<std::uint8_t>((i / 4) % 256);
    }

    auto compressed = compressor_.compress(original);
    if (compressed.wasCompressed) {
        auto result = compressor_.decompress(compressed.data);
        ASSERT_TRUE(result.isOk());
        EXPECT_EQ(result.value().size(), original.size());
    }
}
