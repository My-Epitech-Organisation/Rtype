/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** LobbyBackground.hpp
*/

#ifndef R_TYPE_LOBBYBACKGROUND_HPP
#define R_TYPE_LOBBYBACKGROUND_HPP

#include "../ABackground.hpp"

class LobbyBackground : public ABackground {

public:
    void createEntitiesBackground() override;
    LobbyBackground(std::shared_ptr<ECS::Registry> registry, std::shared_ptr<AssetManager> assetManager) : ABackground(std::move(registry), std::move(assetManager), "LOBBY") {};
};


#endif //R_TYPE_LOBBYBACKGROUND_HPP