/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ACommand
*/

#include <iostream>
#include "ACommand.hpp"

void ACommand::addNewCommand(const Message &command)
{
    std::lock_guard<std::mutex> lock(_mutex);

    std::vector<uint8_t> serialized = command.serialize();
    uint32_t msgSize = static_cast<uint32_t>(serialized.size());

    std::vector<uint8_t> sizeBytes(reinterpret_cast<uint8_t*>(&msgSize),
                                    reinterpret_cast<uint8_t*>(&msgSize) + sizeof(msgSize));
    this->_buffer.write(sizeBytes);
    this->_buffer.write(serialized);
}

void ACommand::execute(Game &game)
{
    std::lock_guard<std::mutex> lock(_mutex);

    while (this->_buffer.size() >= sizeof(uint32_t)) {
        std::vector<uint8_t> sizeBytes = this->_buffer.read(sizeof(uint32_t));
        uint32_t msgSize;
        std::memcpy(&msgSize, sizeBytes.data(), sizeof(msgSize));

        if (this->_buffer.size() < msgSize) {
            break;
        }
        std::vector<uint8_t> msgData = this->_buffer.read(msgSize);
        size_t offset = 0;
        Message msg = Message::deserialize(msgData, offset);

        std::cout << msg << std::endl;
    }
}