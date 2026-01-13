/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** battleMusic.hpp
*/

#ifndef BATTLEMUSIC_HPP_
#define BATTLEMUSIC_HPP_

#include "../ALevelMusic.hpp"

class BattleMusic : public ALevelMusic {
public:
    BattleMusic(std::shared_ptr<ECS::Registry> registry, std::shared_ptr<AssetManager> assetManager)
        : ALevelMusic(std::move(registry), std::move(assetManager), "BattleMusic") {};
    ~BattleMusic() override = default;
    void loadLevelMusic(std::shared_ptr<AudioLib> audioLib) override;
};

#endif /* !BATTLEMUSIC_HPP_ */

