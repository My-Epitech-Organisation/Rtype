/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** BlackHole.hpp
*/

#ifndef R_TYPE_BLACKHOLE_HPP
#define R_TYPE_BLACKHOLE_HPP
#include "../ABackground.hpp"

class BlackHole : public ABackground {
public:
    void createEntitiesBackground() override;
    BlackHole(std::shared_ptr<ECS::Registry> registry, std::shared_ptr<AssetManager> assetManager) : ABackground(std::move(registry), std::move(assetManager), "BlackHole") {};
};


#endif //R_TYPE_BLACKHOLE_HPP