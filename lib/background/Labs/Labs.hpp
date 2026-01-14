/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Labs.hpp
*/

#ifndef R_TYPE_LABS_HPP
#define R_TYPE_LABS_HPP

#include "../ABackground.hpp"

class Labs : public ABackground {
public:
    void createEntitiesBackground() override;
    Labs(std::shared_ptr<ECS::Registry> registry, std::shared_ptr<AssetManager> assetManager) : ABackground(std::move(registry), std::move(assetManager), "Labs") {};
};


#endif //R_TYPE_LABS_HPP