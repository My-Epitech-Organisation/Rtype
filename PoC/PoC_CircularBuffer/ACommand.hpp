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
#include "CircularBuffer.hpp"
#include <memory>
#include <mutex>

class ACommand : public ICommand {
    private:
        rtype::network::CircularBuffer _buffer;
        std::mutex _mutex;

    public:
        ACommand(size_t bufferCapacity = 4096) : _buffer(bufferCapacity) {}
        void execute(Game &game) override;
        void addNewCommand(const Message &command) override;
        bool isEmpty() override {
            std::lock_guard<std::mutex> lock(_mutex);
            return this->_buffer.empty();
        }
        virtual ~ACommand() = default;

    protected:
};

#endif /* !ACOMMAND_HPP_ */
