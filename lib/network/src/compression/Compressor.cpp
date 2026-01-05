/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Compressor - LZ4 compression implementation
*/

#include "Compressor.hpp"

#include <lz4frame.h>

namespace rtype::network {

Compressor::Compressor() noexcept : config_() {}

Compressor::Compressor(const Config& config) noexcept : config_(config) {}

bool Compressor::shouldCompress(std::size_t payloadSize) const noexcept {
    return payloadSize >= config_.minSizeThreshold;
}

std::size_t Compressor::maxCompressedSize(std::size_t originalSize) noexcept {
    return LZ4F_compressFrameBound(originalSize, nullptr);
}

CompressionResult Compressor::compress(const Buffer& payload) const {
    CompressionResult result;
    result.originalSize = payload.size();
    result.wasCompressed = false;

    // Skip compression for small payloads
    if (!shouldCompress(payload.size())) {
        result.data = payload;
        return result;
    }

    // Pre-allocate buffer for worst-case compressed size
    std::size_t maxSize = maxCompressedSize(payload.size());
    Buffer compressed(maxSize);

    // LZ4 frame preferences (use defaults)
    LZ4F_preferences_t prefs{};
    prefs.compressionLevel = 0;  // Default compression level
    prefs.autoFlush = 1;
    prefs.frameInfo.contentSize = payload.size();  // Store original size

    // Compress using LZ4 frame format
    std::size_t compressedSize = LZ4F_compressFrame(
        compressed.data(), compressed.size(), payload.data(), payload.size(),
        &prefs);

    // Check for compression error
    if (LZ4F_isError(compressedSize)) {
        // Compression failed, return original
        result.data = payload;
        return result;
    }

    // Check if compression was beneficial
    auto ratio = static_cast<float>(compressedSize) /
                 static_cast<float>(payload.size());
    if (ratio > config_.maxExpansionRatio) {
        // Compression not beneficial, return original
        result.data = payload;
        return result;
    }

    // Compression successful and beneficial
    compressed.resize(compressedSize);
    result.data = std::move(compressed);
    result.wasCompressed = true;
    return result;
}

Result<Buffer> Compressor::decompress(const Buffer& compressedData) const {
    if (compressedData.empty()) {
        return Err<Buffer>(NetworkError::DecompressionFailed);
    }

    // Create decompression context
    LZ4F_dctx* dctx = nullptr;
    LZ4F_errorCode_t err =
        LZ4F_createDecompressionContext(&dctx, LZ4F_VERSION);
    if (LZ4F_isError(err)) {
        return Err<Buffer>(NetworkError::DecompressionFailed);
    }

    // RAII cleanup for context
    struct ContextGuard {
        LZ4F_dctx* ctx;
        ~ContextGuard() {
            if (ctx) {
                LZ4F_freeDecompressionContext(ctx);
            }
        }
    } guard{dctx};

    // Get frame info to determine original size
    LZ4F_frameInfo_t frameInfo{};
    std::size_t srcSize = compressedData.size();
    err = LZ4F_getFrameInfo(dctx, &frameInfo, compressedData.data(), &srcSize);
    if (LZ4F_isError(err)) {
        return Err<Buffer>(NetworkError::DecompressionFailed);
    }

    // Allocate output buffer
    // contentSize may be 0 if not stored in frame, use conservative estimate
    std::size_t dstCapacity = frameInfo.contentSize > 0
                                  ? frameInfo.contentSize
                                  : compressedData.size() * 4;

    // Safety limit: max payload size
    if (dstCapacity > kMaxPacketSize) {
        dstCapacity = kMaxPacketSize;
    }

    Buffer decompressed(dstCapacity);

    // Decompress remaining data after frame info
    const std::uint8_t* srcPtr = compressedData.data() + srcSize;
    std::size_t srcRemaining = compressedData.size() - srcSize;
    std::size_t dstSize = decompressed.size();

    std::size_t decompressResult = LZ4F_decompress(
        dctx, decompressed.data(), &dstSize, srcPtr, &srcRemaining, nullptr);

    if (LZ4F_isError(decompressResult)) {
        return Err<Buffer>(NetworkError::DecompressionFailed);
    }

    decompressed.resize(dstSize);
    return Ok(std::move(decompressed));
}

}  // namespace rtype::network
