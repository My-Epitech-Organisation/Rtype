/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Message
*/

#include "Message.hpp"

std::ostream& operator<<(std::ostream& os, const Message &obj) 
{
    os << "[" << obj._type << "][" << obj._content << "][" << obj._uid << "]";
    return os;
}

