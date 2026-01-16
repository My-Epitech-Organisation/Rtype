/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** AsteroidsSpace.hpp
*/

#ifndef R_TYPE_ASTEROIDSSPACE_HPP
#define R_TYPE_ASTEROIDSSPACE_HPP

#include "../ABackground.hpp"

class AsteroidsSpace : public ABackground {
public:
    void createEntitiesBackground() override;
    AsteroidsSpace(std::shared_ptr<ECS::Registry> registry, std::shared_ptr<AssetManager> assetManager) : ABackground(std::move(registry), std::move(assetManager), "AsteroidsSpace") {};
};


#endif //R_TYPE_ASTEROIDSSPACE_HPP