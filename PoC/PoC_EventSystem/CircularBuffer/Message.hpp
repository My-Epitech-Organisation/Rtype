/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Message
*/

#ifndef MESSAGE_HPP_
#define MESSAGE_HPP_
#include <string>
#include <iostream>
#include <vector>
#include <cstdint>
#include <cstring>

class Message {
    public:
        std::string _type;
        std::string _content;
        std::string _uid;

    public:
        Message() = default;
        Message(const std::string &type, const std::string &content, const std::string &uid = "NONE")
            : _type(type), _content(content), _uid(uid) {};
        ~Message() = default;

        std::vector<uint8_t> serialize() const {
            std::vector<uint8_t> data;

            uint32_t typeLen = static_cast<uint32_t>(_type.size());
            data.insert(data.end(), reinterpret_cast<uint8_t*>(&typeLen), reinterpret_cast<uint8_t*>(&typeLen) + sizeof(typeLen));
            data.insert(data.end(), _type.begin(), _type.end());

            uint32_t contentLen = static_cast<uint32_t>(_content.size());
            data.insert(data.end(), reinterpret_cast<uint8_t*>(&contentLen), reinterpret_cast<uint8_t*>(&contentLen) + sizeof(contentLen));
            data.insert(data.end(), _content.begin(), _content.end());

            uint32_t uidLen = static_cast<uint32_t>(_uid.size());
            data.insert(data.end(), reinterpret_cast<uint8_t*>(&uidLen), reinterpret_cast<uint8_t*>(&uidLen) + sizeof(uidLen));
            data.insert(data.end(), _uid.begin(), _uid.end());

            return data;
        }

        static Message deserialize(const std::vector<uint8_t>& data, size_t& offset) {
            Message msg;

            uint32_t typeLen;
            std::memcpy(&typeLen, data.data() + offset, sizeof(typeLen));
            offset += sizeof(typeLen);
            msg._type = std::string(data.begin() + offset, data.begin() + offset + typeLen);
            offset += typeLen;

            uint32_t contentLen;
            std::memcpy(&contentLen, data.data() + offset, sizeof(contentLen));
            offset += sizeof(contentLen);
            msg._content = std::string(data.begin() + offset, data.begin() + offset + contentLen);
            offset += contentLen;

            uint32_t uidLen;
            std::memcpy(&uidLen, data.data() + offset, sizeof(uidLen));
            offset += sizeof(uidLen);
            msg._uid = std::string(data.begin() + offset, data.begin() + offset + uidLen);
            offset += uidLen;

            return msg;
        }

        size_t serializedSize() const {
            return sizeof(uint32_t) + _type.size()
                + sizeof(uint32_t) + _content.size()
                + sizeof(uint32_t) + _uid.size();
        }

        friend std::ostream& operator<<(std::ostream& os, const Message &obj);
};

#endif /* !MESSAGE_HPP_ */
