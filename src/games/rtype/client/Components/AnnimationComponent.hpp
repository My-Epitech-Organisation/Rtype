/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AnnimationComponent.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_ANNIMATIONCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_ANNIMATIONCOMPONENT_HPP_

struct Animation {
    int frameCount;
    int currentFrame;
    float frameDuration;
    float elapsedTime;
    bool oneTime;

    explicit Animation(int count = 1, float duration = 0.1f,
                       bool oneTime = false)
        : frameCount(count),
          currentFrame(1),
          frameDuration(duration),
          elapsedTime(0.f),
          oneTime(oneTime) {}
};

#endif  //  SRC_GAMES_RTYPE_CLIENT_COMPONENTS_ANNIMATIONCOMPONENT_HPP_
