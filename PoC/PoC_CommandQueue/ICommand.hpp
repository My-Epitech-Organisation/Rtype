/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ICommand
*/

#ifndef ICOMMAND_HPP_
#define ICOMMAND_HPP_
#include <string>
#include "Game.hpp"

class ICommand {
    public:
        virtual void execute(Game &game) = 0;
        virtual void addNewCommand(const std::string &command) = 0;
        virtual bool isEmpty() = 0;
        ICommand() = default;
        virtual ~ICommand() = default;

    protected:
    private:
};

#endif /* !ICOMMAND_HPP_ */
