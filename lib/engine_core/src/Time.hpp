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
    [[nodiscard]] auto deltaTime() const -> double { return deltaTime_; }
    [[nodiscard]] auto totalTime() const -> double { return totalTime_; }

   private:
    static constexpr double TARGET_DELTA_TIME = 0.016;
    double deltaTime_ = 0.0;
    double totalTime_ = 0.0;
};

}  // namespace rtype::engine::core
