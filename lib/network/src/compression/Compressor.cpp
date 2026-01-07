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

    if (!shouldCompress(payload.size())) {
        result.data = payload;
        return result;
    }

    std::size_t maxSize = maxCompressedSize(payload.size());
    Buffer compressed(maxSize);

    LZ4F_preferences_t prefs{};
    prefs.compressionLevel = 0;
    prefs.autoFlush = 1;
    prefs.frameInfo.contentSize = payload.size();

    std::size_t compressedSize = LZ4F_compressFrame(
        compressed.data(), compressed.size(), payload.data(), payload.size(),
        &prefs);

    if (LZ4F_isError(compressedSize)) {
        result.data = payload;
        return result;
    }

    auto ratio = static_cast<float>(compressedSize) /
                 static_cast<float>(payload.size());
    if (ratio > config_.maxExpansionRatio) {
        result.data = payload;
        return result;
    }

    compressed.resize(compressedSize);
    result.data = std::move(compressed);
    result.wasCompressed = true;
    return result;
}

Result<Buffer> Compressor::decompress(const Buffer& compressedData) const {
    if (compressedData.empty()) {
        return Err<Buffer>(NetworkError::DecompressionFailed);
    }

    LZ4F_dctx* dctx = nullptr;
    LZ4F_errorCode_t err =
        LZ4F_createDecompressionContext(&dctx, LZ4F_VERSION);
    if (LZ4F_isError(err)) {
        return Err<Buffer>(NetworkError::DecompressionFailed);
    }

    struct ContextGuard {
        LZ4F_dctx* ctx;
        ~ContextGuard() {
            if (ctx) {
                LZ4F_freeDecompressionContext(ctx);
            }
        }
    } guard{dctx};

    LZ4F_frameInfo_t frameInfo{};
    std::size_t srcSize = compressedData.size();
    err = LZ4F_getFrameInfo(dctx, &frameInfo, compressedData.data(), &srcSize);
    if (LZ4F_isError(err)) {
        return Err<Buffer>(NetworkError::DecompressionFailed);
    }

    std::size_t dstCapacity = frameInfo.contentSize > 0
                                  ? frameInfo.contentSize
                                  : compressedData.size() * 4;

    if (dstCapacity > kMaxPacketSize) {
        dstCapacity = kMaxPacketSize;
    }

    Buffer decompressed(dstCapacity);

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
