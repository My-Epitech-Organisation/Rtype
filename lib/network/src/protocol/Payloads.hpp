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
    Pickup = 3,   ///< Collectible / power-up
    Obstacle = 4  ///< Static or moving obstacle
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
 * @note Contains a reason code (maps to DisconnectReason)
 */
struct DisconnectPayload {
    std::uint8_t reason{static_cast<std::uint8_t>(4)};  // Default LocalRequest
};

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
 * @brief Payload for S_GAME_OVER (0x07)
 */
struct GameOverPayload {
    std::uint32_t finalScore{0};
};

/**
 * @brief Payload for C_REQUEST_LOBBIES (0x0B)
 * @note Empty payload - request has no data
 */
struct RequestLobbiesPayload {};

/**
 * @brief Single lobby information entry
 *
 * Used within S_LOBBY_LIST response
 */
struct LobbyInfo {
    std::array<char, 6> code;
    std::uint16_t port;
    std::uint8_t playerCount;
    std::uint8_t maxPlayers;
    std::uint8_t isActive;
    std::array<char, 16> levelName;
};

/**
 * @brief Header for S_LOBBY_LIST (0x0C)
 *
 * Variable-length payload: count + array of LobbyInfo entries.
 * The full payload size is: 1 + (count * 11) bytes.
 */
struct LobbyListHeader {
    std::uint8_t count;
};

/**
 * @brief Maximum lobbies in a single S_LOBBY_LIST response
 *
 * Limited by payload size: (kMaxPayloadSize - 1) / sizeof(LobbyInfo)
 * = (1384 - 1) / 11 = 125 lobbies (well above our max of 16)
 */
inline constexpr std::size_t kMaxLobbiesInResponse = 50;

/**
 * @brief Payload for S_ENTITY_SPAWN (0x10)
 *
 * Server instructs clients to create a new game entity.
 */
