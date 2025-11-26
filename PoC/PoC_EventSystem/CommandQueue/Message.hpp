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

class Message {
    public:
        std::string _type;
        std::string _content;
        std::string _uid;

    public:
        Message() = delete;
        Message(const std::string &type, const std::string &content, const std::string &uid = "NONE")
            : _type(type), _content(content), _uid(uid) {};
        ~Message() = default;
        friend std::ostream& operator<<(std::ostream& os, const Message &obj);
};

#endif /* !MESSAGE_HPP_ */
