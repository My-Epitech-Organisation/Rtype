/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Packet
*/

#include "rtype/network/Packet.hpp"

namespace rtype::network {

Packet::Packet() : type_(PacketType::Unknown) {}

Packet::Packet(PacketType type) : type_(type) {}

}  // namespace rtype::network
