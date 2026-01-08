/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Clock.cpp
*/

#include "Clock.hpp"

void rtype::display::Clock::restart() {
    auto now = std::chrono::high_resolution_clock::now();
    this->_start = now;
}

rtype::display::Time rtype::display::Clock::getElapsedTime() const {
    auto now = std::chrono::high_resolution_clock::now();
    return Time{ now - this->_start };
}


rtype::display::Clock::Clock() {
    this->_start = std::chrono::high_resolution_clock::now();
}
