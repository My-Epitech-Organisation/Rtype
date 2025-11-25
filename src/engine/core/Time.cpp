/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Time
*/

#include "Time.hpp"

namespace rtype::engine::core {

Time::Time() : deltaTime_(0.0), totalTime_(0.0), lastFrameTime_(0.0) {}

void Time::update() {
    // Minimal placeholder - would normally use actual timing
    deltaTime_ = 0.016;  // ~60 FPS
    totalTime_ += deltaTime_;
}

}  // namespace rtype::engine::core
