/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** OpCode - Protocol Operation Codes as per RFC RTGP v1.0.0
*/

#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace rtype::network {

/**
 * @brief Protocol Operation Codes as defined in RFC RTGP v1.0.0 Section 5
 *
 * OpCodes are divided into categories:
 * - Session Management (0x01-0x0F)
 * - Entity Management (0x10-0x1F)
 * - Input & Reconciliation (0x20-0x2F)
 * - Reserved/System (0xF0-0xFF)
 */
enum class OpCode : std::uint8_t {
    /// Client requests connection (RELIABLE)
    C_CONNECT = 0x01,

    /// Server accepts connection, assigns User ID (RELIABLE)
    S_ACCEPT = 0x02,

    /// Graceful session termination (RELIABLE)
    DISCONNECT = 0x03,

    /// Client requests list of connected users (RELIABLE)
    C_GET_USERS = 0x04,

    /// Server responds with user list (RELIABLE)
    R_GET_USERS = 0x05,

    /// Server notifies game state change (RELIABLE)
    S_UPDATE_STATE = 0x06,

    /// Server notifies game over with final score (RELIABLE)
    S_GAME_OVER = 0x07,

    /// Client signals ready in lobby (RELIABLE)
    C_READY = 0x08,

    /// Server signals game start with countdown (RELIABLE)
    S_GAME_START = 0x09,

    /// Server broadcasts player ready state change (RELIABLE)
    S_PLAYER_READY_STATE = 0x0A,

    /// Server spawns new entity (RELIABLE)
    S_ENTITY_SPAWN = 0x10,

    /// Server updates entity position/velocity (UNRELIABLE)
    S_ENTITY_MOVE = 0x11,

    /// Server destroys entity (RELIABLE)
    S_ENTITY_DESTROY = 0x12,

    /// Server updates entity health/lives (RELIABLE)
    S_ENTITY_HEALTH = 0x13,

    /// Server notifies a power-up pickup (RELIABLE)
    S_POWERUP_EVENT = 0x14,

    /// Client sends input state (UNRELIABLE)
    C_INPUT = 0x20,

    /// Server sends authoritative position (UNRELIABLE)
    S_UPDATE_POS = 0x21,

    /// Latency measurement request (UNRELIABLE)
    PING = 0xF0,

    /// Latency measurement response (UNRELIABLE)
    PONG = 0xF1,

    /// Acknowledgment packet (UNRELIABLE)
    ACK = 0xF2,
};

namespace OpCodeRange {
constexpr std::uint8_t kSessionMin = 0x01;
constexpr std::uint8_t kSessionMax = 0x0F;
constexpr std::uint8_t kEntityMin = 0x10;
constexpr std::uint8_t kEntityMax = 0x1F;
constexpr std::uint8_t kInputMin = 0x20;
constexpr std::uint8_t kInputMax = 0x2F;
constexpr std::uint8_t kSystemMin = 0xF0;
constexpr std::uint8_t kSystemMax = 0xFF;
}  // namespace OpCodeRange

/**
 * @brief Check if an OpCode requires reliable delivery (ACK)
 * @param opcode The operation code to check
 * @return true if the packet must be acknowledged
 *
 * As per RFC RTGP v1.0.0:
 * - All session management ops are RELIABLE
 * - S_ENTITY_SPAWN and S_ENTITY_DESTROY are RELIABLE
 * - S_ENTITY_MOVE, C_INPUT, S_UPDATE_POS, PING, PONG are UNRELIABLE
 */
[[nodiscard]] constexpr bool isReliable(OpCode opcode) noexcept {
    switch (opcode) {
        case OpCode::C_CONNECT:
        case OpCode::S_ACCEPT:
        case OpCode::DISCONNECT:
        case OpCode::C_GET_USERS:
        case OpCode::R_GET_USERS:
        case OpCode::S_UPDATE_STATE:
        case OpCode::S_GAME_OVER:
        case OpCode::C_READY:
        case OpCode::S_GAME_START:
        case OpCode::S_PLAYER_READY_STATE:
        case OpCode::S_ENTITY_SPAWN:
        case OpCode::S_ENTITY_DESTROY:
        case OpCode::S_ENTITY_HEALTH:
        case OpCode::S_POWERUP_EVENT:
            return true;

        case OpCode::S_ENTITY_MOVE:
        case OpCode::C_INPUT:
        case OpCode::S_UPDATE_POS:
        case OpCode::PING:
        case OpCode::PONG:
        case OpCode::ACK:
            return false;
    }
    return false;
}

/**
 * @brief Check if an OpCode is sent by the client
 * @param opcode The operation code to check
 * @return true if this is a client-originated opcode
 */
[[nodiscard]] constexpr bool isClientOpCode(OpCode opcode) noexcept {
    switch (opcode) {
        case OpCode::C_CONNECT:
        case OpCode::C_GET_USERS:
        case OpCode::C_READY:
        case OpCode::C_INPUT:
        case OpCode::PING:
            return true;

        case OpCode::DISCONNECT:
            return true;

        default:
            return false;
    }
}

