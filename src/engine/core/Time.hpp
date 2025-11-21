/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Time
*/

#pragma once

namespace rtype::engine::core {

class Time {
public:
    Time();

    void update();
    double deltaTime() const { return deltaTime_; }
    double totalTime() const { return totalTime_; }

private:
    double deltaTime_;
    double totalTime_;
    double lastFrameTime_;
};

} // namespace rtype::engine::core
