/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ACommand
*/


#ifndef ACOMMAND_HPP_
#define ACOMMAND_HPP_

#include "ICommand.hpp"
#include "Game.hpp"
#include <queue>
#include <memory>
#include <mutex>

class ACommand : public ICommand {
    private:
        std::queue<std::string> _commands;
        std::mutex _mutex;

    public:
        ACommand() = default;
        void execute(Game &game) override;
        void addNewCommand(const std::string &command) override;
        bool isEmpty() override {
            std::lock_guard<std::mutex> lock(_mutex);
            return this->_commands.empty();
        }
        virtual ~ACommand() = default;

    protected:
};

#endif /* !ACOMMAND_HPP_ */
