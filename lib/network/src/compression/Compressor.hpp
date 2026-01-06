/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Compressor - LZ4 compression utility for network packets (RFC RTGP v1.4.0)
*/

#pragma once

#include <cstdint>

#include "core/Error.hpp"
#include "core/Types.hpp"

namespace rtype::network {

/**
 * @brief Result of a compression operation
 */
struct CompressionResult {
    Buffer data;               ///< Compressed data (or original if not compressed)
    std::size_t originalSize;  ///< Original uncompressed size
    bool wasCompressed;        ///< True if compression was applied
};

/**
 * @brief LZ4 compression utility for network packets
 *
 * Provides transparent compression/decompression using LZ4 frame format.
 * Compression is only applied when beneficial (compressed < original).
 *
 * Thread-safety: All methods are thread-safe (stateless).
 *
 * @see RFC RTGP v1.4.0 Section 4.4
 */
class Compressor {
   public:
    /**
     * @brief Configuration for compression behavior
     */
    struct Config {
        std::size_t minSizeThreshold = 64;

        float maxExpansionRatio = 1.0f;
    };

    Compressor() noexcept;
    explicit Compressor(const Config& config) noexcept;
    ~Compressor() = default;

    Compressor(const Compressor&) = delete;
    Compressor& operator=(const Compressor&) = delete;
    Compressor(Compressor&&) = default;
    Compressor& operator=(Compressor&&) = default;

    /**
     * @brief Attempt to compress payload
     *
     * Uses LZ4 frame format which includes original size metadata.
     * Returns uncompressed data if compression is not beneficial.
     *
     * @param payload Raw payload data to compress
     * @return CompressionResult with compressed data and metadata
     */
    [[nodiscard]] CompressionResult compress(const Buffer& payload) const;

    /**
     * @brief Decompress LZ4 frame data
     *
     * Decompresses payload using LZ4 frame format.
     * Frame header contains original size for safe allocation.
     *
     * @param compressedData LZ4 frame data
     * @return Ok with decompressed data, Err on decompression failure
     */
    [[nodiscard]] Result<Buffer> decompress(const Buffer& compressedData) const;

    /**
     * @brief Check if payload should be compressed
     *
     * @param payloadSize Size of the payload in bytes
     * @return true if payload meets minimum size threshold
     */
    [[nodiscard]] bool shouldCompress(std::size_t payloadSize) const noexcept;

    /**
     * @brief Get maximum compressed size bound
     *
     * Returns the worst-case compressed size for buffer pre-allocation.
     *
     * @param originalSize Original payload size
     * @return Maximum possible compressed size
     */
    [[nodiscard]] static std::size_t
    maxCompressedSize(std::size_t originalSize) noexcept;

   private:
    Config config_;
};

}  // namespace rtype::network
