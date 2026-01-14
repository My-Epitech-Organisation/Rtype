/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** chillMusic
*/

#ifndef CHILLMUSIC_HPP_
#define CHILLMUSIC_HPP_

#include "../ALevelMusic.hpp"

class ChillMusic : public ALevelMusic {
public:
    ChillMusic(std::shared_ptr<ECS::Registry> registry, std::shared_ptr<AssetManager> assetManager)
        : ALevelMusic(std::move(registry), std::move(assetManager), "ChillMusic") {};
    ~ChillMusic() override = default;
    void loadLevelMusic(std::shared_ptr<AudioLib> audioLib) override;
};

#endif /* !CHILLMUSIC_HPP_ */

