/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Network Error Handling - Error types and Result monad
*/

#pragma once

#include <array>
#include <cstdint>
#include <string_view>
#include <system_error>
#include <variant>

namespace rtype::network {

// ============================================================================
// Network Error Codes
// ============================================================================

/**
 * @brief Network-specific error codes
 *
 * These errors are specific to the R-Type network protocol and operations.
 * For system-level socket errors, use std::error_code from Asio.
 */
enum class NetworkError : std::uint8_t {
    /// No error (success)
    None = 0,

    // --- Connection Errors (1-19) ---
    /// Socket not bound or connected
    NotConnected = 1,
    /// Connection was refused by remote host
    ConnectionRefused = 2,
    /// Connection timed out
    Timeout = 3,
    /// Host not found (DNS resolution failed)
    HostNotFound = 4,
    /// Network is unreachable
    NetworkUnreachable = 5,
    /// Address already in use (bind failed)
    AddressInUse = 6,

    // --- Protocol Errors (20-39) ---
    /// Invalid magic byte in packet header
    InvalidMagic = 20,
    /// Unknown or unsupported opcode
    UnknownOpcode = 21,
    /// Packet size exceeds MTU limit
    PacketTooLarge = 22,
    /// Packet size smaller than header
    PacketTooSmall = 23,
    /// Malformed packet structure
    MalformedPacket = 24,
    /// Invalid sequence ID (replay attack or old packet)
    InvalidSequence = 25,
    /// User ID validation failed
    InvalidUserId = 26,

    // --- Operation Errors (40-59) ---
    /// Operation was cancelled
    Cancelled = 40,
    /// Resource temporarily unavailable (would block)
    WouldBlock = 41,
    /// Buffer is full, cannot queue more data
    BufferFull = 42,
    /// Internal error (bug)
    InternalError = 43,

    // --- Reliability Layer Errors (60-79) ---
    /// Max retransmission attempts exceeded
    MaxRetriesExceeded = 60,
    /// ACK timeout waiting for acknowledgement
    AckTimeout = 61,
};

/**
 * @brief Convert NetworkError to human-readable string
 */
[[nodiscard]] constexpr std::string_view toString(NetworkError error) noexcept {
    constexpr std::array<std::pair<NetworkError, std::string_view>, 22>
        kErrorMessages = {{
            {NetworkError::None, "Success"},
            {NetworkError::NotConnected, "Not connected"},
            {NetworkError::ConnectionRefused, "Connection refused"},
            {NetworkError::Timeout, "Operation timed out"},
            {NetworkError::HostNotFound, "Host not found"},
            {NetworkError::NetworkUnreachable, "Network unreachable"},
            {NetworkError::AddressInUse, "Address already in use"},
            {NetworkError::InvalidMagic, "Invalid magic byte"},
            {NetworkError::UnknownOpcode, "Unknown opcode"},
            {NetworkError::PacketTooLarge, "Packet too large"},
            {NetworkError::PacketTooSmall, "Packet too small"},
            {NetworkError::MalformedPacket, "Malformed packet"},
            {NetworkError::InvalidSequence, "Invalid sequence ID"},
            {NetworkError::InvalidUserId, "Invalid user ID"},
            {NetworkError::Cancelled, "Operation cancelled"},
            {NetworkError::WouldBlock, "Would block"},
            {NetworkError::BufferFull, "Buffer full"},
            {NetworkError::InternalError, "Internal error"},
            {NetworkError::MaxRetriesExceeded, "Max retries exceeded"},
            {NetworkError::AckTimeout, "ACK timeout"},
        }};

    for (const auto& [code, message] : kErrorMessages) {
        if (code == error) {
            return message;
        }
    }
    return "Unknown error";
}

// ============================================================================
// Result Type (Error Handling without Exceptions)
// ============================================================================

/**
 * @brief Result type for operations that can fail
 *
 * Lightweight exception-free error handling. Use isOk()/isErr() to check state,
 * value()/error() for access, map()/andThen() for functional composition.
 *
 * @tparam T The success value type
 * @tparam E The error type (defaults to NetworkError)
 */
template <typename T, typename E = NetworkError>
class Result {
   public:
    // --- Constructors ---

    /// Construct success result
    static Result ok(T value) { return Result(std::move(value)); }

    /// Construct error result
    static Result err(E error) { return Result(error); }

