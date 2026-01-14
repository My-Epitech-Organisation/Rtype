/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SpatialStation.hpp
*/

#ifndef R_TYPE_SPATIALSTATION_HPP
#define R_TYPE_SPATIALSTATION_HPP
#include "../ABackground.hpp"

class SpatialStation : public ABackground {
public:
    void createEntitiesBackground() override;
    SpatialStation(std::shared_ptr<ECS::Registry> registry, std::shared_ptr<AssetManager> assetManager) : ABackground(std::move(registry), std::move(assetManager), "SpatialStation") {};
};


#endif //R_TYPE_SPATIALSTATION_HPP