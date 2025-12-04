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
#include <utility>
#include <variant>

namespace rtype::network {

/**
 * @brief Enumeration of all possible network errors
 *
 * This enum defines all error codes that can occur during network operations.
 * Errors are categorized by type for easier handling and debugging.
 */
enum class NetworkError : std::uint8_t {
    None = 0,

    // Connection Errors (1-19)
    NotConnected = 1,
    ConnectionRefused = 2,
    Timeout = 3,
    HostNotFound = 4,
    NetworkUnreachable = 5,
    AddressInUse = 6,

    // Protocol Errors (20-39)
    InvalidMagic = 20,
    UnknownOpcode = 21,
    PacketTooLarge = 22,
    PacketTooSmall = 23,
    MalformedPacket = 24,
    InvalidSequence = 25,
    InvalidUserId = 26,
    DuplicatePacket = 27,

    // Operation Errors (40-59)
    Cancelled = 40,
    WouldBlock = 41,
    BufferFull = 42,
    InternalError = 43,

    // Reliability Layer Errors (60-79)
    RetryLimitExceeded = 60,
    AckTimeout = 62,
};

/**
 * @brief Convert a NetworkError to its string representation
 *
 * @param error The error code to convert
 * @return A string view containing the error message
 */
[[nodiscard]] constexpr std::string_view toString(NetworkError error) noexcept {
    constexpr std::array<std::pair<NetworkError, std::string_view>, 24>
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
            {NetworkError::DuplicatePacket, "Duplicate packet"},
            {NetworkError::Cancelled, "Operation cancelled"},
            {NetworkError::WouldBlock, "Would block"},
            {NetworkError::BufferFull, "Buffer full"},
            {NetworkError::InternalError, "Internal error"},
            {NetworkError::RetryLimitExceeded, "Retry limit exceeded"},
            {NetworkError::AckTimeout, "ACK timeout"},
        }};

    for (const auto& [code, message] : kErrorMessages) {
        if (code == error) {
            return message;
        }
    }
    return "Unknown error";
}

/**
 * @brief A Result monad for handling operations that may succeed or fail
 *
 * This class provides a type-safe way to handle operations that can either
 * return a value of type T or an error of type E. It follows the Result pattern
 * commonly used in functional programming and Rust.
 *
 * @tparam T The type of the success value
 * @tparam E The type of the error (defaults to NetworkError)
 */
template <typename T, typename E = NetworkError>
class Result {
   public:
    static Result ok(T value) { return Result(std::move(value)); }
    static Result err(E error) { return Result(error); }

    Result(T value) : data_(std::move(value)) {}  // NOLINT: implicit

    template <typename U = E,
              typename = std::enable_if_t<!std::is_same_v<U, T>>>
    Result(E error) : data_(error) {}  // NOLINT: implicit

    [[nodiscard]] bool isOk() const noexcept {
        return std::holds_alternative<T>(data_);
    }

    [[nodiscard]] bool isErr() const noexcept {
        return std::holds_alternative<E>(data_);
    }

    explicit operator bool() const noexcept { return isOk(); }

    [[nodiscard]] T& value() & { return std::get<T>(data_); }
    [[nodiscard]] const T& value() const& { return std::get<T>(data_); }
    [[nodiscard]] T&& value() && { return std::get<T>(std::move(data_)); }
    [[nodiscard]] E error() const noexcept { return std::get<E>(data_); }

    [[nodiscard]] T valueOr(T defaultValue) const& {
        return isOk() ? value() : std::move(defaultValue);
    }

    [[nodiscard]] T valueOr(T defaultValue) && {
        return isOk() ? std::move(value()) : std::move(defaultValue);
    }

    template <typename Fn>
    auto map(Fn&& fn) const& -> Result<decltype(fn(std::declval<T>())), E> {
        using U = decltype(fn(std::declval<T>()));
        if (isOk()) {
            return Result<U, E>::ok(fn(value()));
        }
        return Result<U, E>::err(error());
    }

    template <typename Fn>
    auto mapErr(Fn&& fn) const& -> Result<T, decltype(fn(std::declval<E>()))> {
        using E2 = decltype(fn(std::declval<E>()));
        if (isErr()) {
            return Result<T, E2>::err(fn(error()));
        }
        return Result<T, E2>::ok(value());
    }

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
 * @brief Specialization of Result for void operations
 *
 * This specialization handles operations that don't return a value but can
 * still fail.
 *
 * @tparam E The type of the error
 */
template <typename E>
class Result<void, E> {
   public:
    static Result ok() { return Result(true); }
    static Result err(E error) { return Result(error); }

    Result() : data_(true) {}
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

/**
 * @brief Helper functions for creating Result instances
 *
 * These functions provide convenient ways to create Result objects
 * without having to specify template parameters explicitly.
 */
template <typename T>
[[nodiscard]] inline Result<T> Ok(T value) {
    return Result<T>::ok(std::move(value));
}

[[nodiscard]] inline Result<void> Ok() { return Result<void>::ok(); }

template <typename T = void>
[[nodiscard]] inline Result<T> Err(NetworkError error) {
    return Result<T>::err(error);
}

}  // namespace rtype::network

#include <ostream>

inline std::ostream& operator<<(std::ostream& os,
                                rtype::network::NetworkError error) {
    return os << rtype::network::toString(error);
}