    /// Construct success result (implicit conversion)
    Result(T value) : data_(std::move(value)) {}  // NOLINT: implicit

    /// Construct error result (implicit conversion for error type)
    template <typename U = E,
              typename = std::enable_if_t<!std::is_same_v<U, T>>>
    Result(E error) : data_(error) {}  // NOLINT: implicit

    // --- State Queries ---

    /// Check if result is success
    [[nodiscard]] bool isOk() const noexcept {
        return std::holds_alternative<T>(data_);
    }

    /// Check if result is error
    [[nodiscard]] bool isErr() const noexcept {
        return std::holds_alternative<E>(data_);
    }

    /// Bool conversion (true if success)
    explicit operator bool() const noexcept { return isOk(); }

    // --- Value Access ---

    /**
     * @brief Get success value (undefined behavior if error)
     * @pre isOk() == true
     */
    [[nodiscard]] T& value() & { return std::get<T>(data_); }

    [[nodiscard]] const T& value() const& { return std::get<T>(data_); }

    [[nodiscard]] T&& value() && { return std::get<T>(std::move(data_)); }

    /**
     * @brief Get error value (undefined behavior if success)
     * @pre isErr() == true
     */
    [[nodiscard]] E error() const noexcept { return std::get<E>(data_); }

    /**
     * @brief Get value or default if error
     */
    [[nodiscard]] T valueOr(T defaultValue) const& {
        return isOk() ? value() : std::move(defaultValue);
    }

    [[nodiscard]] T valueOr(T defaultValue) && {
        return isOk() ? std::move(value()) : std::move(defaultValue);
    }

    // --- Functional Operations ---

    /**
     * @brief Apply function to value if success
     * @param fn Function T -> U
     * @return Result<U, E>
     */
    template <typename Fn>
    auto map(Fn&& fn) const& -> Result<decltype(fn(std::declval<T>())), E> {
        using U = decltype(fn(std::declval<T>()));
        if (isOk()) {
            return Result<U, E>::ok(fn(value()));
        }
        return Result<U, E>::err(error());
    }

    /**
     * @brief Apply function to error if error
     * @param fn Function E -> E2
     * @return Result<T, E2>
     */
    template <typename Fn>
    auto mapErr(Fn&& fn) const& -> Result<T, decltype(fn(std::declval<E>()))> {
        using E2 = decltype(fn(std::declval<E>()));
        if (isErr()) {
            return Result<T, E2>::err(fn(error()));
        }
        return Result<T, E2>::ok(value());
    }

    /**
     * @brief Chain operations (flatMap/bind)
     * @param fn Function T -> Result<U, E>
     * @return Result<U, E>
     */
    template <typename Fn>
    auto andThen(Fn&& fn) const& -> decltype(fn(std::declval<T>())) {
        if (isOk()) {
            return fn(value());
        }
        using ReturnType = decltype(fn(std::declval<T>()));
        return ReturnType::err(error());
    }

   private:
    std::variant<T, E> data_;
};

/**
 * @brief Specialization for void success type
 *
 * Used when an operation can fail but has no return value on success.
 */
template <typename E>
class Result<void, E> {
   public:
    /// Construct success result
    static Result ok() { return Result(true); }

    /// Construct error result
    static Result err(E error) { return Result(error); }

    // Success constructor
    Result() : data_(true) {}

    // Error constructor
    Result(E error) : data_(error) {}  // NOLINT: implicit

    [[nodiscard]] bool isOk() const noexcept {
        return std::holds_alternative<bool>(data_);
    }

    [[nodiscard]] bool isErr() const noexcept {
        return std::holds_alternative<E>(data_);
    }

    explicit operator bool() const noexcept { return isOk(); }

    [[nodiscard]] E error() const noexcept { return std::get<E>(data_); }

   private:
    explicit Result(bool success) : data_(success) {}

    std::variant<bool, E> data_;
};

// ============================================================================
// Helper Functions
// ============================================================================

/// Create success result
template <typename T>
[[nodiscard]] inline Result<T> Ok(T value) {
    return Result<T>::ok(std::move(value));
}

/// Create void success result
[[nodiscard]] inline Result<void> Ok() { return Result<void>::ok(); }

/// Create error result
template <typename T = void>
[[nodiscard]] inline Result<T> Err(NetworkError error) {
    return Result<T>::err(error);
}

}  // namespace rtype::network
