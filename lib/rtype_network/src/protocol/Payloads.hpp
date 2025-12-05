/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Payloads - Protocol Payload Structures as per RFC RTGP v1.0.0
*/

#pragma once

#include <array>
#include <cstdint>
#include <type_traits>

#include "OpCode.hpp"

namespace rtype::network {

/**
 * @brief Game state enumeration for S_UPDATE_STATE payload
 */
enum class GameState : std::uint8_t {
    Lobby = 0,     ///< Waiting for players
    Running = 1,   ///< Game in progress
    Paused = 2,    ///< Game paused
    GameOver = 3,  ///< Game ended
};

/**
 * @brief Entity type enumeration for S_ENTITY_SPAWN payload
 */
enum class EntityType : std::uint8_t {
    Player = 0,   ///< Player spaceship
    Bydos = 1,    ///< Enemy (Bydos)
    Missile = 2,  ///< Projectile
};

/**
 * @brief Input mask flags for C_INPUT payload
 *
 * Can be combined with bitwise OR for simultaneous inputs.
 */
namespace InputMask {
inline constexpr std::uint8_t kNone = 0x00;
inline constexpr std::uint8_t kUp = 0x01;
inline constexpr std::uint8_t kDown = 0x02;
inline constexpr std::uint8_t kLeft = 0x04;
inline constexpr std::uint8_t kRight = 0x08;
inline constexpr std::uint8_t kShoot = 0x10;
}  // namespace InputMask

#pragma pack(push, 1)

/**
 * @brief Payload for C_CONNECT (0x01)
 * @note Empty payload - connection request has no data
 */
struct ConnectPayload {};

/**
 * @brief Payload for S_ACCEPT (0x02)
 *
 * Server assigns a unique User ID to the connecting client.
 */
struct AcceptPayload {
    std::uint32_t newUserId;
};

/**
 * @brief Payload for DISCONNECT (0x03)
 * @note Empty payload - disconnect has no data
 */
struct DisconnectPayload {};

/**
 * @brief Payload for C_GET_USERS (0x04)
 * @note Empty payload - request has no data
 */
struct GetUsersRequestPayload {};

/**
 * @brief Fixed header for R_GET_USERS (0x05)
 *
 * Variable-length payload: count + array of user IDs.
 * The full payload size is: 1 + (count * 4) bytes.
 *
 * @note For deserialization, read the header and then read `count` uint32_t
 * user IDs from the payload.
 */
struct GetUsersResponseHeader {
    std::uint8_t count;
};

/**
 * @brief Maximum number of users in R_GET_USERS response
 *
 * Limited by count field being uint8_t: max 255 users
 * (255 * 4 bytes per user ID + 1 byte count = 1021 bytes < 1384 max payload
 * size)
 */
inline constexpr std::size_t kMaxUsersInResponse = 255;

/**
 * @brief Payload for S_UPDATE_STATE (0x06)
 */
struct UpdateStatePayload {
    std::uint8_t stateId;

    [[nodiscard]] constexpr GameState getState() const noexcept {
        return static_cast<GameState>(stateId);
    }
};

/**
 * @brief Payload for S_ENTITY_SPAWN (0x10)
 *
 * Server instructs clients to create a new game entity.
 */
struct EntitySpawnPayload {
    std::uint32_t entityId;
    std::uint8_t type;
    float posX;
    float posY;

    [[nodiscard]] constexpr EntityType getType() const noexcept {
        return static_cast<EntityType>(type);
    }
};

/**
 * @brief Payload for S_ENTITY_MOVE (0x11)
 *
 * Regular position/velocity update for an entity.
 * Unreliable - lost packets are corrected by next update.
 */
struct EntityMovePayload {
    std::uint32_t entityId;
    float posX;
    float posY;
    float velX;
    float velY;
};

/**
 * @brief Payload for S_ENTITY_DESTROY (0x12)
 *
 * Server instructs clients to remove an entity.
 */
struct EntityDestroyPayload {
    std::uint32_t entityId;
};

/**
 * @brief Payload for C_INPUT (0x20)
 *
 * Client sends current input state to server.
 */
struct InputPayload {
    std::uint8_t inputMask;

