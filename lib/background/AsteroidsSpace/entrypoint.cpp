/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** entrypoint.cpp
*/

#ifdef _WIN32
    #define RTYPE_EXPORT __declspec(dllexport)
#else
    #define RTYPE_EXPORT
#endif

#include <iostream>
#include "AsteroidsSpace.hpp"

extern "C" {
    RTYPE_EXPORT IBackground *createBackground(std::shared_ptr<ECS::Registry> registry) {
        return new AsteroidsSpace(std::move(registry));
    }
}