struct EntitySpawnPayload {
    std::uint32_t entityId;
    std::uint8_t type;
    std::uint8_t subType;
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
 *
 * Uses fixed-point quantization to reduce bandwidth and a server tick to
 * allow client-side interpolation.
 */
struct EntityMovePayload {
    std::uint32_t entityId;   ///< Network entity id
    std::uint32_t serverTick; ///< Monotonic server tick for interpolation
    std::int16_t posX;        ///< Quantized position X (fixed-point)
    std::int16_t posY;        ///< Quantized position Y (fixed-point)
    std::int16_t velX;        ///< Quantized velocity X (fixed-point)
    std::int16_t velY;        ///< Quantized velocity Y (fixed-point)
};

/**
 * @brief Header for S_ENTITY_MOVE_BATCH (0x15)
 *
 * Variable-length payload: header (5 bytes) + array of EntityMoveBatchEntry.
 * The full payload size is: 5 + (count * 12) bytes.
 */
struct EntityMoveBatchHeader {
    std::uint8_t count;       ///< Number of entity updates in this batch (1-115)
    std::uint32_t serverTick; ///< Shared server tick for all entries
};

/**
 * @brief Compact entry for batched entity moves (no serverTick - shared in header)
 *
 * 12 bytes per entity vs 16 bytes in EntityMovePayload = 25% savings
 */
struct EntityMoveBatchEntry {
    std::uint32_t entityId;   ///< Network entity id
    std::int16_t posX;        ///< Quantized position X (fixed-point)
    std::int16_t posY;        ///< Quantized position Y (fixed-point)
    std::int16_t velX;        ///< Quantized velocity X (fixed-point)
    std::int16_t velY;        ///< Quantized velocity Y (fixed-point)
};

/**
 * @brief Payload for C_JOIN_LOBBY (0x0D)
 *
 * Client sends lobby code to authenticate and join
 */
struct JoinLobbyPayload {
    std::array<char, 6> code;
};

/**
 * @brief Payload for S_JOIN_LOBBY_RESPONSE (0x0E)
 *
 * Server responds to join request with success/failure
 */
struct JoinLobbyResponsePayload {
    std::uint8_t accepted;      ///< 1 = accepted, 0 = rejected
    std::uint8_t reason;        ///< Reason code (0 = success, 1 = invalid code, 2 = lobby full)
    std::array<char, 16> levelName;
};

/**
 * @brief Maximum entities per batch packet
 *
 * Limited by payload size: (kMaxPayloadSize - 5) / sizeof(EntityMoveBatchEntry)
 * = (1384 - 5) / 12 = 114 entities
 */
// Payload size: 5 byte header + N * 12 bytes (EntityMoveBatchEntry)
inline constexpr std::size_t kMaxEntitiesPerBatch = 114;

/**
 * @brief Payload for S_ENTITY_DESTROY (0x12)
 *
 * Server instructs clients to remove an entity.
 */
struct EntityDestroyPayload {
    std::uint32_t entityId;
};

/**
 * @brief Payload for S_ENTITY_HEALTH (0x13)
 *
 * Server broadcasts authoritative health/lives for an entity.
 */
struct EntityHealthPayload {
    std::uint32_t entityId;
    std::int32_t current;
    std::int32_t max;
};

/**
 * @brief Payload for S_POWERUP_EVENT (0x14)
 *
 * Server notifies that a player picked up a power-up.
 */
struct PowerUpEventPayload {
    std::uint32_t playerId;
    std::uint8_t powerUpType;
    float duration;
};

/**
 * @brief Payload for S_LEVEL_ANNOUNCE (0x18)
 *
 * Server announces name of new level for visual notification.
 */
struct LevelAnnouncePayload {
    std::array<char, 32> levelName;
    std::array<char, 32> background;
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
 * @note Unreliable - timestamp can be tracked via seqId
 */
struct PingPayload {};

/**
 * @brief Payload for PONG (0xF1)
 * @note Unreliable - echoes the seqId from PING via ackId
 */
struct PongPayload {};

/**
 * @brief Payload for C_READY (0x08)
 * @note Reliable - client signals ready/unready state in lobby
 */
struct LobbyReadyPayload {
    std::uint8_t isReady;
};

/**
 * @brief Payload for S_GAME_START (0x09)
 * @note Reliable - server signals all players ready, game starting
 */
struct GameStartPayload {
    float countdownDuration;  ///< in seconds
};

/**
 * @brief Payload for S_PLAYER_READY_STATE (0x0A)
 * @note Reliable - server broadcasts when a player's ready state changes
 */
struct PlayerReadyStatePayload {
    std::uint32_t userId;
    std::uint8_t isReady;
};

/**
 * @brief Bandwidth mode enumeration for C_SET_BANDWIDTH_MODE
 */
enum class BandwidthMode : std::uint8_t {
    Normal = 0,  ///< Full update rate (60Hz players, ~2Hz enemies)
    Low = 1,     ///< Reduced update rate (~20Hz players, ~0.5Hz enemies)
};

/**
 * @brief Payload for C_SET_BANDWIDTH_MODE (0x16)
 * @note Reliable - client requests bandwidth mode preference
 */
struct BandwidthModePayload {
    std::uint8_t mode;  ///< BandwidthMode value
};

/**
 * @brief Payload for S_BANDWIDTH_MODE_CHANGED (0x17)
 * @note Reliable - server broadcasts bandwidth mode change to all clients
 */
struct BandwidthModeChangedPayload {
    std::uint32_t userId;  ///< User who changed the mode
    std::uint8_t mode;     ///< BandwidthMode value (0=Normal, 1=Low)
    std::uint8_t activeCount;  ///< Number of clients with low bandwidth enabled
};

/**
 * @brief Payload for C_CHAT and S_CHAT (0x30/0x31)
 */
struct ChatPayload {
    std::uint32_t userId; // 0 for system messages or sender id
    char message[256];
};

#pragma pack(pop)

static_assert(sizeof(ConnectPayload) == 1,
              "ConnectPayload is an empty struct (size 1 in C++), "
              "serialization returns 0 bytes");
static_assert(sizeof(DisconnectPayload) == 1,
              "DisconnectPayload must be exactly 1 byte (reason)");
static_assert(sizeof(GetUsersRequestPayload) == 1,
              "GetUsersRequestPayload is an empty struct (size 1 in C++), "
              "serialization returns 0 bytes");
static_assert(sizeof(PingPayload) == 1,
              "PingPayload is an empty struct (size 1 in C++), serialization "
              "returns 0 bytes");
static_assert(sizeof(PongPayload) == 1,
              "PongPayload is an empty struct (size 1 in C++), serialization "
              "returns 0 bytes");
static_assert(sizeof(LobbyReadyPayload) == 1,
              "LobbyReadyPayload must be 1 byte (uint8_t)");
static_assert(sizeof(BandwidthModePayload) == 1,
              "BandwidthModePayload must be 1 byte (uint8_t)");
static_assert(sizeof(BandwidthModeChangedPayload) == 6,
              "BandwidthModeChangedPayload must be 6 bytes (4+1+1)");

static_assert(sizeof(AcceptPayload) == 4,
              "AcceptPayload must be 4 bytes (uint32_t)");
static_assert(sizeof(GetUsersResponseHeader) == 1,
              "GetUsersResponseHeader must be 1 byte");
static_assert(sizeof(UpdateStatePayload) == 1,
              "UpdateStatePayload must be 1 byte");
static_assert(sizeof(GameOverPayload) == 4,
              "GameOverPayload must be 4 bytes (uint32_t)");
static_assert(sizeof(EntitySpawnPayload) == 14,
              "EntitySpawnPayload must be 14 bytes (4+1+1+4+4)");
static_assert(sizeof(EntityMovePayload) == 16,
              "EntityMovePayload must be 16 bytes (4+4+2+2+2+2)");
static_assert(sizeof(EntityMoveBatchHeader) == 5,
              "EntityMoveBatchHeader must be 5 bytes (1+4)");
static_assert(sizeof(EntityMoveBatchEntry) == 12,
              "EntityMoveBatchEntry must be 12 bytes (4+2+2+2+2)");
static_assert(sizeof(EntityDestroyPayload) == 4,
              "EntityDestroyPayload must be 4 bytes");
static_assert(sizeof(EntityHealthPayload) == 12,
              "EntityHealthPayload must be 12 bytes (4+4+4)");
static_assert(sizeof(PowerUpEventPayload) == 9,
              "PowerUpEventPayload must be 9 bytes (4+1+4)");
static_assert(sizeof(InputPayload) == 1, "InputPayload must be 1 byte");
static_assert(sizeof(UpdatePosPayload) == 8,
              "UpdatePosPayload must be 8 bytes (4+4)");
static_assert(sizeof(GameStartPayload) == 4,
              "GameStartPayload must be 4 bytes (float)");
static_assert(sizeof(PlayerReadyStatePayload) == 5,
              "PlayerReadyStatePayload must be 5 bytes (4+1)");
static_assert(sizeof(LobbyInfo) == 27,
              "LobbyInfo must be 27 bytes (6+2+1+1+1+16)");
static_assert(sizeof(LobbyListHeader) == 1,
              "LobbyListHeader must be 1 byte");
static_assert(sizeof(JoinLobbyPayload) == 6,
              "JoinLobbyPayload must be 6 bytes (char[6])");
static_assert(sizeof(JoinLobbyResponsePayload) == 18,
              "JoinLobbyResponsePayload must be 18 bytes (uint8_t+uint8_t+char[16])");
static_assert(sizeof(ChatPayload) == 260,
              "ChatPayload must be 260 bytes (4+256)");

static_assert(std::is_trivially_copyable_v<AcceptPayload>);
static_assert(std::is_trivially_copyable_v<UpdateStatePayload>);
static_assert(std::is_trivially_copyable_v<GameOverPayload>);
static_assert(std::is_trivially_copyable_v<EntitySpawnPayload>);
static_assert(std::is_trivially_copyable_v<EntityMovePayload>);
static_assert(std::is_trivially_copyable_v<EntityMoveBatchHeader>);
static_assert(std::is_trivially_copyable_v<EntityDestroyPayload>);
static_assert(std::is_trivially_copyable_v<EntityHealthPayload>);
static_assert(std::is_trivially_copyable_v<PowerUpEventPayload>);
static_assert(std::is_trivially_copyable_v<InputPayload>);
static_assert(std::is_trivially_copyable_v<UpdatePosPayload>);
static_assert(std::is_trivially_copyable_v<LobbyReadyPayload>);
static_assert(std::is_trivially_copyable_v<GameStartPayload>);
static_assert(std::is_trivially_copyable_v<PlayerReadyStatePayload>);
static_assert(std::is_trivially_copyable_v<ChatPayload>);

static_assert(std::is_standard_layout_v<AcceptPayload>);
static_assert(std::is_standard_layout_v<UpdateStatePayload>);
static_assert(std::is_standard_layout_v<GameOverPayload>);
static_assert(std::is_standard_layout_v<EntitySpawnPayload>);
static_assert(std::is_standard_layout_v<EntityMovePayload>);
static_assert(std::is_standard_layout_v<EntityMoveBatchHeader>);
static_assert(std::is_standard_layout_v<EntityDestroyPayload>);
static_assert(std::is_standard_layout_v<EntityHealthPayload>);
static_assert(std::is_standard_layout_v<PowerUpEventPayload>);
static_assert(std::is_standard_layout_v<InputPayload>);
static_assert(std::is_standard_layout_v<UpdatePosPayload>);
static_assert(std::is_standard_layout_v<LobbyReadyPayload>);
static_assert(std::is_standard_layout_v<GameStartPayload>);
static_assert(std::is_standard_layout_v<PlayerReadyStatePayload>);
static_assert(std::is_standard_layout_v<ChatPayload>);

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
        case OpCode::C_GET_USERS:
        case OpCode::C_REQUEST_LOBBIES:
        case OpCode::PING:
        case OpCode::PONG:
        case OpCode::ACK:
            return 0;

