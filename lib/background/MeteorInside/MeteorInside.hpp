/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** MeteorInside.hpp
*/

#ifndef R_TYPE_METEORINSIDE_HPP
#define R_TYPE_METEORINSIDE_HPP
#include "../ABackground.hpp"

class MeteorInside : public ABackground {
public:
    void createEntitiesBackground() override;
    MeteorInside(std::shared_ptr<ECS::Registry> registry, std::shared_ptr<AssetManager> assetManager) : ABackground(std::move(registry), std::move(assetManager), "MeteorInside") {};
};


#endif //R_TYPE_METEORINSIDE_HPP