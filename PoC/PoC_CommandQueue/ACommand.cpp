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
    this->_commands.push(command);
}

void ACommand::execute(Game &game)
{
    std::lock_guard<std::mutex> lock(_mutex);
    while (!this->_commands.empty()) {
        std::cout << this->_commands.front() << std::endl;
        this->_commands.pop();
    }
}