        case OpCode::S_ACCEPT:
            return sizeof(AcceptPayload);
        case OpCode::S_LEVEL_ANNOUNCE:
            return sizeof(LevelAnnouncePayload);
        case OpCode::R_GET_USERS:
            return 0;
        case OpCode::S_UPDATE_STATE:
            return sizeof(UpdateStatePayload);
        case OpCode::S_GAME_OVER:
            return sizeof(GameOverPayload);
        case OpCode::C_READY:
            return sizeof(LobbyReadyPayload);
        case OpCode::S_GAME_START:
            return sizeof(GameStartPayload);
        case OpCode::S_PLAYER_READY_STATE:
            return sizeof(PlayerReadyStatePayload);
        case OpCode::S_LOBBY_LIST:
            return 0;  // Variable-length payload
        case OpCode::C_JOIN_LOBBY:
            return sizeof(JoinLobbyPayload);
        case OpCode::S_JOIN_LOBBY_RESPONSE:
            return sizeof(JoinLobbyResponsePayload);

        case OpCode::S_ENTITY_SPAWN:
            return sizeof(EntitySpawnPayload);
        case OpCode::S_ENTITY_MOVE:
            return sizeof(EntityMovePayload);
        case OpCode::S_ENTITY_MOVE_BATCH:
            return 0;  // Variable-length payload
        case OpCode::C_SET_BANDWIDTH_MODE:
            return sizeof(BandwidthModePayload);
        case OpCode::S_BANDWIDTH_MODE_CHANGED:
            return sizeof(BandwidthModeChangedPayload);
        case OpCode::S_ENTITY_DESTROY:
            return sizeof(EntityDestroyPayload);
        case OpCode::S_ENTITY_HEALTH:
            return sizeof(EntityHealthPayload);
        case OpCode::S_POWERUP_EVENT:
            return sizeof(PowerUpEventPayload);

        case OpCode::C_CHAT:
        case OpCode::S_CHAT:
            return sizeof(ChatPayload);

        case OpCode::C_INPUT:
            return sizeof(InputPayload);
        case OpCode::S_UPDATE_POS:
            return sizeof(UpdatePosPayload);
        case OpCode::DISCONNECT:
            return sizeof(DisconnectPayload);
    }
    return 0;
}

/**
 * @brief Check if an OpCode has a variable-length payload
 */
[[nodiscard]] constexpr bool hasVariablePayload(OpCode opcode) noexcept {
    return opcode == OpCode::R_GET_USERS ||
           opcode == OpCode::S_ENTITY_MOVE_BATCH ||
           opcode == OpCode::S_LOBBY_LIST;
}

}  // namespace rtype::network
