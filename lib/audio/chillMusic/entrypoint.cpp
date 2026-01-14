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
#include "ChillMusic.hpp"

extern "C" {
    RTYPE_EXPORT ILevelMusic *createLevelMusic(std::shared_ptr<ECS::Registry> registry, std::shared_ptr<AssetManager> assetManager) {
        return new ChillMusic(std::move(registry), std::move(assetManager));
    }
}