    [[nodiscard]] constexpr bool isUp() const noexcept {
        return (inputMask & InputMask::kUp) != 0;
    }
    [[nodiscard]] constexpr bool isDown() const noexcept {
        return (inputMask & InputMask::kDown) != 0;
    }
    [[nodiscard]] constexpr bool isLeft() const noexcept {
        return (inputMask & InputMask::kLeft) != 0;
    }
    [[nodiscard]] constexpr bool isRight() const noexcept {
        return (inputMask & InputMask::kRight) != 0;
    }
    [[nodiscard]] constexpr bool isShoot() const noexcept {
        return (inputMask & InputMask::kShoot) != 0;
    }
};

/**
 * @brief Payload for S_UPDATE_POS (0x21)
 *
 * Server sends authoritative position to correct client prediction.
 */
struct UpdatePosPayload {
    float posX;
    float posY;
};

/**
 * @brief Payload for PING (0xF0)
 * @note Empty - timestamp can be tracked via seqId
 */
struct PingPayload {};

/**
 * @brief Payload for PONG (0xF1)
 * @note Empty - echoes the seqId from PING via ackId
 */
struct PongPayload {};

#pragma pack(pop)

static_assert(sizeof(ConnectPayload) == 1,
              "ConnectPayload is an empty struct (size 1 in C++), "
              "serialization returns 0 bytes");
static_assert(sizeof(DisconnectPayload) == 1,
              "DisconnectPayload is an empty struct (size 1 in C++), "
              "serialization returns 0 bytes");
static_assert(sizeof(GetUsersRequestPayload) == 1,
              "GetUsersRequestPayload is an empty struct (size 1 in C++), "
              "serialization returns 0 bytes");
static_assert(sizeof(PingPayload) == 1,
              "PingPayload is an empty struct (size 1 in C++), serialization "
              "returns 0 bytes");
static_assert(sizeof(PongPayload) == 1,
              "PongPayload is an empty struct (size 1 in C++), serialization "
              "returns 0 bytes");

static_assert(sizeof(AcceptPayload) == 4,
              "AcceptPayload must be 4 bytes (uint32_t)");
static_assert(sizeof(GetUsersResponseHeader) == 1,
              "GetUsersResponseHeader must be 1 byte");
static_assert(sizeof(UpdateStatePayload) == 1,
              "UpdateStatePayload must be 1 byte");
static_assert(sizeof(EntitySpawnPayload) == 13,
              "EntitySpawnPayload must be 13 bytes (4+1+4+4)");
static_assert(sizeof(EntityMovePayload) == 20,
              "EntityMovePayload must be 20 bytes (4+4+4+4+4)");
static_assert(sizeof(EntityDestroyPayload) == 4,
              "EntityDestroyPayload must be 4 bytes");
static_assert(sizeof(InputPayload) == 1, "InputPayload must be 1 byte");
static_assert(sizeof(UpdatePosPayload) == 8,
              "UpdatePosPayload must be 8 bytes (4+4)");

static_assert(std::is_trivially_copyable_v<AcceptPayload>);
static_assert(std::is_trivially_copyable_v<UpdateStatePayload>);
static_assert(std::is_trivially_copyable_v<EntitySpawnPayload>);
static_assert(std::is_trivially_copyable_v<EntityMovePayload>);
static_assert(std::is_trivially_copyable_v<EntityDestroyPayload>);
static_assert(std::is_trivially_copyable_v<InputPayload>);
static_assert(std::is_trivially_copyable_v<UpdatePosPayload>);

static_assert(std::is_standard_layout_v<AcceptPayload>);
static_assert(std::is_standard_layout_v<UpdateStatePayload>);
static_assert(std::is_standard_layout_v<EntitySpawnPayload>);
static_assert(std::is_standard_layout_v<EntityMovePayload>);
static_assert(std::is_standard_layout_v<EntityDestroyPayload>);
static_assert(std::is_standard_layout_v<InputPayload>);
static_assert(std::is_standard_layout_v<UpdatePosPayload>);

/**
 * @brief Get the expected payload size for a given OpCode
 * @param opcode The operation code
 * @return Expected payload size, or 0 for variable/empty payloads
 *
 * @note R_GET_USERS (0x05) has variable size, returns 0
 */
[[nodiscard]] constexpr std::size_t getPayloadSize(OpCode opcode) noexcept {
    switch (opcode) {
        case OpCode::C_CONNECT:
        case OpCode::DISCONNECT:
        case OpCode::C_GET_USERS:
        case OpCode::PING:
        case OpCode::PONG:
            return 0;

        case OpCode::S_ACCEPT:
            return sizeof(AcceptPayload);
        case OpCode::R_GET_USERS:
            return 0;
        case OpCode::S_UPDATE_STATE:
            return sizeof(UpdateStatePayload);

        case OpCode::S_ENTITY_SPAWN:
            return sizeof(EntitySpawnPayload);
        case OpCode::S_ENTITY_MOVE:
            return sizeof(EntityMovePayload);
        case OpCode::S_ENTITY_DESTROY:
            return sizeof(EntityDestroyPayload);

        case OpCode::C_INPUT:
            return sizeof(InputPayload);
        case OpCode::S_UPDATE_POS:
            return sizeof(UpdatePosPayload);
    }
    return 0;
}

/**
 * @brief Check if an OpCode has a variable-length payload
 */
[[nodiscard]] constexpr bool hasVariablePayload(OpCode opcode) noexcept {
    return opcode == OpCode::R_GET_USERS;
}

}  // namespace rtype::network
