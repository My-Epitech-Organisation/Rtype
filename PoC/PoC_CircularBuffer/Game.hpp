/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Game
*/

#ifndef GAME_HPP_
#define GAME_HPP_

#include <mutex>

class Game {
    private:
        bool _isAppRunning = true;
        std::mutex _mutex;
    public:
        bool isAppRunning()  { std::lock_guard<std::mutex> lock(this->_mutex); return this->_isAppRunning; }
        void setAppRunning(bool status) { std::lock_guard<std::mutex> lock(this->_mutex); this->_isAppRunning = status; }
        Game() = default;
        ~Game() = default;

    protected:
    private:
};

#endif /* !GAME_HPP_ */
