/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Time
*/

#include "Time.hpp"

namespace rtype::engine::core {

Time::Time() = default;

void Time::update() {
    // Minimal placeholder - would normally use actual timing
    deltaTime_ = TARGET_DELTA_TIME;  // ~60 FPS
    totalTime_ += deltaTime_;
}

}  // namespace rtype::engine::core
