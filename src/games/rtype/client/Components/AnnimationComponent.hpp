/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AnnimationComponent.hpp
*/

#ifndef R_TYPE_ANNIMATIONCOMPONENT_HPP
#define R_TYPE_ANNIMATIONCOMPONENT_HPP

struct Animation {
    int frameCount;
    int currentFrame;
    float frameDuration;
    float elapsedTime;
    bool oneTime;

    Animation(int count = 1, float duration = 0.1f, bool oneTime = false)
        : frameCount(count),
          currentFrame(1),
          frameDuration(duration),
          elapsedTime(0.f),
          oneTime(oneTime) {}

};

#endif //R_TYPE_ANNIMATIONCOMPONENT_HPP