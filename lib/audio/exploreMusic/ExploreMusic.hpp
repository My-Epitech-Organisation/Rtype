/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** ExploreMusic.hpp
*/

#ifndef EXPLOREMUSIC_HPP_
#define EXPLOREMUSIC_HPP_
#include "../ALevelMusic.hpp"

class ExploreMusic : public ALevelMusic {
public:
    ExploreMusic(std::shared_ptr<ECS::Registry> registry, std::shared_ptr<AssetManager> assetManager)
        : ALevelMusic(std::move(registry), std::move(assetManager), "ExploreMusic") {};
    ~ExploreMusic() override = default;
    void loadLevelMusic(std::shared_ptr<AudioLib> audioLib) override;
};

#endif /* !EXPLOREMUSIC_HPP_ */