/**
 * @brief Check if an OpCode is sent by the server
 * @param opcode The operation code to check
 * @return true if this is a server-originated opcode
 */
[[nodiscard]] constexpr bool isServerOpCode(OpCode opcode) noexcept {
    switch (opcode) {
        case OpCode::S_ACCEPT:
        case OpCode::R_GET_USERS:
        case OpCode::S_UPDATE_STATE:
        case OpCode::S_GAME_OVER:
        case OpCode::S_GAME_START:
        case OpCode::S_PLAYER_READY_STATE:
        case OpCode::S_ENTITY_SPAWN:
        case OpCode::S_ENTITY_MOVE:
        case OpCode::S_ENTITY_DESTROY:
        case OpCode::S_ENTITY_HEALTH:
        case OpCode::S_POWERUP_EVENT:
        case OpCode::S_UPDATE_POS:
        case OpCode::PONG:
            return true;

        case OpCode::DISCONNECT:
            return true;

        default:
            return false;
    }
}

/**
 * @brief Check if a raw byte value is a valid OpCode
 * @param value The raw byte to validate
 * @return true if the value corresponds to a defined OpCode
 */
[[nodiscard]] constexpr bool isValidOpCode(std::uint8_t value) noexcept {
    switch (static_cast<OpCode>(value)) {
        case OpCode::C_CONNECT:
        case OpCode::S_ACCEPT:
        case OpCode::DISCONNECT:
        case OpCode::C_GET_USERS:
        case OpCode::R_GET_USERS:
        case OpCode::S_UPDATE_STATE:
        case OpCode::S_GAME_OVER:
        case OpCode::C_READY:
        case OpCode::S_GAME_START:
        case OpCode::S_PLAYER_READY_STATE:
        case OpCode::S_ENTITY_SPAWN:
        case OpCode::S_ENTITY_MOVE:
        case OpCode::S_ENTITY_DESTROY:
        case OpCode::S_ENTITY_HEALTH:
        case OpCode::S_POWERUP_EVENT:
        case OpCode::C_INPUT:
        case OpCode::S_UPDATE_POS:
        case OpCode::PING:
        case OpCode::PONG:
        case OpCode::ACK:
            return true;
        default:
            return false;
    }
}

/**
 * @brief Get the category name for an OpCode
 * @param opcode The operation code
 * @return Category name string
 */
[[nodiscard]] constexpr std::string_view getCategory(OpCode opcode) noexcept {
    auto val = static_cast<std::uint8_t>(opcode);
    if (val >= OpCodeRange::kSessionMin && val <= OpCodeRange::kSessionMax) {
        return "Session";
    }
    if (val >= OpCodeRange::kEntityMin && val <= OpCodeRange::kEntityMax) {
        return "Entity";
    }
    if (val >= OpCodeRange::kInputMin && val <= OpCodeRange::kInputMax) {
        return "Input";
    }
    if (val >= OpCodeRange::kSystemMin) {
        return "System";
    }
    return "Unknown";
}

/**
 * @brief Convert OpCode to human-readable string
 * @param opcode The operation code
 * @return String representation of the opcode
 */
[[nodiscard]] constexpr std::string_view toString(OpCode opcode) noexcept {
    switch (opcode) {
        case OpCode::C_CONNECT:
            return "C_CONNECT";
        case OpCode::S_ACCEPT:
            return "S_ACCEPT";
        case OpCode::DISCONNECT:
            return "DISCONNECT";
        case OpCode::C_GET_USERS:
            return "C_GET_USERS";
        case OpCode::R_GET_USERS:
            return "R_GET_USERS";
        case OpCode::S_UPDATE_STATE:
            return "S_UPDATE_STATE";
        case OpCode::S_GAME_OVER:
            return "S_GAME_OVER";
        case OpCode::C_READY:
            return "C_READY";
        case OpCode::S_GAME_START:
            return "S_GAME_START";
        case OpCode::S_PLAYER_READY_STATE:
            return "S_PLAYER_READY_STATE";
        case OpCode::S_ENTITY_SPAWN:
            return "S_ENTITY_SPAWN";
        case OpCode::S_ENTITY_MOVE:
            return "S_ENTITY_MOVE";
        case OpCode::S_ENTITY_DESTROY:
            return "S_ENTITY_DESTROY";
        case OpCode::S_ENTITY_HEALTH:
            return "S_ENTITY_HEALTH";
        case OpCode::S_POWERUP_EVENT:
            return "S_POWERUP_EVENT";
        case OpCode::C_INPUT:
            return "C_INPUT";
        case OpCode::S_UPDATE_POS:
            return "S_UPDATE_POS";
        case OpCode::PING:
            return "PING";
        case OpCode::PONG:
            return "PONG";
        case OpCode::ACK:
            return "ACK";
    }
    return "UNKNOWN";
}

}  // namespace rtype::network
