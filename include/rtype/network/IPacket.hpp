/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** IPacket - Public interface for network packets
*/

#pragma once

#include <vector>
#include <cstdint>

namespace rtype::network {

/**
 * @brief Enumeration of all packet types in the R-Type protocol
 */
enum class PacketType : uint8_t {
    Unknown = 0,
    PlayerInput,      ///< Client -> Server: Player input commands
    EntityUpdate,     ///< Server -> Client: Entity state updates
    EntitySpawn,      ///< Server -> Client: New entity spawned
    EntityDestroy     ///< Server -> Client: Entity destroyed
};

/**
 * @brief Public interface for network packets
 * 
 * Packets are the fundamental unit of network communication.
 * Each packet has a type and optional binary data payload.
 * 
 * Example usage:
 * @code
 * IPacket& packet = createPacket(PacketType::PlayerInput);
 * packet.setData(inputData);
 * socket.send(packet.serialize());
 * @endcode
 */
class IPacket {
public:
    virtual ~IPacket() = default;
    
    /**
     * @brief Get the packet type
     * @return Type of the packet
     */
    virtual PacketType type() const = 0;
    
    /**
     * @brief Get the packet data payload
     * @return Reference to the data vector
     */
    virtual const std::vector<uint8_t>& data() const = 0;
    
    /**
     * @brief Set the packet data payload
     * @param data New data for the packet
     */
    virtual void setData(const std::vector<uint8_t>& data) = 0;
    
    /**
     * @brief Serialize the packet to binary format
     * @return Serialized packet as byte vector
     */
    virtual std::vector<uint8_t> serialize() const = 0;
};

} // namespace rtype::network
