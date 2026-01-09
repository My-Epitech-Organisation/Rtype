/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Clock.hpp
*/

#ifndef R_TYPE_CLOCK_HPP
#define R_TYPE_CLOCK_HPP
#include <chrono>

namespace rtype::display {
struct Time {
    std::chrono::nanoseconds duration;

    float asSeconds() const {
        return std::chrono::duration<float>(duration).count();
    }

    int asMilliseconds() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    }
};
class Clock {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> _start;
public:
    void restart();
    Time getElapsedTime() const;
    Clock();
};
}

#endif //R_TYPE_CLOCK_HPP
